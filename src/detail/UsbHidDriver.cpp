#include "UsbHidDriver.h"

#if CONFIG_TINYUSB_HID_ENABLED

#include "esp32-hal-tinyusb.h"
#include "USB.h"
#include "esp_hid_common.h"

#define USB_HID_DEVICES_MAX 10

ESP_EVENT_DEFINE_BASE(ARDUINO_USB_HID_EVENTS);
esp_err_t arduino_usb_event_post(esp_event_base_t event_base, int32_t event_id, void *event_data, size_t event_data_size, TickType_t ticks_to_wait);
esp_err_t arduino_usb_event_handler_register_with(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void *event_handler_arg);

typedef struct {
    UsbHidDevice * device;
    uint8_t reports_num;
    uint8_t * report_ids;
} tinyusb_hid_device_t;

static tinyusb_hid_device_t tinyusb_hid_devices[USB_HID_DEVICES_MAX];
static uint8_t tinyusb_hid_devices_num = 0;
static bool tinyusb_hid_devices_is_initialized = false;
static SemaphoreHandle_t tinyusb_hid_device_input_sem = NULL;
static SemaphoreHandle_t tinyusb_hid_device_input_mutex = NULL;
static bool tinyusb_hid_is_initialized = false;
static uint8_t tinyusb_loaded_hid_devices_num = 0;
static uint16_t tinyusb_hid_device_descriptor_len = 0;
static uint8_t * tinyusb_hid_device_descriptor = NULL;

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    static const char * tinyusb_hid_device_report_types[4] = {"INVALID", "INPUT", "OUTPUT", "FEATURE"};
#endif

static bool tinyusb_enable_hid_device(uint16_t descriptor_len, UsbHidDevice * device){
    if(tinyusb_hid_is_initialized){
        log_e("TinyUSB HID has already started! Device not enabled");
        return false;
    }
    if(tinyusb_loaded_hid_devices_num >= USB_HID_DEVICES_MAX){
        log_e("Maximum devices already enabled! Device not enabled");
        return false;
    }
    tinyusb_hid_device_descriptor_len += descriptor_len;
    tinyusb_hid_devices[tinyusb_loaded_hid_devices_num++].device = device;
    return true;
}

UsbHidDevice * tinyusb_get_device_by_report_id(uint8_t report_id){
    for(uint8_t i=0; i<tinyusb_loaded_hid_devices_num; i++){
        tinyusb_hid_device_t * device = &tinyusb_hid_devices[i];
        if(device->device && device->reports_num){
            for(uint8_t r=0; r<device->reports_num; r++){
                if(report_id == device->report_ids[r]){
                    return device->device;
                }
            }
        }
    }
    return NULL;
}

static uint16_t tinyusb_on_get_feature(uint8_t report_id, uint8_t* buffer, uint16_t reqlen){
    UsbHidDevice * device = tinyusb_get_device_by_report_id(report_id);
    if(device){
        return device->_onGetFeature(report_id, buffer, reqlen);
    }
    return 0;
}

static void tinyusb_on_set_feature(uint8_t report_id, const uint8_t* buffer, uint16_t reqlen){
    UsbHidDevice * device = tinyusb_get_device_by_report_id(report_id);
    if(device){
        device->_onSetFeature(report_id, buffer, reqlen);
    }
}

static void tinyusb_on_set_output(uint8_t report_id, const uint8_t* buffer, uint16_t reqlen){
    UsbHidDevice * device = tinyusb_get_device_by_report_id(report_id);
    if(device){
        device->_onOutput(report_id, buffer, reqlen);
    }
}

static uint16_t tinyusb_on_add_descriptor(uint8_t device_index, uint8_t * dst){
    uint16_t res = 0;
    tinyusb_hid_device_t * device = &tinyusb_hid_devices[device_index];
    if(device->device){
        res = device->device->_onGetDescriptor(dst);
    }
    return res;
}

static bool tinyusb_load_enabled_hid_devices(){
    if(tinyusb_hid_device_descriptor != NULL){
        return true;
    }
    tinyusb_hid_device_descriptor = (uint8_t *)malloc(tinyusb_hid_device_descriptor_len);
    if (tinyusb_hid_device_descriptor == NULL) {
        log_e("HID Descriptor Malloc Failed");
        return false;
    }
    uint8_t * dst = tinyusb_hid_device_descriptor;

    for(uint8_t i=0; i<tinyusb_loaded_hid_devices_num; i++){
        uint16_t len = tinyusb_on_add_descriptor(i, dst);
        if (!len) {
            break;
        } else {
            dst += len;
        }
    }
    return true;
}

extern "C" uint16_t tusb_hid_load_descriptor(uint8_t * dst, uint8_t * itf)
{
    if(tinyusb_hid_is_initialized){
        return 0;
    }
    tinyusb_hid_is_initialized = true;

    uint8_t str_index = tinyusb_add_string_descriptor("BadKB HID"); // <-- RENAMED
    uint8_t ep_in = tinyusb_get_free_in_endpoint();
    TU_VERIFY (ep_in != 0);
    uint8_t ep_out = tinyusb_get_free_out_endpoint();
    TU_VERIFY (ep_out != 0);
    uint8_t descriptor[TUD_HID_INOUT_DESC_LEN] = {
        TUD_HID_INOUT_DESCRIPTOR(*itf, str_index, HID_ITF_PROTOCOL_NONE, tinyusb_hid_device_descriptor_len, ep_out, (uint8_t)(0x80 | ep_in), 64, 1)
    };
    *itf+=1;
    memcpy(dst, descriptor, TUD_HID_INOUT_DESC_LEN);
    return TUD_HID_INOUT_DESC_LEN;
}

uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance){
    if(!tinyusb_load_enabled_hid_devices()){
        return NULL;
    }
    return tinyusb_hid_device_descriptor;
}

void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol){
    arduino_usb_hid_event_data_t p;
    p.instance = instance;
    p.set_protocol.protocol = protocol;
    arduino_usb_event_post(ARDUINO_USB_HID_EVENTS, ARDUINO_USB_HID_SET_PROTOCOL_EVENT, &p, sizeof(arduino_usb_hid_event_data_t), portMAX_DELAY);
}

bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate){
    arduino_usb_hid_event_data_t p;
    p.instance = instance;
    p.set_idle.idle_rate = idle_rate;
    arduino_usb_event_post(ARDUINO_USB_HID_EVENTS, ARDUINO_USB_HID_SET_IDLE_EVENT, &p, sizeof(arduino_usb_hid_event_data_t), portMAX_DELAY);
    return true;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen){
    return tinyusb_on_get_feature(report_id, buffer, reqlen);
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize){
    if(report_type == HID_REPORT_TYPE_OUTPUT){
        tinyusb_on_set_output(report_id, buffer, bufsize);
    }else{
        tinyusb_on_set_feature(report_id, buffer, bufsize);
    }
}

UsbHidDriver::UsbHidDriver(){
    if(!tinyusb_hid_devices_is_initialized){
        tinyusb_hid_devices_is_initialized = true;
        for(uint8_t i=0; i<USB_HID_DEVICES_MAX; i++){
            memset(&tinyusb_hid_devices[i], 0, sizeof(tinyusb_hid_device_t));
        }
        tinyusb_hid_devices_num = 0;
        tinyusb_enable_interface(USB_INTERFACE_HID, TUD_HID_INOUT_DESC_LEN, tusb_hid_load_descriptor);
    }
}

void UsbHidDriver::begin(){
    if(tinyusb_hid_device_input_sem == NULL){
        tinyusb_hid_device_input_sem = xSemaphoreCreateBinary();
    }
    if(tinyusb_hid_device_input_mutex == NULL){
        tinyusb_hid_device_input_mutex = xSemaphoreCreateMutex();
    }
    USB.begin();
}

void UsbHidDriver::end(){
    if (tinyusb_hid_device_input_sem != NULL) {
        vSemaphoreDelete(tinyusb_hid_device_input_sem);
        tinyusb_hid_device_input_sem = NULL;
    }
    if (tinyusb_hid_device_input_mutex != NULL) {
        vSemaphoreDelete(tinyusb_hid_device_input_mutex);
        tinyusb_hid_device_input_mutex = NULL;
    }
}

bool UsbHidDriver::ready(void){
    return tud_hid_n_ready(0);
}

template <class F> struct ArgType;
template <class R, class T1, class T2, class T3>
struct ArgType<R(*)(T1, T2, T3)> { typedef T3 type3; };
typedef ArgType<decltype(&tud_hid_report_complete_cb)>::type3 tud_hid_report_complete_cb_len_t;

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, tud_hid_report_complete_cb_len_t len){
    if (tinyusb_hid_device_input_sem) {
        xSemaphoreGive(tinyusb_hid_device_input_sem);
    }
}

bool UsbHidDriver::SendReport(uint8_t id, const void* data, size_t len, uint32_t timeout_ms){
    if(!tinyusb_hid_device_input_sem || !tinyusb_hid_device_input_mutex){
        return false;
    }
    if(xSemaphoreTake(tinyusb_hid_device_input_mutex, timeout_ms / portTICK_PERIOD_MS) != pdTRUE){
        return false;
    }
    bool res = ready();
    if(res){
        xSemaphoreTake(tinyusb_hid_device_input_sem, 0);
        res = tud_hid_n_report(0, id, data, len);
        if(res){
            if(xSemaphoreTake(tinyusb_hid_device_input_sem, timeout_ms / portTICK_PERIOD_MS) != pdTRUE){
                res = false;
            }
        }
    }
    xSemaphoreGive(tinyusb_hid_device_input_mutex);
    return res;
}

bool UsbHidDriver::addDevice(UsbHidDevice * device, uint16_t descriptor_len){
    if(device && tinyusb_loaded_hid_devices_num < USB_HID_DEVICES_MAX){
        return tinyusb_enable_hid_device(descriptor_len, device);
    }
    return false;
}

void UsbHidDriver::onEvent(esp_event_handler_t callback){
    onEvent(ARDUINO_USB_HID_ANY_EVENT, callback);
}
void UsbHidDriver::onEvent(arduino_usb_hid_event_t event, esp_event_handler_t callback){
    arduino_usb_event_handler_register_with(ARDUINO_USB_HID_EVENTS, event, callback, this);
}

#endif /* CONFIG_TINYUSB_HID_ENABLED */
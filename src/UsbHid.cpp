#include "UsbHid.h"
#include "layouts/KeyboardLayout.h"

#if CONFIG_TINYUSB_HID_ENABLED

ESP_EVENT_DEFINE_BASE(ARDUINO_USB_HID_KEYBOARD_EVENTS);
esp_err_t arduino_usb_event_post(esp_event_base_t event_base, int32_t event_id, void *event_data, size_t event_data_size, TickType_t ticks_to_wait);
esp_err_t arduino_usb_event_handler_register_with(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler, void *event_handler_arg);

static const uint8_t report_descriptor[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_REPORT_ID_KEYBOARD))
};

UsbHid::UsbHid(): hid(){
    static bool initialized = false;
    if(!initialized){
        initialized = true;
        hid.addDevice(this, sizeof(report_descriptor));
    }
}

uint16_t UsbHid::_onGetDescriptor(uint8_t* dst){
    memcpy(dst, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
}

void UsbHid::begin(const uint8_t *layout){
    _asciimap = layout;
    // The USB stack is started by calling USB.begin() globally
}

void UsbHid::end(){
    hid.end();
}

void UsbHid::onEvent(esp_event_handler_t callback){
    onEvent(ARDUINO_USB_HID_KEYBOARD_ANY_EVENT, callback);
}
void UsbHid::onEvent(arduino_usb_hid_keyboard_event_t event, esp_event_handler_t callback){
    arduino_usb_event_handler_register_with(ARDUINO_USB_HID_KEYBOARD_EVENTS, event, callback, this);
}

void UsbHid::_onOutput(uint8_t report_id, const uint8_t* buffer, uint16_t len){
    if(report_id == HID_REPORT_ID_KEYBOARD){
        arduino_usb_hid_keyboard_event_data_t p;
        p.leds = buffer[0];
        arduino_usb_event_post(ARDUINO_USB_HID_KEYBOARD_EVENTS, ARDUINO_USB_HID_KEYBOARD_LED_EVENT, &p, sizeof(arduino_usb_hid_keyboard_event_data_t), portMAX_DELAY);
    }
}

void UsbHid::sendReport(KeyReport* keys)
{
    hid_keyboard_report_t report;
    report.reserved = 0;
    report.modifier = keys->modifiers;
    if (keys->keys) {
        memcpy(report.keycode, keys->keys, 6);
    } else {
        memset(report.keycode, 0, 6);
    }
    hid.SendReport(HID_REPORT_ID_KEYBOARD, &report, sizeof(report));
}

#define SHIFT 0x80

size_t UsbHid::pressRaw(uint8_t k) 
{
    uint8_t i;
    if ((k && k < 0xA5) || (k >= 0xE0 && k < 0xE8)) {
        if (_keyReport.keys[0] != k && _keyReport.keys[1] != k && 
            _keyReport.keys[2] != k && _keyReport.keys[3] != k &&
            _keyReport.keys[4] != k && _keyReport.keys[5] != k) {
            
            for (i=0; i<6; i++) {
                if (_keyReport.keys[i] == 0x00) {
                    _keyReport.keys[i] = k;
                    break;
                }
            }
            if (i == 6) {
                return 0;
            }   
        }
    } else {
        return 0;
    }
    sendReport(&_keyReport);
    return 1;
}

size_t UsbHid::releaseRaw(uint8_t k) 
{
    uint8_t i;
    if (k >= 0xE0 && k < 0xE8) {
        _keyReport.modifiers &= ~(1<<(k-0x80));
    } else if (k && k < 0xA5) {
        for (i=0; i<6; i++) {
            if (0 != k && _keyReport.keys[i] == k) {
                _keyReport.keys[i] = 0x00;
            }
        }
    } else {
        return 0;
    }

    sendReport(&_keyReport);
    return 1;
}

size_t UsbHid::press(uint8_t k) 
{
    if(k>=0xE0 && k<0xE8) {
        // k is not to be changed
    }
    else if (k >= 0x88) {
        k = k - 0x88;
    } else if (k >= 0x80) {
        _keyReport.modifiers |= (1<<(k-0x80));
        k = 0;
    } else {
        k = pgm_read_byte(_asciimap + k);
        if (!k) {
            return 0;
        }

		if ((k & ALT_GR) == ALT_GR) {
			_keyReport.modifiers |= 0x40;
			k &= 0x3F;
		} else if ((k & SHIFT) == SHIFT) {
			_keyReport.modifiers |= 0x02;
			k &= 0x7F;
		}
		if (k == ISO_REPLACEMENT)
			k = ISO_KEY;
	}
    return pressRaw(k);
}

size_t UsbHid::release(uint8_t k) 
{
    if (k >= 0x88) {
        k = k - 0x88;
    } else if (k >= 0x80) {
        _keyReport.modifiers &= ~(1<<(k-0x80));
        k = 0;
    } else {
        k = pgm_read_byte(_asciimap + k);
        if (!k) {
            return 0;
        }
		if ((k & ALT_GR) == ALT_GR) {
			_keyReport.modifiers &= ~(0x40);
			k &= 0x3F;
		} else if ((k & SHIFT) == SHIFT) {
			_keyReport.modifiers &= ~(0x02);
			k &= 0x7F;
		}
		if (k == ISO_REPLACEMENT)
			k = ISO_KEY;
    }
    return releaseRaw(k);
}

void UsbHid::releaseAll(void)
{
    memset(&_keyReport, 0, sizeof(KeyReport));
    sendReport(&_keyReport);
}

size_t UsbHid::write(uint8_t c)
{
    size_t p = press(c);
    release(c);
    return p;
}

size_t UsbHid::write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        if (*buffer != '\r') {
            if (write(*buffer)) {
              n++;
            } else {
              break;
            }
        }
        buffer++;
    }
    return n;
}

#endif /* CONFIG_TINYUSB_HID_ENABLED */
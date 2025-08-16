#ifndef USB_HID_H
#define USB_HID_H

#include "sdkconfig.h"
#if CONFIG_TINYUSB_HID_ENABLED

#include "HIDInterface.h"
#include "detail/UsbHidDriver.h"
#include "layouts/KeyboardLayout.h"
#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(ARDUINO_USB_HID_KEYBOARD_EVENTS);

typedef enum {
    ARDUINO_USB_HID_KEYBOARD_ANY_EVENT = ESP_EVENT_ANY_ID,
    ARDUINO_USB_HID_KEYBOARD_LED_EVENT = 0,
    ARDUINO_USB_HID_KEYBOARD_MAX_EVENT,
} arduino_usb_hid_keyboard_event_t;

typedef union {
    struct {
        uint8_t numlock : 1;
        uint8_t capslock : 1;
        uint8_t scrolllock : 1;
        uint8_t compose : 1;
        uint8_t kana : 1;
        uint8_t reserved : 3;
    };
    uint8_t leds;
} arduino_usb_hid_keyboard_event_data_t;

class UsbHid : public UsbHidDevice, public HIDInterface {
private:
    UsbHidDriver hid;
    KeyReport _keyReport;
    const uint8_t *_asciimap;
    void sendReport(KeyReport* keys);

public:
    UsbHid(void);
    void begin(const uint8_t *layout = KeyboardLayout_en_US) override;
    void end(void) override;

    size_t press(uint8_t k) override;
    size_t release(uint8_t k) override;
    void releaseAll(void) override;
    
    size_t write(uint8_t k) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    // press() and release() for media keys are not supported by the base USBHIDKeyboard
    size_t press(const MediaKeyReport k) override { return 0; } 

    // raw functions work with TinyUSB's HID_KEY_* macros
    size_t pressRaw(uint8_t k) override;
    size_t releaseRaw(uint8_t k) override;

    void setLayout(const uint8_t *layout) override { _asciimap = layout; };
    bool isConnected() override { return hid.ready(); }

    void onEvent(esp_event_handler_t callback);
    void onEvent(arduino_usb_hid_keyboard_event_t event, esp_event_handler_t callback);

    // internal use
    uint16_t _onGetDescriptor(uint8_t *buffer) override;
    void _onOutput(uint8_t report_id, const uint8_t *buffer, uint16_t len) override;
};

#endif // CONFIG_TINYUSB_HID_ENABLED
#endif // USB_HID_H
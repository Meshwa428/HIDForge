#include "UsbMouse.h"

#if CONFIG_TINYUSB_HID_ENABLED

static const uint8_t report_descriptor[] = {
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(HID_REPORT_ID_MOUSE))
};

UsbMouse::UsbMouse() : hid(), _buttons(0) {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        hid.addDevice(this, sizeof(report_descriptor));
    }
}

uint16_t UsbMouse::_onGetDescriptor(uint8_t *dst) {
    memcpy(dst, report_descriptor, sizeof(report_descriptor));
    return sizeof(report_descriptor);
}

void UsbMouse::begin() {
    hid.begin();
}

void UsbMouse::end() {
    hid.end();
}

void UsbMouse::click(uint8_t b) {
    press(b);
    release(b);
}

void UsbMouse::press(uint8_t b) {
    _buttons |= b;
    move(0, 0, 0, 0);
}

void UsbMouse::release(uint8_t b) {
    _buttons &= ~b;
    move(0, 0, 0, 0);
}

bool UsbMouse::isPressed(uint8_t b) {
    return (_buttons & b) != 0;
}

void UsbMouse::move(int8_t x, int8_t y, int8_t wheel, int8_t hWheel) {
    if (isConnected()) {
        hid_mouse_report_t report = {
            .buttons = _buttons,
            .x = x,
            .y = y,
            .wheel = wheel,
            .pan = hWheel
        };
        hid.SendReport(HID_REPORT_ID_MOUSE, &report, sizeof(report));
    }
}

#endif // CONFIG_TINYUSB_HID_ENABLED
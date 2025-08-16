#ifndef USB_MOUSE_H
#define USB_MOUSE_H

#include "sdkconfig.h"
#if CONFIG_TINYUSB_HID_ENABLED

#include "MouseInterface.h"
#include "detail/UsbHidDriver.h"

class UsbMouse : public UsbHidDevice, public MouseInterface {
private:
    UsbHidDriver hid;
    uint8_t _buttons;
    void sendReport(void);

public:
    UsbMouse(void);
    void begin(void) override;
    void end(void) override;

    void press(uint8_t b = MOUSE_LEFT) override;
    void release(uint8_t b = MOUSE_LEFT) override;
    void click(uint8_t b = MOUSE_LEFT) override;

    void move(int8_t x, int8_t y, int8_t wheel = 0, int8_t hWheel = 0) override;
    
    bool isPressed(uint8_t b = MOUSE_LEFT) override;
    bool isConnected() override { return hid.ready(); }

    // Internal use
    uint16_t _onGetDescriptor(uint8_t *buffer) override;
};

#endif // CONFIG_TINYUSB_HID_ENABLED
#endif // USB_MOUSE_H
#ifndef HID_INTERFACE_H
#define HID_INTERFACE_H

#include <Arduino.h>
#include "common/keys.h"

class HIDInterface : public Print {
public:
    virtual void begin(const uint8_t *layout) = 0;
    virtual void end(void) = 0;
    
    virtual size_t press(uint8_t k) = 0;
    virtual size_t pressRaw(uint8_t k) = 0;
    virtual size_t press(const MediaKeyReport k) = 0;
    
    virtual size_t release(uint8_t k) = 0;
    virtual size_t releaseRaw(uint8_t k) = 0;
    virtual void releaseAll(void) = 0;

    virtual size_t write(uint8_t k) override = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) override = 0;

    virtual bool isConnected() = 0;
    virtual void setLayout(const uint8_t *layout) = 0;
};

#endif // HID_INTERFACE_H
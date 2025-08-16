#ifndef MOUSE_INTERFACE_H
#define MOUSE_INTERFACE_H

#include <Arduino.h>
#include "common/keys.h"

class MouseInterface {
public:
    virtual ~MouseInterface() {}

    virtual void begin() = 0;
    virtual void end() = 0;
    
    virtual void press(uint8_t b = MOUSE_LEFT) = 0;
    virtual void release(uint8_t b = MOUSE_LEFT) = 0;
    virtual void click(uint8_t b = MOUSE_LEFT) = 0;
    
    virtual void move(int8_t x, int8_t y, int8_t wheel = 0, int8_t hWheel = 0) = 0;
    
    virtual bool isPressed(uint8_t b = MOUSE_LEFT) = 0;
    virtual bool isConnected() = 0;
};

#endif // MOUSE_INTERFACE_H
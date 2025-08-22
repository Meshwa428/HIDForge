#ifndef BLE_HID_ADAPTER_H
#define BLE_HID_ADAPTER_H

#include "HIDInterface.h"
#include "BleKeyboard.h"

class BleHid : public HIDInterface {
public:
    BleHid(BleKeyboard* keyboard);
    ~BleHid();

    void begin(const uint8_t *layout = nullptr) override;
    void end(void) override;
    
    size_t press(uint8_t k) override;
    size_t pressRaw(uint8_t k) override;
    size_t press(const MediaKeyReport k) override;

    size_t release(uint8_t k) override;
    size_t releaseRaw(uint8_t k) override;
    void releaseAll(void) override;

    size_t write(uint8_t k) override;
    size_t write(const MediaKeyReport c);
    size_t write(const uint8_t *buffer, size_t size) override;

    bool isConnected() override;
    void setLayout(const uint8_t *layout) override;

private:
    BleKeyboard* bleKeyboard_;
    const uint8_t* _asciimap;
};

#endif // BLE_HID_ADAPTER_H

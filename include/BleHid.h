#ifndef BLE_HID_H
#define BLE_HID_H

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "HIDInterface.h"
#include "layouts/KeyboardLayout.h"
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <NimBLEServer.h>

class BleHid : public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks, public HIDInterface {
private:
    NimBLEHIDDevice* hid;
    NimBLECharacteristic* inputKeyboard;
    NimBLECharacteristic* outputKeyboard;
    NimBLECharacteristic* inputMediaKeys;
    NimBLEAdvertising* advertising;
    NimBLEServer* pServer;
    KeyReport _keyReport;
    MediaKeyReport _mediaKeyReport;
    String deviceName;
    String deviceManufacturer;
    uint8_t batteryLevel;
    bool connected = false;
    uint32_t _delay_ms = 7;
    const uint8_t* _asciimap;

    void delay_ms(uint64_t ms);
    void sendReport(KeyReport* keys);
    void sendReport(MediaKeyReport* keys);

protected:
    virtual void onStarted(NimBLEServer *pServer) {};
    virtual void onConnect(NimBLEServer *pServer) override;
    virtual void onDisconnect(NimBLEServer *pServer) override;
    virtual void onAuthenticationComplete(ble_gap_conn_desc* desc);
    virtual void onWrite(NimBLECharacteristic* me) override;

public:
    BleHid(String deviceName = "ESP32 Keyboard", String deviceManufacturer = "BruceFW", uint8_t batteryLevel = 100);
    
    void begin(const uint8_t *layout = KeyboardLayout_en_US) override;
    void end(void) override;
    
    size_t press(uint8_t k) override;
    size_t press(const MediaKeyReport k) override;

    size_t release(uint8_t k) override;
    size_t release(const MediaKeyReport k);

    void releaseAll(void) override;

    size_t write(uint8_t c) override;
    size_t write(const MediaKeyReport c);
    size_t write(const uint8_t *buffer, size_t size) override;

    bool isConnected(void) override;
    void setLayout(const uint8_t *layout = KeyboardLayout_en_US) override { _asciimap = layout; }
    
    void setBatteryLevel(uint8_t level);
    void setName(String deviceName);
    void setDelay(uint32_t ms);

    // Raw press/release is not standard for this BLE implementation, but we'll map it.
    size_t pressRaw(uint8_t k) override { return press(k); }
    size_t releaseRaw(uint8_t k) override { return release(k); }
};

#endif // CONFIG_BT_ENABLED
#endif // BLE_HID_H
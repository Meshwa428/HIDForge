#ifndef HIDFORGE_BLE_MOUSE_H
#define HIDFORGE_BLE_MOUSE_H

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "MouseInterface.h"
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <NimBLEServer.h>
#include "NimBLEConnInfo.h"

class BleMouse : public NimBLEServerCallbacks, public MouseInterface {
private:
    NimBLEHIDDevice* hid;
    NimBLECharacteristic* inputMouse;
    NimBLEAdvertising* advertising;
    
    uint8_t _buttons;
    bool connected = false;
    String deviceName;
    String deviceManufacturer;
    uint8_t batteryLevel;
    
    void sendReport();

protected:
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;

public:
    BleMouse(String deviceName = "HIDForge Mouse", String deviceManufacturer = "HIDForge", uint8_t batteryLevel = 100);
    
    void begin() override;
    void end() override;
    
    void press(uint8_t b = MOUSE_LEFT) override;
    void release(uint8_t b = MOUSE_LEFT) override;
    void click(uint8_t b = MOUSE_LEFT) override;

    void move(int8_t x, int8_t y, int8_t wheel = 0, int8_t hWheel = 0) override;

    bool isPressed(uint8_t b = MOUSE_LEFT) override;
    bool isConnected() override;
};

#endif // CONFIG_BT_ENABLED
#endif // HIDFORGE_BLE_MOUSE_H
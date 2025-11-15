#include "BleMouse.h"
#include "HIDTypes.h"

#if defined(CONFIG_BT_ENABLED)

#if defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define LOG_TAG ""
#else
#include "esp_log.h"
static const char* LOG_TAG = "BleMouse";
#endif

#define MOUSE_ID 0x03

static const uint8_t _hidReportDescriptor[] = {
    USAGE_PAGE(1), 0x01,       // USAGE_PAGE (Generic Desktop)
    USAGE(1), 0x02,            // USAGE (Mouse)
    COLLECTION(1), 0x01,       // COLLECTION (Application)
    REPORT_ID(1), MOUSE_ID,
    USAGE(1), 0x01,            //   USAGE (Pointer)
    COLLECTION(1), 0x00,       //   COLLECTION (Physical)
    USAGE_PAGE(1), 0x09,       //     USAGE_PAGE (Button)
    USAGE_MINIMUM(1), 0x01,    //     USAGE_MINIMUM (Button 1)
    USAGE_MAXIMUM(1), 0x05,    //     USAGE_MAXIMUM (Button 5)
    LOGICAL_MINIMUM(1), 0x00,  //     LOGICAL_MINIMUM (0)
    LOGICAL_MAXIMUM(1), 0x01,  //     LOGICAL_MAXIMUM (1)
    REPORT_SIZE(1), 0x01,      //     REPORT_SIZE (1)
    REPORT_COUNT(1), 0x05,      //     REPORT_COUNT (5)
    HIDINPUT(1), 0x02,         //     INPUT (Data,Var,Abs)
    REPORT_SIZE(1), 0x03,      //     REPORT_SIZE (3)
    REPORT_COUNT(1), 0x01,      //     REPORT_COUNT (1)
    HIDINPUT(1), 0x03,         //     INPUT (Cnst,Var,Abs)
    USAGE_PAGE(1), 0x01,       //     USAGE_PAGE (Generic Desktop)
    USAGE(1), 0x30,            //     USAGE (X)
    USAGE(1), 0x31,            //     USAGE (Y)
    USAGE(1), 0x38,            //     USAGE (Wheel)
    LOGICAL_MINIMUM(1), 0x81,  //     LOGICAL_MINIMUM (-127)
    LOGICAL_MAXIMUM(1), 0x7f,  //     LOGICAL_MAXIMUM (127)
    REPORT_SIZE(1), 0x08,      //     REPORT_SIZE (8)
    REPORT_COUNT(1), 0x03,      //     REPORT_COUNT (3)
    HIDINPUT(1), 0x06,         //     INPUT (Data,Var,Rel)
    END_COLLECTION(0),         //   END_COLLECTION
    END_COLLECTION(0)          // END_COLLECTION
};


BleMouse::BleMouse(String deviceName, String deviceManufacturer, uint8_t batteryLevel)
    : hid(0), deviceName(deviceName), deviceManufacturer(deviceManufacturer), batteryLevel(batteryLevel), _buttons(0) {}

void BleMouse::begin() {
    NimBLEServer* pServer = NimBLEDevice::getServer();
    if (pServer == nullptr) {
        ESP_LOGE(LOG_TAG, "BLE Server not found. Please start BleManager first.");
        return;
    }
    pServer->setCallbacks(this);

    hid = new NimBLEHIDDevice(pServer);
    inputMouse = hid->getInputReport(MOUSE_ID);

    hid->setManufacturer(deviceManufacturer.c_str());
    hid->setPnp(0x02, 0x05ac, 0x0220, 0x011b); // Apple Inc, Magic Mouse
    hid->setHidInfo(0x00, 0x02);

    hid->setReportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
    hid->startServices();

    advertising = pServer->getAdvertising();
    advertising->setAppearance(HID_MOUSE);
    advertising->addServiceUUID(hid->getHidService()->getUUID());
    advertising->enableScanResponse(true);
    advertising->start();
    hid->setBatteryLevel(batteryLevel);
}

void BleMouse::end() {
    NimBLEServer* pServer = NimBLEDevice::getServer();
    if (pServer && hid) {
        pServer->setCallbacks(nullptr);

        NimBLEService* hidService = hid->getHidService();
        NimBLEService* deviceInfoService = hid->getDeviceInfoService();
        NimBLEService* batteryService = hid->getBatteryService();

        if (hidService) pServer->removeService(hidService, true);
        if (deviceInfoService) pServer->removeService(deviceInfoService, true);
        if (batteryService) pServer->removeService(batteryService, true);

        delete hid;
        hid = nullptr;
    }
    this->connected = false;
}

void BleMouse::click(uint8_t b) {
    press(b);
    delay(10);
    release(b);
}

void BleMouse::move(int8_t x, int8_t y, int8_t wheel, int8_t hWheel) {
    if (this->isConnected()) {
        uint8_t mouse_report[4] = { _buttons, (uint8_t)x, (uint8_t)y, (uint8_t)wheel};
        this->inputMouse->setValue(mouse_report, sizeof(mouse_report));
        this->inputMouse->notify();
    }
}

void BleMouse::press(uint8_t b) {
    _buttons |= b;
    move(0, 0, 0, 0);
}

void BleMouse::release(uint8_t b) {
    _buttons &= ~b;
    move(0, 0, 0, 0);
}

bool BleMouse::isPressed(uint8_t b) {
    return (_buttons & b) != 0;
}

bool BleMouse::isConnected() {
    return this->connected;
}

void BleMouse::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    ESP_LOGI(LOG_TAG, "Client connection initiated, waiting for pairing...");
}

void BleMouse::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    this->connected = false;
    ESP_LOGI(LOG_TAG, "Client disconnected");
}

void BleMouse::onAuthenticationComplete(NimBLEConnInfo& connInfo)
{
    if(!connInfo.isEncrypted()) {
        // This shouldn't happen, but just in case.
        NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
        ESP_LOGE(LOG_TAG, "Authentication complete but connection not encrypted");
        return;
    }

    ESP_LOGI(LOG_TAG, "Authentication complete");
    NimBLEDevice::getServer()->updateConnParams(connInfo.getConnHandle(), 6, 12, 0, 300);
    this->connected = true;
}

#endif // CONFIG_BT_ENABLED

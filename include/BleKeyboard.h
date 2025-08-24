#ifndef ESP32_BLE_KEYBOARD_H
#define ESP32_BLE_KEYBOARD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "NimBLECharacteristic.h"
#include "NimBLEHIDDevice.h"
#include "NimBLEServer.h"
#include "NimBLEAdvertising.h"
#include "NimBLEConnInfo.h"
#include "common/keys.h"

#include "Print.h"

#define BLE_KEYBOARD_VERSION "0.0.5"
#define BLE_KEYBOARD_VERSION_MAJOR 0
#define BLE_KEYBOARD_VERSION_MINOR 0
#define BLE_KEYBOARD_VERSION_REVISION 5

class BleKeyboard : public Print, public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks
{
private:
  NimBLEHIDDevice* hid;
  NimBLECharacteristic* inputKeyboard;
  NimBLECharacteristic* outputKeyboard;
  NimBLECharacteristic* inputMediaKeys;
  NimBLEAdvertising*    advertising;
  KeyReport          _keyReport;
  MediaKeyReport     _mediaKeyReport;
  std::string        deviceName;
  std::string        deviceManufacturer;
  uint8_t            batteryLevel;
  bool               connected = false;
  uint32_t           _delay_ms = 7;
  void delay_ms(uint64_t ms);

  uint16_t vid       = 0x05ac;
  uint16_t pid       = 0x820a;
  uint16_t version   = 0x0210;

public:
  BleKeyboard(std::string deviceName = "ESP32 Keyboard", std::string deviceManufacturer = "Espressif", uint8_t batteryLevel = 100);
  virtual ~BleKeyboard(); // <-- FIX: Declare virtual destructor
  void begin(void);
  void end(void);
  void sendReport(KeyReport* keys);
  void sendReport(MediaKeyReport* keys);
  size_t press(uint8_t k);
  size_t press(const MediaKeyReport k);
  size_t pressRaw(uint8_t k);
  size_t release(uint8_t k);
  size_t releaseRaw(uint8_t k);
  size_t release(const MediaKeyReport k);
  size_t write(uint8_t c);
  size_t write(const MediaKeyReport c);
  size_t write(const uint8_t *buffer, size_t size);
  void releaseAll(void);
  bool isConnected(void);
  void setBatteryLevel(uint8_t level);
  void setName(std::string deviceName);
  void setDelay(uint32_t ms);

  void set_vendor_id(uint16_t vid);
  void set_product_id(uint16_t pid);
  void set_version(uint16_t version);
  void setWriteError(void);
protected:
  virtual void onStarted(NimBLEServer *pServer) { };
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
  void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
};

#endif // CONFIG_BT_ENABLED
#endif // ESP32_BLE_KEYBOARD_H

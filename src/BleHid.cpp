#include "BleHid.h"
#include "layouts/KeyboardLayout.h"

#if defined(CONFIG_BT_ENABLED)

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include "HIDTypes.h"
#include "sdkconfig.h"
#include <driver/adc.h>

#if defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define LOG_TAG ""
#else
#include "esp_log.h"
static const char *LOG_TAG = "BLEDevice";
#endif

// Report IDs:
#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02

static const uint8_t _hidReportDescriptor[] = {
    USAGE_PAGE(1), 0x01, USAGE(1), 0x06, COLLECTION(1), 0x01,
    REPORT_ID(1), KEYBOARD_ID, USAGE_PAGE(1), 0x07, USAGE_MINIMUM(1), 0xE0, USAGE_MAXIMUM(1), 0xE7, LOGICAL_MINIMUM(1), 0x00, LOGICAL_MAXIMUM(1), 0x01, REPORT_SIZE(1), 0x01, REPORT_COUNT(1), 0x08, HIDINPUT(1), 0x02,
    REPORT_COUNT(1), 0x01, REPORT_SIZE(1), 0x08, HIDINPUT(1), 0x01,
    REPORT_COUNT(1), 0x05, REPORT_SIZE(1), 0x01, USAGE_PAGE(1), 0x08, USAGE_MINIMUM(1), 0x01, USAGE_MAXIMUM(1), 0x05, HIDOUTPUT(1), 0x02,
    REPORT_COUNT(1), 0x01, REPORT_SIZE(1), 0x03, HIDOUTPUT(1), 0x01,
    REPORT_COUNT(1), 0x06, REPORT_SIZE(1), 0x08, LOGICAL_MINIMUM(1), 0x00, LOGICAL_MAXIMUM(1), 0x65, USAGE_PAGE(1), 0x07, USAGE_MINIMUM(1), 0x00, USAGE_MAXIMUM(1), 0x65, HIDINPUT(1), 0x00, END_COLLECTION(0),
    USAGE_PAGE(1), 0x0C, USAGE(1), 0x01, COLLECTION(1), 0x01,
    REPORT_ID(1), MEDIA_KEYS_ID, USAGE_PAGE(1), 0x0C, LOGICAL_MINIMUM(1), 0x00, LOGICAL_MAXIMUM(1), 0x01, REPORT_SIZE(1), 0x01, REPORT_COUNT(1), 0x10,
    USAGE(1), 0xB5, USAGE(1), 0xB6, USAGE(1), 0xB7, USAGE(1), 0xCD, USAGE(1), 0xE2, USAGE(1), 0xE9, USAGE(1), 0xEA, USAGE(2), 0x23, 0x02,
    USAGE(2), 0x94, 0x01, USAGE(2), 0x92, 0x01, USAGE(2), 0x2A, 0x02, USAGE(2), 0x21, 0x02, USAGE(2), 0x26, 0x02, USAGE(2), 0x24, 0x02, USAGE(2), 0x83, 0x01, USAGE(2), 0x8A, 0x01,
    HIDINPUT(1), 0x02, END_COLLECTION(0)
};

BleHid::BleHid(String deviceName, String deviceManufacturer, uint8_t batteryLevel)
    : hid(0), deviceName(deviceName),
      deviceManufacturer(deviceManufacturer), batteryLevel(batteryLevel) {}

void BleHid::begin(const uint8_t *layout) {
    _asciimap = layout;
    NimBLEDevice::init(deviceName.c_str());
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(this);

    hid = new NimBLEHIDDevice(pServer);
    inputKeyboard = hid->inputReport(KEYBOARD_ID);
    outputKeyboard = hid->outputReport(KEYBOARD_ID);
    inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);

    outputKeyboard->setCallbacks(this);
    
    hid->manufacturer()->setValue(deviceManufacturer.c_str());
    hid->pnp(0x02, 0x05ac, 0x820a, 0x0210);
    hid->hidInfo(0x00, 0x01);

    NimBLEDevice::setSecurityAuth(true, true, true);

    hid->reportMap((uint8_t *)_hidReportDescriptor, sizeof(_hidReportDescriptor));
    hid->startServices();

    onStarted(pServer);

    advertising = pServer->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->setScanResponse(false);
    advertising->start();
    hid->setBatteryLevel(batteryLevel);
}

void BleHid::end(void) {
    if (pServer->getConnectedCount() > 0) {
        pServer->disconnect(pServer->getPeerInfo(0).getConnHandle());
    }
    delete hid;
    NimBLEDevice::deinit(true);
    this->connected = false;
}

bool BleHid::isConnected(void) { 
    return this->connected; 
}

void BleHid::setBatteryLevel(uint8_t level) {
    this->batteryLevel = level;
    if (hid != 0) this->hid->setBatteryLevel(this->batteryLevel);
}

void BleHid::setName(String deviceName) { this->deviceName = deviceName; }

void BleHid::setDelay(uint32_t ms) { this->_delay_ms = ms; }

void BleHid::sendReport(KeyReport *keys) {
    if (this->isConnected()) {
        this->inputKeyboard->setValue((uint8_t *)keys, sizeof(KeyReport));
        this->inputKeyboard->notify();
        this->delay_ms(_delay_ms);
    }
}

void BleHid::sendReport(MediaKeyReport *keys) {
    if (this->isConnected()) {
        this->inputMediaKeys->setValue((uint8_t *)keys, sizeof(MediaKeyReport));
        this->inputMediaKeys->notify();
        this->delay_ms(_delay_ms);
    }
}

size_t BleHid::press(uint8_t k) {
    uint8_t i;
    if (k >= 0xE0 && k < 0xE8) {
        _keyReport.modifiers |= (1 << (k - 0xE0));
    } else if (k >= 0x88) {
        k = k - 0x88;
    } else if (k >= 0x80) {
        _keyReport.modifiers |= (1 << (k - 0x80));
        k = 0;
    } else {
        k = pgm_read_byte(_asciimap + k);
        if (!k) return 0;
        if ((k & ALT_GR) == ALT_GR) {
            _keyReport.modifiers |= 0x40;
            k &= 0x3F;
        } else if ((k & SHIFT) == SHIFT) {
            _keyReport.modifiers |= 0x02;
            k &= 0x7F;
        }
        if (k == ISO_REPLACEMENT) k = ISO_KEY;
    }

    if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
        _keyReport.keys[2] != k && _keyReport.keys[3] != k &&
        _keyReport.keys[4] != k && _keyReport.keys[5] != k) {
        for (i = 0; i < 6; i++) {
            if (_keyReport.keys[i] == 0x00) {
                _keyReport.keys[i] = k;
                break;
            }
        }
        if (i == 6) return 0;
    }
    sendReport(&_keyReport);
    return 1;
}

size_t BleHid::press(const MediaKeyReport k) {
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);

    mediaKeyReport_16 |= k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

    sendReport(&_mediaKeyReport);
    return 1;
}

size_t BleHid::release(uint8_t k) {
    uint8_t i;
    if (k >= 0xE0 && k < 0xE8) {
        _keyReport.modifiers &= ~(1 << (k - 0xE0));
    } else if (k >= 0x88) {
        k = k - 0x88;
    } else if (k >= 0x80) {
        _keyReport.modifiers &= ~(1 << (k - 0x80));
        k = 0;
    } else {
        k = pgm_read_byte(_asciimap + k);
        if (!k) return 0;
        if ((k & ALT_GR) == ALT_GR) {
            _keyReport.modifiers &= ~(0x40);
            k &= 0x3F;
        } else if ((k & SHIFT) == SHIFT) {
            _keyReport.modifiers &= ~(0x02);
            k &= 0x7F;
        }
        if (k == ISO_REPLACEMENT) k = ISO_KEY;
    }

    for (i = 0; i < 6; i++) {
        if (0 != k && _keyReport.keys[i] == k) {
            _keyReport.keys[i] = 0x00;
        }
    }

    sendReport(&_keyReport);
    return 1;
}

size_t BleHid::release(const MediaKeyReport k) {
    uint16_t k_16 = k[1] | (k[0] << 8);
    uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
    mediaKeyReport_16 &= ~k_16;
    _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
    _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

    sendReport(&_mediaKeyReport);
    return 1;
}

void BleHid::releaseAll(void) {
    memset(&_keyReport, 0, sizeof(KeyReport));
    memset(&_mediaKeyReport, 0, sizeof(MediaKeyReport));
    sendReport(&_keyReport);
    sendReport(&_mediaKeyReport);
}

size_t BleHid::write(uint8_t c) {
    size_t p = press(c);
    release(c);
    return p;
}

size_t BleHid::write(const MediaKeyReport c) {
    size_t p = press(c);
    release(c);
    return p;
}

size_t BleHid::write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
        if (*buffer != '\r') {
            if (write(*buffer)) {
                n++;
            } else {
                break;
            }
        }
        buffer++;
    }
    return n;
}

void BleHid::onConnect(NimBLEServer *pServer) {
    // A client has connected at the link layer, but we are not ready for HID yet.
    // Wait for the onAuthenticationComplete callback.
    ESP_LOGI(LOG_TAG, "Client connection initiated, waiting for pairing...");
}

void BleHid::onDisconnect(NimBLEServer *pServer) {
    this->connected = false;
    ESP_LOGI(LOG_TAG, "Client disconnected");
}

void BleHid::onAuthenticationComplete(ble_gap_conn_desc* desc) {
    if (desc->sec_state.encrypted) {
        ESP_LOGI(LOG_TAG, "Paired successfully. HID ready.");
        this->connected = true;
    } else {
        ESP_LOGE(LOG_TAG, "Pairing failed");
        this->connected = false;
        NimBLEDevice::getServer()->disconnect(desc->conn_handle);
    }
}

void BleHid::onWrite(NimBLECharacteristic *me) {
    uint8_t* value = (uint8_t*)(me->getValue().c_str());
    ESP_LOGI(LOG_TAG, "special keys: %d", *value);
}

void BleHid::delay_ms(uint64_t ms) {
    uint64_t m = esp_timer_get_time();
    if (ms) {
        uint64_t e = (m + (ms * 1000));
        if (m > e) { // overflow
            while (esp_timer_get_time() > e) {}
        }
        while (esp_timer_get_time() < e) {}
    }
}
#endif // CONFIG_BT_ENABLED
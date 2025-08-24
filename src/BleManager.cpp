#include "BleManager.h"
#include <NimBLEDevice.h>
#include <BleKeyboard.h>
#include "esp_log.h"

static const char* LOG_TAG = "BLE_MGR";

BleManager::BleManager() :
    bleTaskHandle_(nullptr),
    startSemaphore_(nullptr),
    stopSemaphore_(nullptr),
    currentState_(State::IDLE),
    startKeyboardRequested_(false),
    stopKeyboardRequested_(false)
{}

BleManager::~BleManager() {
    if (bleTaskHandle_ != nullptr) vTaskDelete(bleTaskHandle_);
    if (startSemaphore_ != nullptr) vSemaphoreDelete(startSemaphore_);
    if (stopSemaphore_ != nullptr) vSemaphoreDelete(stopSemaphore_);
}

void BleManager::setup() {
    startSemaphore_ = xSemaphoreCreateBinary();
    stopSemaphore_ = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(
        this->bleTaskWrapper,
        "BLEManagerTask",
        4096,
        this,
        5,
        &bleTaskHandle_,
        CONFIG_BT_NIMBLE_PINNED_TO_CORE
    );
}

BleKeyboard* BleManager::startKeyboard() {
    if (currentState_ != State::IDLE) {
        ESP_LOGW(LOG_TAG, "startKeyboard called when not idle. State: %d", (int)currentState_);
        return bleKeyboard_.get();
    }

    ESP_LOGI(LOG_TAG, "Main thread requesting keyboard start...");
    startKeyboardRequested_ = true;

    if (xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000)) == pdTRUE) {
        ESP_LOGI(LOG_TAG, "Main thread confirmed keyboard is ready.");
        return bleKeyboard_.get();
    } else {
        ESP_LOGE(LOG_TAG, "startKeyboard timed out!");
        startKeyboardRequested_ = false;
        return nullptr;
    }
}

void BleManager::stopKeyboard() {
    if (currentState_ != State::KEYBOARD_ACTIVE) {
        return;
    }
    ESP_LOGI(LOG_TAG, "Main thread requesting keyboard stop...");
    stopKeyboardRequested_ = true;

    xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
    ESP_LOGI(LOG_TAG, "Main thread confirmed keyboard is stopped.");
}

bool BleManager::isKeyboardConnected() const {
    if (bleKeyboard_ && currentState_ == State::KEYBOARD_ACTIVE) {
        return bleKeyboard_->isConnected();
    }
    return false;
}

BleManager::State BleManager::getState() const {
    return currentState_;
}


void BleManager::bleTaskWrapper(void* param) {
    static_cast<BleManager*>(param)->taskLoop();
}

void BleManager::taskLoop() {
    ESP_LOGI(LOG_TAG, "BLE Manager task started.");
    NimBLEServer *pServer = nullptr;
    currentState_ = State::IDLE;

    for (;;) {
        if (currentState_ == State::IDLE && startKeyboardRequested_) {
            ESP_LOGI(LOG_TAG, "Start request received. Initializing NimBLE...");
            if(!NimBLEDevice::init("")) {
                ESP_LOGE(LOG_TAG, "NimBLE::init() failed!");
                bleKeyboard_.reset();
            } else {
                ESP_LOGI(LOG_TAG, "NimBLE::init() success. Creating server...");
                pServer = NimBLEDevice::createServer();
                ESP_LOGI(LOG_TAG, "Server created. Creating keyboard...");
                bleKeyboard_.reset(new BleKeyboard("HIDForge Keyboard", "HIDForge", 100));
                ESP_LOGI(LOG_TAG, "Keyboard created. Calling begin()...");
                bleKeyboard_->begin();
                currentState_ = State::KEYBOARD_ACTIVE;
                ESP_LOGI(LOG_TAG, "Keyboard started. State is now KEYBOARD_ACTIVE.");
            }

            startKeyboardRequested_ = false;
            xSemaphoreGive(startSemaphore_);
            ESP_LOGI(LOG_TAG, "Start semaphore given.");

        } else if (currentState_ == State::KEYBOARD_ACTIVE && stopKeyboardRequested_) {
            ESP_LOGI(LOG_TAG, "Stop request received. Shutting down...");
            if (pServer && pServer->getAdvertising()->isAdvertising()) {
                pServer->getAdvertising()->stop();
            }
            if (pServer && pServer->getConnectedCount() > 0) {
                 auto peerDevices = pServer->getPeerDevices();
                 for(auto& peer : peerDevices) pServer->disconnect(peer);
                 vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(bleKeyboard_) {
                bleKeyboard_->end();
            }
            if(NimBLEDevice::isInitialized()) {
                ESP_LOGI(LOG_TAG, "De-initializing NimBLE...");
                NimBLEDevice::deinit(true);
                ESP_LOGI(LOG_TAG, "NimBLE de-initialized.");
            }
            bleKeyboard_.reset();
            pServer = nullptr;
            currentState_ = State::IDLE;
            stopKeyboardRequested_ = false;
            xSemaphoreGive(stopSemaphore_);
            ESP_LOGI(LOG_TAG, "Stop semaphore given. State is now IDLE.");
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

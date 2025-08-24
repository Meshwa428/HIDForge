#include "BleManager.h"
#include <NimBLEDevice.h>
#include <BleKeyboard.h>

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
        return bleKeyboard_.get();
    }

    startKeyboardRequested_ = true;

    if (xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000)) == pdTRUE) {
        return bleKeyboard_.get();
    } else {
        startKeyboardRequested_ = false;
        return nullptr;
    }
}

void BleManager::stopKeyboard() {
    if (currentState_ != State::KEYBOARD_ACTIVE) {
        return;
    }
    stopKeyboardRequested_ = true;

    xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
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
    NimBLEServer *pServer = nullptr;
    currentState_ = State::IDLE;

    for (;;) {
        if (currentState_ == State::IDLE && startKeyboardRequested_) {
            if(!NimBLEDevice::init("")) {
                bleKeyboard_.reset();
            } else {
                pServer = NimBLEDevice::createServer();
                bleKeyboard_.reset(new BleKeyboard("Kiva Ducky", "Kiva Systems", 100));
                bleKeyboard_->begin();
                currentState_ = State::KEYBOARD_ACTIVE;
            }

            startKeyboardRequested_ = false;
            xSemaphoreGive(startSemaphore_);

        } else if (currentState_ == State::KEYBOARD_ACTIVE && stopKeyboardRequested_) {
            // --- THIS IS THE CORRECTED, SAFE SHUTDOWN SEQUENCE ---
            // 1. Stop advertising and disconnect clients (part of NimBLE state)
            if (pServer && pServer->getAdvertising()->isAdvertising()) {
                pServer->getAdvertising()->stop();
            }
            if (pServer && pServer->getConnectedCount() > 0) {
                 auto peerDevices = pServer->getPeerDevices();
                 for(auto& peer : peerDevices) pServer->disconnect(peer);
                 vTaskDelay(pdMS_TO_TICKS(200));
            }

            // 2. Clean up services and HID device *within* the keyboard object.
            // The BleKeyboard object itself still exists.
            if(bleKeyboard_) {
                bleKeyboard_->end();
            }

            // 3. De-initialize the entire NimBLE stack. This correctly cleans up
            // the server and removes its internal pointer to our BleKeyboard object.
            if(NimBLEDevice::isInitialized()) {
                NimBLEDevice::deinit(true);
            }

            // 4. NOW it is safe to destroy the BleKeyboard C++ object.
            bleKeyboard_.reset();

            pServer = nullptr;
            currentState_ = State::IDLE;
            stopKeyboardRequested_ = false;
            xSemaphoreGive(stopSemaphore_); // Unblock main thread
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#include "BleManager.h"
#include <NimBLEDevice.h>
#include "BleKeyboard.h"
#include "BleMouse.h"

BleManager::BleManager() :
    bleTaskHandle_(nullptr),
    startSemaphore_(nullptr),
    stopSemaphore_(nullptr),
    currentState_(State::IDLE),
    startKeyboardRequested_(false),
    stopKeyboardRequested_(false),
    startMouseRequested_(false),
    stopMouseRequested_(false)
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
    if (currentState_ == State::ACTIVE && bleKeyboard_) {
        return bleKeyboard_.get();
    }
    startKeyboardRequested_ = true;
    if (xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000)) == pdTRUE) {
        return bleKeyboard_.get();
    }
    return nullptr;
}

BleMouse* BleManager::startMouse() {
    if (currentState_ == State::ACTIVE && bleMouse_) {
        return bleMouse_.get();
    }
    startMouseRequested_ = true;
    if (xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000)) == pdTRUE) {
        return bleMouse_.get();
    }
    return nullptr;
}

void BleManager::stopKeyboard() {
    if (!bleKeyboard_) return;
    stopKeyboardRequested_ = true;
    xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
}

void BleManager::stopMouse() {
    if (!bleMouse_) return;
    stopMouseRequested_ = true;
    xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
}

bool BleManager::isKeyboardConnected() const {
    return bleKeyboard_ && bleKeyboard_->isConnected();
}

bool BleManager::isMouseConnected() const {
    return bleMouse_ && bleMouse_->isConnected();
}

BleManager::State BleManager::getState() const {
    return currentState_;
}

void BleManager::bleTaskWrapper(void* param) {
    static_cast<BleManager*>(param)->taskLoop();
}

void BleManager::taskLoop() {
    NimBLEServer *pServer = nullptr;

    for (;;) {
        // --- HANDLE STARTUP REQUESTS ---
        if (startKeyboardRequested_ || startMouseRequested_) {
            if (currentState_ == State::IDLE) {
                if(NimBLEDevice::init("")) {
                    pServer = NimBLEDevice::createServer();
                    currentState_ = State::ACTIVE;
                } else {
                    // Initialization failed, clear requests and unblock caller
                    bleKeyboard_.reset();
                    bleMouse_.reset();
                    startKeyboardRequested_ = false;
                    startMouseRequested_ = false;
                    xSemaphoreGive(startSemaphore_);
                    goto loop_delay; // Skip to end of loop
                }
            }

            if (startKeyboardRequested_) {
                if (!bleKeyboard_) {
                    bleKeyboard_ = std::make_unique<BleKeyboard>();
                    bleKeyboard_->begin();
                }
                startKeyboardRequested_ = false;
            }

            if (startMouseRequested_) {
                if (!bleMouse_) {
                    bleMouse_ = std::make_unique<BleMouse>();
                    bleMouse_->begin();
                }
                startMouseRequested_ = false;
            }
            
            xSemaphoreGive(startSemaphore_);
        }
        // --- HANDLE SHUTDOWN REQUESTS ---
        else if (stopKeyboardRequested_ || stopMouseRequested_) {
            if (stopKeyboardRequested_) {
                if(bleKeyboard_) {
                    bleKeyboard_->end();
                    bleKeyboard_.reset();
                }
                stopKeyboardRequested_ = false;
            }
            if (stopMouseRequested_) {
                if(bleMouse_) {
                    bleMouse_->end();
                    bleMouse_.reset();
                }
                stopMouseRequested_ = false;
            }

            // If no devices are left, shut down the entire BLE stack
            if (!bleKeyboard_ && !bleMouse_ && currentState_ == State::ACTIVE) {
                if (pServer && pServer->getAdvertising()->isAdvertising()) {
                    pServer->getAdvertising()->stop();
                }
                if (pServer && pServer->getConnectedCount() > 0) {
                     auto peerDevices = pServer->getPeerDevices();
                     for(auto& peer : peerDevices) pServer->disconnect(peer);
                     vTaskDelay(pdMS_TO_TICKS(200));
                }
                if(NimBLEDevice::isInitialized()) {
                    NimBLEDevice::deinit(true);
                }
                pServer = nullptr;
                currentState_ = State::IDLE;
            }
            xSemaphoreGive(stopSemaphore_);
        }

    loop_delay:
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
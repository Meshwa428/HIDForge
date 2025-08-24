#include "BleManager.h"
#include <NimBLEDevice.h>
#include <BleKeyboard.h>
#include <BleMouse.h>

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
    startKeyboardRequested_ = true;
    if (currentState_ == State::IDLE) {
        xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000));
    }
    return bleKeyboard_.get();
}

void BleManager::stopKeyboard() {
    stopKeyboardRequested_ = true;
    if (bleMouse_ == nullptr) {
        xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
    }
}

BleMouse* BleManager::startMouse() {
    startMouseRequested_ = true;
    if (currentState_ == State::IDLE) {
        xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000));
    }
    return bleMouse_.get();
}

void BleManager::stopMouse() {
    stopMouseRequested_ = true;
    if (bleKeyboard_ == nullptr) {
        xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
    }
}

bool BleManager::isKeyboardConnected() const {
    if (bleKeyboard_) {
        return bleKeyboard_->isConnected();
    }
    return false;
}

bool BleManager::isMouseConnected() const {
    if (bleMouse_) {
        return bleMouse_->isConnected();
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
        // Start sequence
        if (currentState_ == State::IDLE && (startKeyboardRequested_ || startMouseRequested_)) {
            if(!NimBLEDevice::init("")) {
                // handle error
            } else {
                pServer = NimBLEDevice::createServer();
                currentState_ = State::ACTIVE;
            }
            xSemaphoreGive(startSemaphore_);
        }

        if (currentState_ == State::ACTIVE) {
            if (startKeyboardRequested_) {
                bleKeyboard_.reset(new BleKeyboard());
                bleKeyboard_->begin();
                startKeyboardRequested_ = false;
            }
            if (startMouseRequested_) {
                bleMouse_.reset(new BleMouse());
                bleMouse_->begin();
                startMouseRequested_ = false;
            }
        }

        // Stop sequence
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

        if (bleKeyboard_ == nullptr && bleMouse_ == nullptr && currentState_ == State::ACTIVE) {
            if(NimBLEDevice::isInitialized()) {
                NimBLEDevice::deinit(true);
            }
            pServer = nullptr;
            currentState_ = State::IDLE;
            xSemaphoreGive(stopSemaphore_);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

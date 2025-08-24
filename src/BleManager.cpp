#include "BleManager.h"
#include <NimBLEDevice.h>

BleManager::BleManager() :
    bleTaskHandle_(nullptr),
    startSemaphore_(nullptr),
    stopSemaphore_(nullptr),
    currentState_(State::IDLE),
    startRequested_(false),
    stopRequested_(false)
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

void BleManager::start() {
    if (currentState_ != State::IDLE) {
        return;
    }

    startRequested_ = true;
    xSemaphoreTake(startSemaphore_, pdMS_TO_TICKS(5000));
}

void BleManager::stop() {
    if (currentState_ != State::ACTIVE) {
        return;
    }
    stopRequested_ = true;
    xSemaphoreTake(stopSemaphore_, pdMS_TO_TICKS(2000));
}

BleManager::State BleManager::getState() const {
    return currentState_;
}

void BleManager::bleTaskWrapper(void* param) {
    static_cast<BleManager*>(param)->taskLoop();
}

void BleManager::taskLoop() {
    currentState_ = State::IDLE;

    for (;;) {
        if (currentState_ == State::IDLE && startRequested_) {
            if(NimBLEDevice::init("")) {
                NimBLEServer* pServer = NimBLEDevice::createServer();
                currentState_ = State::ACTIVE;
            }

            startRequested_ = false;
            xSemaphoreGive(startSemaphore_);

        } else if (currentState_ == State::ACTIVE && stopRequested_) {
            if(NimBLEDevice::isInitialized()) {
                NimBLEDevice::deinit(true);
            }

            currentState_ = State::IDLE;
            stopRequested_ = false;
            xSemaphoreGive(stopSemaphore_);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

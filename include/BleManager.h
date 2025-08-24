#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class BleManager {
public:
    enum class State {
        IDLE,           // Stack is off
        ACTIVE          // Stack is on
    };

    BleManager();
    ~BleManager();

    void setup();
    void start();
    void stop();

    State getState() const;

private:
    static void bleTaskWrapper(void* param);
    void taskLoop();

    TaskHandle_t bleTaskHandle_;
    SemaphoreHandle_t startSemaphore_; // Confirms startup is complete
    SemaphoreHandle_t stopSemaphore_;  // Confirms shutdown is complete

    volatile State currentState_;
    volatile bool startRequested_;
    volatile bool stopRequested_;
};

#endif // BLE_MANAGER_H

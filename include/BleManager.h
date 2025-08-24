#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <string>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class BleKeyboard;
class BleMouse;

class BleManager {
public:
    enum class State {
        IDLE,           // Stack is off
        ACTIVE          // Stack is on
    };

    BleManager();
    ~BleManager();

    void setup();

    BleKeyboard* startKeyboard();
    void stopKeyboard();
    BleMouse* startMouse();
    void stopMouse();

    State getState() const;
    bool isKeyboardConnected() const;
    bool isMouseConnected() const;

private:
    static void bleTaskWrapper(void* param);
    void taskLoop();

    TaskHandle_t bleTaskHandle_;
    SemaphoreHandle_t startSemaphore_; // Confirms startup is complete
    SemaphoreHandle_t stopSemaphore_;  // Confirms shutdown is complete

    volatile State currentState_;
    volatile bool startKeyboardRequested_;
    volatile bool stopKeyboardRequested_;
    volatile bool startMouseRequested_;
    volatile bool stopMouseRequested_;

    std::unique_ptr<BleKeyboard> bleKeyboard_;
    std::unique_ptr<BleMouse> bleMouse_;
};

#endif // BLE_MANAGER_H

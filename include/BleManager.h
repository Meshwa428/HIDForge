// HIDForge/include/BleManager.h

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <string>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "HIDInterface.h"
#include "MouseInterface.h"

// Forward declare the concrete implementation classes
class BleKeyboard;
class BleMouse;
class BleHid;

class BleManager {
public:
    enum class State {
        IDLE,           // Stack is off
        ACTIVE          // Stack is on and one or more services are running
    };

    BleManager();
    ~BleManager();

    void setup();

    // --- REVISED Public API ---
    // Returns a NON-OWNING pointer to a generic HIDInterface.
    HIDInterface* startKeyboard();
    
    // Returns a NON-OWNING pointer to a generic MouseInterface.
    MouseInterface* startMouse();

    void stopKeyboard();
    void stopMouse();

    State getState() const;
    bool isKeyboardConnected() const;
    bool isMouseConnected() const;

private:
    static void bleTaskWrapper(void* param);
    void taskLoop();

    TaskHandle_t bleTaskHandle_;
    SemaphoreHandle_t startSemaphore_;
    SemaphoreHandle_t stopSemaphore_;

    volatile State currentState_;
    volatile bool startKeyboardRequested_;
    volatile bool stopKeyboardRequested_;
    volatile bool startMouseRequested_;
    volatile bool stopMouseRequested_;

    // unique_ptr to manage device object lifetimes
    std::unique_ptr<BleKeyboard> bleKeyboard_;
    std::unique_ptr<BleMouse> bleMouse_;

    // --- NEW: unique_ptr to manage the wrapper object lifetimes ---
    std::unique_ptr<BleHid> bleHidWrapper_;
};

#endif // BLE_MANAGER_H
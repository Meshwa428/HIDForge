#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <string>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class BleKeyboard;
class BleMouse; // Forward declare BleMouse

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
    // Returns a valid keyboard pointer on success, nullptr on failure.
    BleKeyboard* startKeyboard();
    
    // Returns a valid mouse pointer on success, nullptr on failure.
    BleMouse* startMouse();

    // Stops the keyboard service. Shuts down BLE stack if it's the last active device.
    void stopKeyboard();
    
    // Stops the mouse service. Shuts down BLE stack if it's the last active device.
    void stopMouse();

    // --- Getters ---
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

    // unique_ptr to manage device object lifetimes
    std::unique_ptr<BleKeyboard> bleKeyboard_;
    std::unique_ptr<BleMouse> bleMouse_;
};

#endif // BLE_MANAGER_H
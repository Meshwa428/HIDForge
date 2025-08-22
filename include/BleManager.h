#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <string>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class BleKeyboard;

class BleManager {
public:
    enum class State {
        IDLE,           // Stack is off
        KEYBOARD_ACTIVE // Stack is on and keyboard services are running
    };

    BleManager();
    ~BleManager();

    void setup();

    // --- REVISED Public API ---
    // Returns a valid keyboard pointer on success, nullptr on failure.
    // This function BLOCKS until the BLE task is fully ready.
    BleKeyboard* startKeyboard();

    // This function BLOCKS until the BLE task has fully shut down.
    void stopKeyboard();

    // --- Getters for UI ---
    State getState() const;
    bool isKeyboardConnected() const;

private:
    static void bleTaskWrapper(void* param);
    void taskLoop();

    TaskHandle_t bleTaskHandle_;
    SemaphoreHandle_t startSemaphore_; // Confirms startup is complete
    SemaphoreHandle_t stopSemaphore_;  // Confirms shutdown is complete

    volatile State currentState_;
    volatile bool startKeyboardRequested_;
    volatile bool stopKeyboardRequested_;

    // unique_ptr to manage the keyboard object's lifetime
    std::unique_ptr<BleKeyboard> bleKeyboard_;
};

#endif // BLE_MANAGER_H

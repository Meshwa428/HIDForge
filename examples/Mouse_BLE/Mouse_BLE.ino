#include <Arduino.h>
#include <HIDForge.h>

BleManager bleManager;
BleMouse* mouse = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting BLE Mouse Example with BleManager...");
  
  // Initialize the manager's background task
  bleManager.setup();

  // Start the mouse. This will also start the BLE stack if it's not already running.
  mouse = bleManager.startMouse();
  
  if (mouse) {
    Serial.println("Waiting for a device to connect and pair...");
  } else {
    Serial.println("Failed to start BLE mouse. Halting.");
    while(1);
  }
}

void loop() {
  if (bleManager.isMouseConnected()) {
    // Move the mouse in a circle
    for (int i = 0; i < 360; i += 15) {
      int x = 20 * cos(i * PI / 180);
      int y = 20 * sin(i * PI / 180);
      mouse->move(x, y);
      delay(50);
    }
    
    // Perform a right-click
    mouse->click(MOUSE_RIGHT);
    delay(1000);
  }
}
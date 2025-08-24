#include <Arduino.h>
#include "BleManager.h"
#include "BleMouse.h"

BleManager bleManager;
BleMouse* mouse = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  bleManager.setup();
  mouse = bleManager.startMouse();

  Serial.println("Starting BLE Mouse Example...");
  Serial.println("Waiting for a device to connect and pair...");

  if (mouse) {
    while(!mouse->isConnected()) {
      delay(100);
    }
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
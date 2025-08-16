#include <Arduino.h>
#include <BadKB.h>

BleMouse mouse("BadKB BLE Mouse");

void setup() {
  Serial.begin(115200);
  delay(1000);

  mouse.begin();
  Serial.println("Starting BLE Mouse Example...");
  Serial.println("Waiting for a device to connect and pair...");
}

void loop() {
  if (mouse.isConnected()) {
    // Move the mouse in a circle
    for (int i = 0; i < 360; i += 15) {
      int x = 20 * cos(i * PI / 180);
      int y = 20 * sin(i * PI / 180);
      mouse.move(x, y);
      delay(50);
    }
    
    // Perform a right-click
    mouse.click(MOUSE_RIGHT);
    delay(1000);
  }
}
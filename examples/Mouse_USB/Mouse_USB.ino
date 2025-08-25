#include <Arduino.h>
#include <HIDForge.h> // Correct main header

UsbMouse mouse;

void setup() {
  Serial.begin(115200);
  delay(1000);

  mouse.begin();
  USB.begin(); // <-- IMPORTANT: Start the USB stack
  
  Serial.println("Starting USB Mouse Example...");

  while(!mouse.isConnected()) {
    delay(100);
  }
  
  Serial.println("USB Mouse Connected. Moving in 3 seconds...");
  delay(3000);
}

void loop() {
  // Move the mouse in a square
  mouse.move(50, 0);  // Right
  delay(500);
  mouse.move(0, 50);  // Down
  delay(500);
  mouse.move(-50, 0); // Left
  delay(500);
  mouse.move(0, -50); // Up
  delay(500);

  // Click the left button
  mouse.click();
  delay(1000);
}
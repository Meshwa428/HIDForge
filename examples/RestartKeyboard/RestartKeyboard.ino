#include <Arduino.h>
#include "BleManager.h"
#include "BleKeyboard.h"

BleManager bleManager;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE Keyboard restart example");

  bleManager.setup();
}

void loop() {
  Serial.println("Starting keyboard...");
  BleKeyboard* keyboard = bleManager.startKeyboard();

  if (keyboard) {
    Serial.println("Keyboard started. Waiting for connection...");
    while(!keyboard->isConnected()) {
      delay(100);
    }
    Serial.println("Keyboard connected. Sending 'Hello'");
    keyboard->print("Hello");
    delay(2000);
  } else {
    Serial.println("Failed to start keyboard");
  }

  Serial.println("Stopping keyboard...");
  bleManager.stopKeyboard();
  Serial.println("Keyboard stopped.");

  Serial.println("Waiting 5 seconds before restarting...");
  delay(5000);
}

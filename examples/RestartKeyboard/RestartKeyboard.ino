#include <Arduino.h>
#include "BleManager.h"
#include "BleKeyboard.h"

BleManager bleManager;
BleKeyboard keyboard("HIDForge Keyboard");

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting BLE Keyboard restart example");
}

void loop() {
  Serial.println("Starting BLE stack...");
  bleManager.start();
  keyboard.begin();

  Serial.println("Keyboard started. Waiting for connection...");
  while(!keyboard.isConnected()) {
    delay(100);
  }
  Serial.println("Keyboard connected. Sending 'Hello'");
  keyboard.print("Hello");
  delay(2000);

  Serial.println("Stopping BLE stack...");
  bleManager.stop();
  Serial.println("BLE stack stopped.");

  Serial.println("Waiting 5 seconds before restarting...");
  delay(5000);
}

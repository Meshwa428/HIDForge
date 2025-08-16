#include <Arduino.h>
#include <BadKB.h>

UsbHid keyboard;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize USB HID with the US layout
  keyboard.begin(KeyboardLayout_en_US);
  Serial.println("Starting USB Keyboard Example...");

  // Wait for the USB device to be connected and ready
  while(!keyboard.isConnected()) {
    delay(100);
  }
  
  Serial.println("USB HID Connected. Typing in 3 seconds...");
  delay(3000); // Give user time to focus a text editor

  // Type a message
  keyboard.println("Hello from BruceBadUSB!");
  
  // Use modifier keys to open the Run dialog on Windows (Win + R)
  keyboard.press(KEY_LEFT_GUI);
  keyboard.press('r');
  keyboard.releaseAll();
  
  delay(500);
  
  // Type a command and press Enter
  keyboard.println("notepad.exe");
  keyboard.press(KEY_RETURN);
  keyboard.release(KEY_RETURN);

  Serial.println("Done.");
}

void loop() {
  // Nothing to do here
}
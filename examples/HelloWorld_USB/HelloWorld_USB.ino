#include <Arduino.h>
#include <HIDForge.h> // Correct main header

UsbHid keyboard;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize USB HID with the US layout
  keyboard.begin(KeyboardLayout_en_US);
  USB.begin(); // <-- IMPORTANT: Start the USB stack
  
  Serial.println("Starting USB Keyboard Example...");

  // Wait for the USB device to be connected and ready
  while(!keyboard.isConnected()) {
    delay(100);
  }
  
  Serial.println("USB HID Connected. Typing in 3 seconds...");
  delay(3000); // Give user time to focus a text editor

  // Type a message
  keyboard.println("Hello from HIDForge!");
  
  // Use modifier keys to open the Run dialog on Windows (Win + R)
  keyboard.press(KEY_LEFT_GUI);
  keyboard.press('r');
  keyboard.releaseAll();
  
  delay(500);
  
  // Type a command and press Enter
  keyboard.println("notepad.exe");
  keyboard.write(KEY_RETURN); // Use write() for a single press-and-release

  Serial.println("Done.");
}

void loop() {
  // Nothing to do here
}
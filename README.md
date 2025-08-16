# BruceBadUSB Library

BruceBadUSB is a powerful and flexible library for the ESP32, designed to create USB and Bluetooth LE (BLE) Human Interface Devices (HID), commonly known as "Bad USBs". Extracted and refactored from the versatile [Bruce project](https://github.com/pr3y/Bruce), this library provides a clean, object-oriented interface for emulating keyboards over both wired and wireless connections.

## Features

- **Dual Mode:** Supports both USB and BLE HID keyboard emulation.
- **Object-Oriented:** Clean `UsbHid` and `BleHid` classes inheriting from a common `HIDInterface`.
- **Multi-Language Support:** Includes a wide range of keyboard layouts (US, UK, DE, FR, ES, IT, and more).
- **Rich Keystroke Support:** Emulate printing characters, modifier keys (Ctrl, Alt, Shift, GUI), and media keys.
- **DuckyScript Ready:** Easily integrate with DuckyScript interpreters for automated keystroke injection attacks (see examples).
- **Self-Contained:** The USB implementation includes its own TinyUSB driver wrapper, minimizing external dependencies.

## Library Structure

- `src/`: Contains all the source code for the library.
  - `BruceBadUSB.h`: Main header file to include in your project.
  - `UsbHid.h`/`.cpp`: Class for USB HID keyboard emulation.
  - `BleHid.h`/`.cpp`: Class for BLE HID keyboard emulation.
  - `HIDInterface.h`: Abstract base class defining the common keyboard API.
  - `layouts/`: Keyboard layout definitions.
  - `common/`: Shared key code definitions.
- `examples/`: Ready-to-use examples.
  - `HelloWorld_USB`: A simple example to type "Hello World!" over USB.
  - `DuckyScript_USB`: A DuckyScript player that reads commands from a string and executes them over USB.
  - `DuckyScript_BLE`: A DuckyScript player that reads commands from a string and executes them over BLE.

## Quick Start

### 1. Installation

Place the `BruceBadUSB` folder into your PlatformIO `lib` directory. PlatformIO will automatically detect the library and its dependencies (`h2zero/NimBLE-Arduino`).

### 2. Basic USB Keyboard Example

This example demonstrates how to type a simple message using the `UsbHid` class.

```cpp
#include <Arduino.h>
#include <BruceBadUSB.h>

UsbHid keyboard;

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial to be ready

  // Use the default US keyboard layout
  keyboard.begin(KeyboardLayout_en_US);

  Serial.println("Starting USB Keyboard Test...");

  // Wait until the USB device is connected and ready
  while(!keyboard.isConnected()) {
    delay(100);
  }
  
  Serial.println("Typing...");

  delay(2000); // Give yourself time to open a text editor
  
  keyboard.println("Hello from BruceBadUSB!");
  keyboard.press(KEY_LEFT_GUI);
  keyboard.press('r');
  keyboard.releaseAll();
  delay(500);
  keyboard.println("notepad.exe");
  keyboard.press(KEY_RETURN);
  keyboard.release(KEY_RETURN);
}

void loop() {
  // Nothing to do here for this example
}
```

## Disclaimer

This library is intended for educational purposes, security research, and professional testing only. Unauthorized use of this software to access or damage computer systems is illegal. The authors are not responsible for any misuse of this library.
```
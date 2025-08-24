# HIDForge: The ESP32 HID Emulation Library

[![GitHub stars](https://img.shields.io/github/stars/Meshwa428/HIDForge?style=social)](https://github.com/Meshwa428/HIDForge/stargazers)

**HIDForge** is a powerful and flexible Arduino library for the ESP32, designed for creating custom Human Interface Devices (HID). Emulate keyboards and mice over both **USB** and **Bluetooth LE (BLE)** with a clean, object-oriented API.

This library now also supports creating **composite USB devices**, allowing you to combine HID functionality with a **Mass Storage Class (MSC)** device to access an SD card directly over USB.

Whether you're building a custom macro pad, an automation tool, an assistive device, or a security research tool, HIDForge provides the foundation you need to bring your project to life.

<br>

## Why Choose HIDForge?

-   ✅ **Easy to Use:** A simple, intuitive API gets your project running in minutes.
-   🔌 **Dual Connectivity:** Create devices that work over a wired **USB** connection or wirelessly with **Bluetooth LE**.
-   💾 **Composite Device Support:** Combine Keyboard, Mouse, and an SD Card Reader (MSC) into a single USB device.
-   🏛️ **Modern C++ Design:** Clean `UsbHid`, `BleHid`, `UsbMouse`, `BleMouse`, and `UsbMsc` classes make your code readable and maintainable.
-   🌍 **International Support:** Comes with a wide range of keyboard layouts (US, UK, DE, FR, ES, etc.) for global compatibility.
-   🖱️ **Full HID Emulation:** Offers complete control over keyboard (keystrokes, modifiers, media keys) and mouse (movement, clicks, scrolling) actions.
-   ⚡ **High Performance:** Built with efficient code and a self-contained TinyUSB driver for USB, ensuring responsive performance.

## Ideal For

*   **Custom Macro Pads:** For streaming, video editing, or programming.
*   **System Automation:** Automate repetitive tasks on any computer.
*   **"BadUSB" devices with payloads** stored on an accessible SD card.
*   **Data Loggers** that can present their data as a simple USB drive.
*   **Security Research:** Simulate keyboard and mouse actions for advanced scenarios.

## Quick Start

### 1. Installation

**PlatformIO (Recommended):**
Add `HIDForge` to your `platformio.ini` `lib_deps` or install via the PlatformIO CLI. The library and its dependencies (`h2zero/NimBLE-Arduino`) will be handled automatically.
```ini
lib_deps = https://github.com/Meshwa428/HIDForge.git
```

**Arduino IDE:**
1.  Download the latest release ZIP file from the [Releases](https://github.com/Meshwa428/HIDForge/releases) page.
2.  In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library...` and select the downloaded file.
3.  Install the `NimBLE-Arduino` library from the Library Manager.
4.  Ensure you have the latest ESP32 board package, which includes the `SD` and `FS` libraries.

### 2. Basic USB Keyboard Example

This example demonstrates how to create a USB keyboard that types a message and opens Notepad on Windows.

```cpp
#include <Arduino.h>
#include <HIDForge.h> // The main library header

// 1. Create an instance of the UsbHid class
UsbHid keyboard;

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial

  // 2. Initialize the keyboard with the US English layout
  keyboard.begin(KeyboardLayout_en_US);

  // 3. Start the USB stack
  USB.begin();

  Serial.println("Starting USB Keyboard Example...");

  // 4. Wait for the host computer to recognize the device
  while(!keyboard.isConnected()) {
    delay(100);
  }
  
  Serial.println("USB HID Connected. Typing in 3 seconds...");
  delay(3000); // Give yourself time to open a text editor

  // 5. Send keystrokes
  keyboard.println("Hello from HIDForge!");
  
  // Use modifier keys to open the Run dialog (Win + R)
  keyboard.press(KEY_LEFT_GUI); // Press the Windows/Command key
  keyboard.press('r');
  keyboard.releaseAll();        // Release all currently pressed keys
  
  delay(500);
  
  // Type a command and press Enter
  keyboard.println("notepad.exe");
  keyboard.write(KEY_RETURN); // write() is a press and immediate release
}

void loop() {
  // Nothing to do here for this example
}
```

### 3. Basic BLE Mouse Example

This example creates a wireless mouse that moves in a circle when a device is connected.

```cpp
#include <Arduino.h>
#include <HIDForge.h>

// 1. Create a BleManager to handle the BLE stack
BleManager bleManager;
BleMouse* mouse = nullptr;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 2. Setup the BleManager
  bleManager.setup();

  // 3. Start the mouse. This will start the BLE stack if not already running.
  mouse = bleManager.startMouse();

  Serial.println("Starting BLE Mouse Example...");
  Serial.println("Waiting for a device to connect and pair...");

  if(mouse) {
    while(!mouse->isConnected()) {
      delay(100);
    }
  }
}

void loop() {
  // 4. Check if a host is connected
  if (bleManager.isMouseConnected()) {
    // Move the mouse in a circle
    for (int i = 0; i < 360; i += 15) {
      int x = 20 * cos(i * PI / 180);
      int y = 20 * sin(i * PI / 180);
      mouse->move(x, y);
      delay(50);
    }
  }
}
```

Check out the `examples/` directory for more, including the new **Composite_HID_MSC** example!

## API Reference

(A more detailed API reference can be found in the project's documentation)

### Core Classes
-   `BleManager`: For managing the BLE keyboard and mouse lifecycle.
-   `UsbHid` / `BleHid`: For keyboard emulation over USB or BLE. The `BleHid` class is now an adapter for `BleKeyboard`.
-   `BleKeyboard`: A self-contained BLE keyboard implementation.
-   `UsbMouse` / `BleMouse`: For mouse emulation over USB or BLE.
-   `UsbMsc`: For SD card mass storage emulation over USB.

### Key Methods

-   `begin()`: Initializes the HID device. For keyboards, you can pass a layout (e.g., `KeyboardLayout_de_DE`). For `UsbMsc`, you pass a pointer to an `SDCard` object.
-   `USB.begin()`: **(New)** After configuring all desired USB components (`UsbHid`, `UsbMouse`, `UsbMsc`), call this once to start the USB stack.
-   `isConnected()`: Returns `true` if a host computer is connected.
-   `press(KEY_CODE)`: Presses and holds a key (e.g., `KEY_LEFT_CTRL`, `'a'`).
-   `release(KEY_CODE)`: Releases a key.
-   `releaseAll()`: Releases all currently held keys.
-   `write(KEY_CODE)`: Presses and immediately releases a key.
-   `print("...")` / `println("...")`: Types a string of text.
-   `move(x, y, wheel, hWheel)`: Moves the mouse cursor and scrolls the wheel.

A full list of key codes can be found in `src/common/keys.h`.

### Using the `BleManager`

The `BleManager` is a new class that simplifies the management of the BLE stack. It runs the BLE stack in a separate FreeRTOS task, and it handles the initialization and de-initialization of the NimBLE stack.

**Starting and Stopping Devices:**
```cpp
#include <HIDForge.h>

BleManager bleManager;

void setup() {
  bleManager.setup();

  // Start the keyboard. This will also start the BLE stack if not already running.
  BleKeyboard* keyboard = bleManager.startKeyboard();

  // Start the mouse.
  BleMouse* mouse = bleManager.startMouse();
}

void loop() {
  // ... use keyboard and mouse objects

  // Stop the keyboard
  bleManager.stopKeyboard();

  // Stop the mouse. This will also stop the BLE stack as it's the last device.
  bleManager.stopMouse();
}
```

## Contributing

Contributions are welcome! Whether it's adding a new keyboard layout, fixing a bug, or improving documentation, please feel free to open a pull request or issue.

## Acknowledgements

HIDForge is built by extracting HID code from the excellent work of the [Bruce project](https://github.com/pr3y/Bruce), from which its core HID logic was extracted and refactored. It also relies on the `NimBLE-Arduino` library for its Bluetooth capabilities and the `TinyUSB` stack for its USB implementation. The MSC functionality is based on work from the [ESP32-S3 SD Card Performance Tests](https://github.com/atomic14/esp32-s3-sdcard) project.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---
### Disclaimer
This library is a powerful tool intended for learning, professional testing, and creative projects. Misuse of this software to access or damage computer systems without authorization is illegal. The authors are not responsible for any damage or illegal activities caused by the use of this library.
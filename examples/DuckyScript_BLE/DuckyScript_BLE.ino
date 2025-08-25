#include <Arduino.h>
#include <HIDForge.h> // Use the main library header

BleManager bleManager;

// A simple DuckyScript to be executed
const char* duckyScript = R"SCRIPT(
DELAY 1000
GUI r
DELAY 500
STRING notepad.exe
ENTER
DELAY 1000
STRING Hello from a BLE DuckyScript!
ENTER
STRING This script is running over Bluetooth.
ENTER
ALT F4
DELAY 500
STRING n
)SCRIPT";

void parseDuckyScript(HIDInterface& hid, const char* script) {
    String scriptStr(script);
    int start = 0;
    int end = scriptStr.indexOf('\n');

    while (start < scriptStr.length()) {
        if (end == -1) end = scriptStr.length();
        
        String line = scriptStr.substring(start, end);
        line.trim();

        if (line.length() > 0) {
            Serial.println("Executing: " + line);
            int spaceIndex = line.indexOf(' ');
            String command = (spaceIndex != -1) ? line.substring(0, spaceIndex) : line;
            String argument = (spaceIndex != -1) ? line.substring(spaceIndex + 1) : "";

            if (command == "STRING") {
                hid.print(argument);
            } else if (command == "DELAY") {
                delay(argument.toInt());
            } else if (command == "ENTER") {
                hid.write(KEY_RETURN);
            } else if (command == "GUI") {
                hid.press(KEY_LEFT_GUI);
                if (argument.length() > 0) hid.press(argument[0]);
                hid.releaseAll();
            } else if (command == "CTRL-SHIFT") {
                hid.press(KEY_LEFT_CTRL);
                hid.press(KEY_LEFT_SHIFT);
                if (argument.length() > 0) hid.press(argument[0]);
                hid.releaseAll();
            } else if (command == "ALT") {
                hid.press(KEY_LEFT_ALT);
                if (argument.length() > 0) hid.press(argument[0]);
                hid.releaseAll();
            } else {
                // Simplified: check for single key commands
                if(command == "ESC") hid.write(KEY_ESC);
                if(command == "F4") hid.write(KEY_F4);
            }
        }
        
        start = end + 1;
        end = scriptStr.indexOf('\n', start);
    }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  bleManager.setup();
  BleKeyboard* keyboard = bleManager.startKeyboard();

  if (keyboard) {
    BleHid hid(keyboard);
    hid.begin(KeyboardLayout_en_US);

    Serial.println("Starting DuckyScript over BLE...");
    Serial.println("Waiting for a device to connect...");

    while(!hid.isConnected()) {
        delay(100);
    }

    Serial.println("BLE Keyboard Connected. Running script in 3 seconds...");
    delay(3000);

    parseDuckyScript(hid, duckyScript);

  } else {
    Serial.println("Failed to start keyboard");
  }


  Serial.println("Script finished.");
}

void loop() {}
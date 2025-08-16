#include <Arduino.h>
#include <HIDForge.h>
#include <SPI.h>
#include <SD.h>

// --- IMPORTANT ---
// Please define your board's SD Card pins here.
// The example pins below are for a generic ESP32-S3 module.
// Using an incorrect pinout will prevent the SD card from working.

// For SPI Mode
#define SD_CS   21
#define SD_SCK  7
#define SD_MISO 8
#define SD_MOSI 9

// For SDIO/SD_MMC Mode (usually much faster)
// #define SD_CLK  GPIO_NUM_36
// #define SD_CMD  GPIO_NUM_35
// #define SD_D0   GPIO_NUM_37
// #define SD_D1   GPIO_NUM_38
// #define SD_D2   GPIO_NUM_33
// #define SD_D3   GPIO_NUM_34

UsbHid keyboard;
UsbMsc storage;
SDCard* card = nullptr;

void setup() {
  Serial.begin(115200);
  // Wait for serial monitor to connect
  while (!Serial);
  delay(1000); 

  Serial.println("Starting Composite HID+MSC Example...");

  // --- Initialize SD card ---
  // Choose ONE of the following methods (SPI or SDIO)

  // Method 1: Initialize SD card using SPI (most common)
  card = new SDCardArduino(Serial, "/sd", SD_MISO, SD_MOSI, SD_SCK, SD_CS);

  // Method 2: Initialize SD card using SDIO/SD_MMC (for better performance)
  // To use this, you must also add "-DUSE_SDIO=1" to your build_flags in platformio.ini
  // card = new SDCardMultiSector(Serial, "/sd", SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);

  if (card == nullptr || card->getSectorCount() == 0) {
    Serial.println("SD Card initialization failed. Halting.");
    while(1);
  }

  card->printCardInfo();

  // 1. Begin individual USB components
  keyboard.begin(KeyboardLayout_en_US);
  storage.begin(card);

  // 2. Start the USB stack with all configured components
  USB.begin();

  Serial.println("USB Composite device started.");

  // Wait for the host to connect to the keyboard
  while(!keyboard.isConnected()) {
    delay(100);
  }

  Serial.println("USB HID Connected. Typing in 5 seconds...");
  Serial.println("The SD card should also be visible as a USB drive.");
  delay(5000);

  // Send some keystrokes
  keyboard.println("Hello from a composite HID+MSC device!");
  keyboard.println("You should see this text AND a USB drive containing the SD card's contents.");
  
  Serial.println("Done.");
}

void loop() {
  // Nothing to do in the loop for this example
  delay(1000);
}
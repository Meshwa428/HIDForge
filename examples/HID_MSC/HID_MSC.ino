#include <Arduino.h>
#include <HIDForge.h>
#include <SPI.h>
#include <SD.h>

// For SPI Mode
#define SD_CS   GPIO_NUM_21
#define SD_SCK  GPIO_NUM_7
#define SD_MISO GPIO_NUM_8
#define SD_MOSI GPIO_NUM_9

// For SDIO/SD_MMC Mode (usually much faster)
// #define SD_CLK  GPIO_NUM_36
// #define SD_CMD  GPIO_NUM_35
// #define SD_D0   GPIO_NUM_37
// #define SD_D1   GPIO_NUM_38
// #define SD_D2   GPIO_NUM_33
// #define SD_D3   GPIO_NUM_34

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
  storage.begin(card);

  USB.begin();

  Serial.println("USB device started.");
}

void loop() {
  // Nothing to do in the loop for this example
  delay(1000);
}
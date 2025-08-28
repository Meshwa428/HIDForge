#include "msc/SDCardArduino.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

SDCardArduino::SDCardArduino(Stream &debug, const char *mount_point, gpio_num_t cs)
    : SDCard(debug, mount_point)
{
  static SPIClass spi(HSPI);
  if (SD.begin(cs, spi, 80000000, mount_point))
  {
    debug.println("SD card initialized");
    m_sector_size = SD.sectorSize();
    m_sector_count = SD.numSectors();
  }
  else
  {
    debug.println("SD card initialization failed");
  }
}

SDCardArduino::~SDCardArduino()
{
  SD.end();
}

void SDCardArduino::printCardInfo()
{
  if (SD.cardType() == CARD_NONE)
  {
    m_debug.println("No SD card attached");
    return;
  }
  m_debug.printf("Card type: %d\n", SD.cardType());
  m_debug.printf("Card size: %lluMB\n", SD.cardSize() / (1024 * 1024));
}

bool SDCardArduino::writeSectors(uint8_t *src, size_t start_sector, size_t sector_count)
{
  bool res = true;
  for (int i = 0; i < sector_count; i++)
  {
    res = SD.writeRAW((uint8_t *)src, start_sector + i);
    if (!res)
    {
      break;
    }
    src += m_sector_size;
  }
  return res;
}

bool SDCardArduino::readSectors(uint8_t *dst, size_t start_sector, size_t sector_count)
{
  bool res = true;
  for (int i = 0; i < sector_count; i++)
  {
    res = SD.readRAW((uint8_t *)dst, start_sector + i);
    if (!res)
    {
      break;
    }
    dst += m_sector_size;
  }
  return res;
}
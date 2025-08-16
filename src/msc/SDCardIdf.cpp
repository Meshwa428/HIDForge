#include "msc/SDCardIdf.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#define SPI_DMA_CHAN SPI_DMA_CH_AUTO

SDCardIdf::SDCardIdf(Stream &debug, const char *mount_point, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
: SDCard(debug, mount_point) 
{
  m_mutex = xSemaphoreCreateMutex();
  m_host.max_freq_khz = SDMMC_FREQ_52M;
  m_host.flags = SDMMC_HOST_FLAG_4BIT;
  esp_err_t ret;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024};

  m_debug.println("Initializing SD card (SDIO)");

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
  slot_config.width = 4;
  slot_config.clk = clk;
  slot_config.cmd = cmd;
  slot_config.d0 = d0;
  slot_config.d1 = d1;
  slot_config.d2 = d2;
  slot_config.d3 = d3;
  ret = esp_vfs_fat_sdmmc_mount(m_mount_point.c_str(), &m_host, &slot_config, &mount_config, &m_card);
  if (ret != ESP_OK)
  {
      m_debug.printf("Failed to initialize the card (%s).\n", esp_err_to_name(ret));
    return;
  }
  m_debug.printf("SDCard mounted at: %s\n", m_mount_point.c_str());
  m_sector_count = m_card->csd.capacity;
  m_sector_size = m_card->csd.sector_size;
}

SDCardIdf::SDCardIdf(Stream &debug, const char *mount_point, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
: SDCard(debug, mount_point) 
{
  m_mutex = xSemaphoreCreateMutex();
  m_host.max_freq_khz = SDMMC_FREQ_52M;
  esp_err_t ret;
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024};

  m_debug.println("Initializing SD card (SPI)");

  spi_bus_config_t bus_cfg = {
      .mosi_io_num = mosi,
      .miso_io_num = miso,
      .sclk_io_num = clk,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 16384,
  };
  ret = spi_bus_initialize(spi_host_device_t(m_host.slot), &bus_cfg, SPI_DMA_CHAN);
  if (ret != ESP_OK)
  {
    m_debug.println("Failed to initialize SPI bus.");
    return;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = cs;
  slot_config.host_id = spi_host_device_t(m_host.slot);

  ret = esp_vfs_fat_sdspi_mount(m_mount_point.c_str(), &m_host, &slot_config, &mount_config, &m_card);
  if (ret != ESP_OK)
  {
      m_debug.printf("Failed to initialize the card (%s).\n", esp_err_to_name(ret));
    return;
  }
  m_debug.printf("SDCard mounted at: %s\n", m_mount_point.c_str());
  m_sector_count = m_card->csd.capacity;
  m_sector_size = m_card->csd.sector_size;
}

SDCardIdf::~SDCardIdf()
{
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  esp_vfs_fat_sdcard_unmount(m_mount_point.c_str(), m_card);
  spi_bus_free(spi_host_device_t(m_host.slot));
  xSemaphoreGive(m_mutex);
  vSemaphoreDelete(m_mutex);
}

void SDCardIdf::printCardInfo()
{
  sdmmc_card_print_info(stdout, m_card);
}
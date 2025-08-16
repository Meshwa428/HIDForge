#include "msc/SDCardLazyWrite.h"
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
#include "sdmmc_cmd.h"

enum class RequestType {
  READ,
  WRITE
};

class Request {
public:
  Request(RequestType type, void *data, size_t start_sector, size_t sector_count)
  : m_type(type), m_start_sector(start_sector), m_sector_count(sector_count) {
    if (type == RequestType::WRITE) {
      m_data = malloc(sector_count * 512);
      memcpy(m_data, data, sector_count * 512);
    } else if(type == RequestType::READ) {
      m_data = data;
    } else {
      m_data = NULL;
    }
  }
  ~Request() {
    if (m_type == RequestType::WRITE) {
      free(m_data);
    }
  }
  RequestType m_type;
  void *m_data;
  size_t m_start_sector;
  size_t m_sector_count;
};

SDCardLazyWrite::SDCardLazyWrite(Stream &debug, const char *mount_point, gpio_num_t miso, gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs)
: SDCardIdf(debug, mount_point, miso, mosi, clk, cs)
{
  m_request_queue = xQueueCreate(10, sizeof(Request *));
  m_read_queue = xQueueCreate(10, sizeof(Request *));
  xTaskCreate([](void *param) {
    SDCardLazyWrite *card = (SDCardLazyWrite *)param;
    card->drainQueue();
  }
  , "SDCardWriter", 4096, this, 1, NULL);
}

SDCardLazyWrite::SDCardLazyWrite(Stream &debug, const char *mount_point, gpio_num_t clk, gpio_num_t cmd, gpio_num_t d0, gpio_num_t d1, gpio_num_t d2, gpio_num_t d3)
: SDCardIdf(debug, mount_point, clk, cmd, d0, d1, d2, d3)
{
  m_request_queue = xQueueCreate(10, sizeof(Request *));
  m_read_queue = xQueueCreate(10, sizeof(Request *));
  xTaskCreate([](void *param) {
    SDCardLazyWrite *card = (SDCardLazyWrite *)param;
    card->drainQueue();
  }
  , "SDCardWriter", 4096, this, 1, NULL);
}

bool SDCardLazyWrite::writeSectors(uint8_t *src, size_t start_sector, size_t sector_count) {
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  Request *req = new Request(RequestType::WRITE, src, start_sector, sector_count);
  xQueueSend(m_request_queue, &req, portMAX_DELAY);
  xSemaphoreGive(m_mutex);
  return true;
}

void SDCardLazyWrite::drainQueue() {
  Request *req;
  while (xQueueReceive(m_request_queue, &req, portMAX_DELAY) == pdTRUE) {
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    if (req->m_type == RequestType::WRITE) {
      sdmmc_write_sectors(m_card, req->m_data, req->m_start_sector, req->m_sector_count);
      delete req;
    } else if (req->m_type == RequestType::READ) {
      sdmmc_read_sectors(m_card, req->m_data, req->m_start_sector, req->m_sector_count);
      xQueueSend(m_read_queue, &req, portMAX_DELAY);
    }
    xSemaphoreGive(m_mutex);
  }
}

bool SDCardLazyWrite::readSectors(uint8_t *dst, size_t start_sector, size_t sector_count) {
  xSemaphoreTake(m_mutex, portMAX_DELAY);
  if (uxQueueMessagesWaiting(m_request_queue) > 0) {
    Request *req = new Request(RequestType::READ, dst, start_sector, sector_count);
    xQueueSend(m_request_queue, &req, portMAX_DELAY);
    xQueueReceive(m_read_queue, &req, portMAX_DELAY);
    delete req;
  } else {
    sdmmc_read_sectors(m_card, dst, start_sector, sector_count);
  }
  xSemaphoreGive(m_mutex);
  return true;
}
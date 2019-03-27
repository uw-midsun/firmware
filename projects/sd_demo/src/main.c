#include <string.h>
#include "delay.h"
#include "ff.h"
#include "interrupt.h"
#include "log.h"
#include "sd_binary.h"
#include "soft_timer.h"
#include "spi.h"
#include "status.h"

SpiSettings spi_settings = { .cs = { .port = GPIO_PORT_A, .pin = 4 },
                             .sclk = { .port = GPIO_PORT_B, .pin = 13 },
                             .mosi = { .port = GPIO_PORT_B, .pin = 15 },  // DI
                             .miso = { .port = GPIO_PORT_B, .pin = 14 },  // DO
                             .mode = SPI_MODE_0,
                             .baudrate = 1200000 };

static void prv_test_callback(const Status *status) {
  printf("CODE: %d:%s:%s %s\n", status->code, status->source, status->caller, status->message);
}

int main(void) {
  LOG_DEBUG("Starting SD card demo\n");

  // Useful for debugging
  status_register_callback(prv_test_callback);

  if (!status_ok(gpio_init())) {
    LOG_CRITICAL("GPIO did not init\n");
    return 0;
  }

  interrupt_init();
  soft_timer_init();

  // Wait to make sure SD card has had enough time to power up
  delay_s(1);

  // Initialize SD card
  if (!status_ok(spi_init(SPI_PORT_2, &spi_settings))) {
    LOG_CRITICAL("Failed to initialize the Spi module\n");
    return 0;
  }

  //
  // Writing to SD card
  //

  FATFS FatFs = { 0 };
  FRESULT fr = { 0 };
  FIL fil = { 0 };
  char *line = "HELLO SD CARD";

  UINT btw = strlen(line);
  UINT written = 0;

  // Mount drive on SPI_PORT_2
  fr = f_mount(&FatFs, "1:", 1);

  if (fr != FR_OK) {
    LOG_CRITICAL("Could not mount SD card. FatFs error %d\n", fr);
    return 0;
  }

  // Open a text file on SPI_PORT_2
  fr = f_open(&fil, "1:message.txt", FA_WRITE | FA_CREATE_ALWAYS);

  if (fr != FR_OK) {
    LOG_CRITICAL("Could not open message.txt. FatFs error %d\n", fr);
    return 0;
  }

  for (uint32_t i = 0; i < 1; i++) {
    fr = f_write(&fil, line, btw, &written);
  }

  if (btw != written) {
    LOG_CRITICAL("Bytes written is not the same as write length (%d, %d)\n", (int)btw,
                 (int)written);
  }

  // Close the file
  fr = f_close(&fil);

  if (fr != FR_OK) {
    LOG_CRITICAL("Could not close file. FatFs error %d\n", fr);
    return 0;
  }

  //
  // Reading from SD card
  //

  uint32_t read_length = strlen(line);

  // Open the file just to read from it
  fr = f_open(&fil, "1:message.txt", FA_READ);

  if (fr != FR_OK) {
    LOG_CRITICAL("Could open file for read. FatFs error %d\n", fr);
    return 0;
  }

  const uint32_t to_read = 25;

  char read_line[to_read];
  memset(read_line, 0, to_read * sizeof(char));

  fr = f_read(&fil, read_line, to_read, &written);

  if (fr != FR_OK) {
    LOG_CRITICAL("Could read file. FatFs error %d\n", fr);
    return 0;
  }

  LOG_DEBUG("Read %d bytes from file: %s\n", (int)written, read_line);

  fr = f_mount(NULL, "1:", 0);

  if (fr != FR_OK) {
    LOG_CRITICAL("Could not unmount SD card. FatFs error %d\n", fr);
    return 0;
  }

  LOG_DEBUG("SD demo finished successfully\n");
  return 0;
}

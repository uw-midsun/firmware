// Test program for STM32F072RB discovery boards - attempts to connect to the onboard MEMS
// gyroscope over SPI. Blinks the blue LED on success or the red LED on fail.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "spi.h"
#include "log.h"

#define GYRO_ID 0xD4

void gyro_cmd(bool read, bool autoincrement, uint8_t addr, uint8_t *data) {
  uint8_t packet[] = { (read & 0x01) << 7 | (autoincrement & 0x01) << 6 | (addr & 0x3F),
                       (read) ? 0 : *data };

  spi_exchange(1, packet, (size_t)(2 - read), data, read);
}

int main(void) {
  gpio_init();

  SPISettings spi_settings = {
    .baudrate = 1500000,
    .mode = SPI_MODE_0,
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_C, 0 },
  };

  // Using SPI port 2 - not using enum so build on x86 will pass
  spi_init(1, &spi_settings);

  uint8_t whoami = 0xAA;
  gyro_cmd(true, false, 0x0F, &whoami);

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT, .state = GPIO_STATE_HIGH,
  };

  GPIOAddress leds[] = { { GPIO_PORT_C, 6 }, { GPIO_PORT_C, 7 } };
  gpio_init_pin(&leds[0], &led_settings);
  gpio_init_pin(&leds[1], &led_settings);

  while (true) {
    LOG_DEBUG("ID: %d\n", whoami);
    gpio_toggle_state(&leds[whoami == GYRO_ID]);

    // arbitrary software delay
    for (volatile int i = 0; i < 2000000; i++) {
    }
  }

  return 0;
}

// Test program for STM32F072RB discovery boards - attempts to connect to the onboard MEMS
// gyroscope over SPI. Blinks the blue LED on success or the red LED on fail.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "i2c.h"

int main(void) {
  gpio_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { GPIO_PORT_B, 8 },
    .sda = { GPIO_PORT_B, 9 }
  };
  i2c_init(I2C_PORT_1, &i2c_settings);

  printf("I2C initialized\n");

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  GPIOAddress leds[] = { { GPIO_PORT_C, 6 }, { GPIO_PORT_C, 7 } };
  gpio_init_pin(&leds[0], &led_settings);
  gpio_init_pin(&leds[1], &led_settings);

  while (true) {
    uint16_t rx_data = { 0 };
    i2c_read(I2C_PORT_1, 0x5A, 0x07, &rx_data, sizeof(rx_data));

    printf("0x%x: %d C\n", rx_data, (int16_t)(rx_data / 50) - 273);

    i2c_read(I2C_PORT_1, 0x5A, 0x06, &rx_data, sizeof(rx_data));

    printf("0x%x\n", rx_data);

    for (volatile int i = 0; i < 4000000; i++) { }
  }

  return 0;
}

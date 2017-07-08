// Test program for STM32F072RB discovery boards - attempts to connect to the onboard MEMS
// gyroscope over SPI. Blinks the blue LED on success or the red LED on fail.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "i2c.h"
#include "stm32f0xx.h"
#include "delay.h"
#include "soft_timer.h"
#include "interrupt.h"

#define I2C_MAG_ID 0x5E
#define I2C_GPIO_ID 0x20

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { GPIO_PORT_B, 8 },
    .sda = { GPIO_PORT_B, 9 }
  };
  i2c_init(I2C_PORT_1, &i2c_settings);

  printf("I2C initialized\n");

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };

  RCC_ClocksTypeDef rcc_clocks;
  RCC_GetClocksFreq(&rcc_clocks);

  printf("%d\n", rcc_clocks.I2C1CLK_Frequency);

  GPIOAddress leds[] = { { GPIO_PORT_B, 3 }, { GPIO_PORT_B, 4 } };
  gpio_init_pin(&leds[0], &led_settings);
  gpio_init_pin(&leds[1], &led_settings);

  uint8_t rx_mag[9] = { 0 };
  i2c_read(I2C_PORT_1, I2C_MAG_ID, rx_mag, SIZEOF_ARRAY(rx_mag));

  uint8_t mag_setup[] = {
    0x00, // RESERVED
    rx_mag[7] & 0x18 | 0x3, // MOD1
    rx_mag[8], // RESERVED
    rx_mag[9] & 0x1F // MOD2
  };
  i2c_write(I2C_PORT_1, I2C_MAG_ID, mag_setup, sizeof(mag_setup));
  delay_us(100);

  while (true) {
    uint8_t rx_data = 0;
    i2c_read_reg(I2C_PORT_1, I2C_GPIO_ID, 0x09, &rx_data, sizeof(rx_data));

    printf("IO %d\n", rx_data);

    i2c_read(I2C_PORT_1, I2C_MAG_ID, rx_mag, 7);

    int16_t x = (int16_t)(rx_mag[0] << 8 | rx_mag[4] & 0xF0) >> 4;
    int16_t y = (int16_t)(rx_mag[1] << 8 | (rx_mag[4] & 0x0F) << 4) >> 4;
    int16_t z = (int16_t)(rx_mag[2] << 8 | (rx_mag[5] & 0x0F) << 4) >> 4;

    printf("MAG %d %d %d\n", x, y, z);

    delay_ms(10);
  }

  return 0;
}

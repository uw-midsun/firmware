// Test program for STM32F072RB discovery boards - attempts to connect to the onboard MEMS
// gyroscope over SPI. Blinks the blue LED on success or the red LED on fail.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "i2c.h"
#include "stm32f0xx.h"
#include "delay.h"
#include "soft_timer.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define MAG_LSB_TO_mT(lsb) ((lsb) * 49 / 500)

int main(void) {
  gpio_init();
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

  uint8_t regs[] = {
    0xFF, // IODIR
    0x00, // IPOL
    0x00, // GPINTEN
    0x00, // DEFVAL
    0x00, // INTCON
    0x00, // IOCON
    0x00, // GPPU
    0x00, // INTF
    0x00, // INTCAP
    0x00, // GPIO
    0x00, // OLAT
  };

  for (volatile int i = 0; i < 500000; i++) { }

  memset(regs, 0, sizeof(regs));
  // i2c_read_reg(I2C_PORT_1, 0x20, 0x00, &regs, SIZEOF_ARRAY(regs));

  // for (int i = 0; i < SIZEOF_ARRAY(regs); i++) {
  //   printf("%d: 0x%x\n", i, regs[i]);
  // }

  uint8_t mag_setup[] = {
    0x00, // RESERVED
    0x01  // MOD1
  };
  i2c_write(I2C_PORT_1, 0x5E, mag_setup, 2);

  while (true) {
    uint8_t rx_data = 0;
    i2c_read_reg(I2C_PORT_1, 0x20, 0x09, &rx_data, sizeof(rx_data));

    printf("IO %d\n", rx_data);

    uint8_t rx_mag[7] = { 0 };
    i2c_read(I2C_PORT_1, 0x5E, rx_mag, SIZEOF_ARRAY(rx_mag));

    int16_t x = (int16_t)(rx_mag[0] << 8 | rx_mag[4] & 0xF0) >> 4;
    int16_t y = (int16_t)(rx_mag[1] << 8 | (rx_mag[4] & 0x0F) << 4) >> 4;
    int16_t z = (int16_t)(rx_mag[2] << 8 | (rx_mag[5] & 0x0F) << 4) >> 4;

    printf("MAG x %d y %d z %d\n", x, y, z);

    delay_ms(100);
  }

  return 0;
}

#include <stdbool.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "i2c.h"
#include "i2c_mcu.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"

static const I2CSettings s_settings = { .speed = I2C_SPEED_FAST,
                                        .sda = { .port = GPIO_PORT_B, .pin = 11 },
                                        .scl = { .port = GPIO_PORT_B, .pin = 10 } };

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  i2c_init(I2C_PORT_2, &s_settings);
  uint8_t d1[] = { 0x5A };
  // uint8_t d2[] = { 0xFF };

  while (true) {
    i2c_write(I2C_PORT_2, 0xAA, d1, SIZEOF_ARRAY(d1));
    i2c_write(I2C_PORT_2, 0x55, d1, SIZEOF_ARRAY(d1));
    // i2c_write(I2C_PORT_2, 8, d2, SIZEOF_ARRAY(d2));
    delay_ms(50);
  }
  return 0;
}

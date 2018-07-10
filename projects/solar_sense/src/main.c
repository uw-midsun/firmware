#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"

#define SOLAR_SENSE_I2C_BUS_SDA \
  { GPIO_PORT_B, 11 }

#define SOLAR_SENSE_I2C_BUS_SCL \
  { GPIO_PORT_B, 10 }


int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();
  event_queue_init();

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,     //
    .sda = SOLAR_SENSE_I2C_BUS_SDA,  //
    .scl = SOLAR_SENSE_I2C_BUS_SCL,  //
  };

  i2c_init(I2C_PORT_2, &i2c_settings);

  while (true) {
  }
  return 0;
}

#include <stdint.h>
#include <stdio.h>

#include "ads1015.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"

int main() {
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { GPIO_PORT_B, 10 },
    .sda = { GPIO_PORT_B, 11 }
  };

  gpio_init();
  interrupt_init();
  gpio_it_init();

  i2c_init(I2C_PORT_2, &settings);

  GPIOAddress address = { GPIO_PORT_B, 2 };

  StatusCode status = ads1015_init(I2C_PORT_2, address);
  printf("i2c %s\n", status_ok(status) ? "initialized" : "uninitialized");

  int16_t data[NUM_ADS1015_CHANNELS];

  while (1) {/*
    for (uint8_t i = 0; i < NUM_ADS1015_CHANNELS; i++) {
      ads1015_read_raw(i, &data[i]);
    }
    printf("[ %d\t%d\t%d\t%d ]\n",
          data[ADS1015_CHANNEL_0],
          data[ADS1015_CHANNEL_1],
          data[ADS1015_CHANNEL_2],
          data[ADS1015_CHANNEL_3]);*/
  }
}

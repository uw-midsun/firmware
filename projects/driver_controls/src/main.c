#include <stdio.h>
#include <stdint.h>

#include "gpio.h"
#include "magnetic_sensor.h"

#define I2C_PORT_1 0

int main() {
  gpio_init();

  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 }
  };

  i2c_init(I2C_PORT_1, &settings);

  magnetic_sensor_init(I2C_PORT_1);

  int16_t readings[3];

  while (1) {
    magnetic_sensor_read_data(I2C_PORT_1, readings);

    printf("x = % 4d\ty = % 4d\tz = % 4d\n", readings[0], readings[1], readings[2]);
  }
}

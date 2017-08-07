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

  int8_t readings[3];

  while (1) {
    readings[0] = magnetic_sensor_read_data(I2C_PORT_1, MAGNETIC_SENSOR_PROBE_BX);
    readings[1] = magnetic_sensor_read_data(I2C_PORT_1, MAGNETIC_SENSOR_PROBE_BY);
    readings[2] = magnetic_sensor_read_data(I2C_PORT_1, MAGNETIC_SENSOR_PROBE_BZ);

    printf("x = %d | y = %d | z = %d\n", readings[0], readings[1], readings[2]);
  }
}

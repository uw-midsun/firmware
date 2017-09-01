#pragma once

#include "gpio.h"

typedef enum {
  I2C_PORT_1 = 0,
  I2C_PORT_2,
  NUM_I2C_PORTS,
} I2CPort;

#define I2C_PORT_1_GPIO_ALTFN GPIO_ALTFN_1
#define I2C_PORT_2_PB_10_11_GPIO_ALTFN GPIO_ALTFN_1
#define I2C_PORT_2_PB_13_14_GPIO_ALTFN GPIO_ALTFN_5

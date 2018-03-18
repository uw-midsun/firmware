#pragma once

#include "can.h"
#include "gpio.h"

#define LIGHTS_CAN_RX_ADDR \
  { GPIO_PORT_B, 6 }
#define LIGHTS_CAN_TX_ADDR \
  { GPIO_PORT_B, 7 }

// declaration of all the configuration variables
const CANSettings can_settings_front;
const CANSettings can_settings_rear;

// TODO(ELEC-376): since module initialization needs board type info (front or back), it will be
// implemented in lights_gpio ticket
// StatusCode init_lights_modules(void);

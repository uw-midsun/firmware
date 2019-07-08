#pragma once

// Pedal Pin Mapping
#define PEDAL_CONFIG_PIN_I2C_SDL \
  { GPIO_PORT_B, 10 }
#define PEDAL_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }
#define PEDAL_CONFIG_PIN_ADS1015_READY \
  { GPIO_PORT_B, 2 }

// CAN
#define PEDAL_CONFIG_PIN_CAN_TX \
  { GPIO_PORT_A, 12 }
#define PEDAL_CONFIG_PIN_CAN_RX \
  { GPIO_PORT_A, 11 }

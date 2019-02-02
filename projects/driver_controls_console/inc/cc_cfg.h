#pragma once
// Console controls config file

// #define CC_CFG_DEBUG_PRINT_EVENTS

#define CC_CFG_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_CONSOLE_CONTROLS
#define CC_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS

#define CC_CFG_CAN_RX \
  { GPIO_PORT_A, 11 }
#define CC_CFG_CAN_TX \
  { GPIO_PORT_A, 12 }

#define DC_CFG_I2C_BUS_PORT I2C_PORT_1
#define DC_CFG_I2C_BUS_SDA \
  { GPIO_PORT_B, 11 }
#define DC_CFG_I2C_BUS_SCL \
  { GPIO_PORT_B, 10 }

#define CC_CFG_CONSOLE_IO_ADDR GPIO_EXPANDER_ADDRESS_1

// Console control IO pins
#define CC_CFG_CONSOLE_POWER_PIN \
  { GPIO_PORT_B, 0 }
#define CC_CFG_CONSOLE_DIRECTION_DRIVE_PIN \
  { GPIO_PORT_A, 5 }
#define CC_CFG_CONSOLE_DIRECTION_NEUTRAL_PIN \
  { GPIO_PORT_A, 6 }
#define CC_CFG_CONSOLE_DIRECTION_REVERSE_PIN \
  { GPIO_PORT_A, 7 }
#define CC_CFG_CONSOLE_DRL_PIN \
  { GPIO_PORT_A, 4 }
#define CC_CFG_CONSOLE_LOWBEAMS_PIN \
  { GPIO_PORT_A, 0 }
#define CC_CFG_CONSOLE_HAZARDS_PIN \
  { GPIO_PORT_A, 1 }

// LED IO-Expander Pins
#define CC_CFG_PWR_LED GPIO_EXPANDER_PIN_1
#define CC_CFG_REVERSE_LED GPIO_EXPANDER_PIN_2
#define CC_CFG_NEUTRAL_LED GPIO_EXPANDER_PIN_3
#define CC_CFG_DRIVE_LED GPIO_EXPANDER_PIN_4
#define CC_CFG_DRL_LED GPIO_EXPANDER_PIN_5
#define CC_CFG_LOW_BEAM_LED GPIO_EXPANDER_PIN_6
#define CC_CFG_HAZARD_LED GPIO_EXPANDER_PIN_7

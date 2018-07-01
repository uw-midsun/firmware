#pragma once
// Driver controls config file

// #define DC_CFG_DEBUG_PRINT_EVENTS
// #define DC_CFG_DISABLE_CENTER_CONSOLE
// #define DC_CFG_DISABLE_CONTROL_STALK
// #define DC_CFG_DISABLE_PEDAL

#define DC_CFG_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_DRIVER_CONTROLS
#define DC_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS
#define DC_CFG_CAN_RX \
  { GPIO_PORT_A, 11 }
#define DC_CFG_CAN_TX \
  { GPIO_PORT_A, 12 }

#define DC_CFG_I2C_BUS_PORT I2C_PORT_1
#define DC_CFG_I2C_BUS_SDA \
  { GPIO_PORT_B, 9 }
#define DC_CFG_I2C_BUS_SCL \
  { GPIO_PORT_B, 8 }

// Center Console
#define DC_CFG_CONSOLE_IO_INT_PIN \
  { GPIO_PORT_A, 9 }
#define DC_CFG_CONSOLE_IO_ADDR GPIO_EXPANDER_ADDRESS_1

// Control Stalk
#define DC_CFG_STALK_IO_INT_PIN \
  { GPIO_PORT_A, 2 }
#define DC_CFG_STALK_IO_ADDR GPIO_EXPANDER_ADDRESS_0
#define DC_CFG_STALK_ADC_RDY_PIN \
  { GPIO_PORT_A, 1 }
#define DC_CFG_STALK_ADC_ADDR ADS1015_ADDRESS_VDD

// Pedal
#define DC_CFG_PEDAL_ADC_RDY_PIN \
  { GPIO_PORT_A, 10 }
#define DC_CFG_PEDAL_ADC_ADDR ADS1015_ADDRESS_GND

#define DC_CFG_STEERING_ADC_RDY_PIN \
  { GPIO_PORT_A, 0 }

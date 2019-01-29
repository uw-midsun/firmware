#pragma once
// Pedal controls config file

// #define PC_CFG_DEBUG_PRINT_EVENTS

#define PC_CFG_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_PEDAL_CONTROLS
#define PC_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS

#define PC_CFG_CAN_RX \
  { GPIO_PORT_A, 11 }
#define PC_CFG_CAN_TX \
  { GPIO_PORT_A, 12 }

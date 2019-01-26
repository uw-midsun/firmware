#pragma once
// Console controls config file

#define CC_CFG_CAN_DEVICE_ID SYSTEM_CAN_DEVICE_CONSOLE_CONTROLS
#define CC_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS

#define CC_CFG_CAN_RX \
  { GPIO_PORT_A, -1 }  // Need to update this when the boards arrive
#define CC_CFG_CAN_TX \
  { GPIO_PORT_A, -1 }  // Need to update this when the boards arrive

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

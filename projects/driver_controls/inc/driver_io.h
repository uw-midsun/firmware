#pragma once

#define DRIVER_IO_GET_PIN(address) ((GPIOAddress)address.pin)
#define DRIVER_IO_NUM_ADDRESSES 12

// Digital address definitions

#define DRIVER_IO_POWER_SWITCH                                                 \
  { GPIO_PORT_C, 0 }
#define DRIVER_IO_DIR_SELECT_FORWARD                                           \
  { GPIO_PORT_B, 2 }
#define DRIVER_IO_DIR_SELECT_REVERSE                                           \
  { GPIO_PORT_B, 3 }
#define DRIVER_IO_CRUISE_CONTROL_PORT                                          \
  { GPIO_PORT_C, 4 }
#define DRIVER_IO_CRUISE_CONTROL_INC                                           \
  { GPIO_PORT_C, 5 }
#define DRIVER_IO_CRUISE_CONTROL_DEC                                           \
  { GPIO_PORT_C, 6 }
#define DRIVER_IO_TURN_SIGNAL_RIGHT                                            \
  { GPIO_PORT_C, 7 }
#define DRIVER_IO_TURN_SIGNAL_LEFT                                             \
  { GPIO_PORT_C, 8 }
#define DRIVER_IO_HAZARD_LIGHT                                                 \
  { GPIO_PORT_C, 9 }
#define DRIVER_IO_HORN                                                         \
  { GPIO_PORT_C, 10 }
#define DRIVER_IO_PUSH_TO_TALK                                                 \
  { GPIO_PORT_C, 11 }

// Analog address definitions

#define DRIVER_IO_GAS_PEDAL                                                    \
  { GPIO_PORT_A, 0 }
#define DRIVER_IO_MECHANICAL_BRAKE                                             \
  { GPIO_PORT_A, 1 }

// Driver IO pin definitions
#define DRIVER_IO_POWER_SWITCH_PIN DRIVER_IO_GET_PIN(DRIVER_IO_POWER_SWITCH)
#define DRIVER_IO_DIR_SELECT_PIN_FORWARD                                       \
  DRIVER_IO_GET_PIN(DRIVER_IO_DIR_SELECT_FORWARD)
#define DRIVER_IO_DIR_SELECT_PIN_REVERSE                                       \
  DRIVER_IO_GET_PIN(DRIVER_IO_DIR_SELECT_REVERSE)
#define DRIVER_IO_CRUISE_CONTROL_PIN                                           \
  DRIVER_IO_GET_PIN(DRIVER_IO_CRUISE_CONTROL_PORT)
#define DRIVER_IO_CRUISE_CONTROL_INC_PIN                                       \
  DRIVER_IO_GET_PIN(DRIVER_IO_CRUISE_CONTROL_INC)
#define DRIVER_IO_CRUISE_CONTROL_DEC_PIN                                       \
  DRIVER_IO_GET_PIN(DRIVER_IO_CRUISE_CONTROL_DEC)
#define DRIVER_IO_TURN_SIGNAL_PIN_RIGHT                                        \
  DRIVER_IO_GET_PIN(DRIVER_IO_TURN_SIGNAL_RIGHT)
#define DRIVER_IO_TURN_SIGNAL_PIN_LEFT                                         \
  DRIVER_IO_GET_PIN(DRIVER_IO_TURN_SIGNAL_LEFT)
#define DRIVER_IO_HAZARD_LIGHT_PIN DRIVER_IO_GET_PIN(DRIVER_IO_HAZARD_LIGHT)
#define DRIVER_IO_HORN_PIN DRIVER_IO_GET_PIN(DRIVER_IO_HORN)
#define DRIVER_IO_PUSH_TO_TALK_PIN DRIVER_IO_GET_PIN(DRIVER_IO_PUSH_TO_TALK)

#define DRIVER_IO_GAS_PEDAL_PIN DRIVER_IO_GET_PIN(DRIVER_IO_GAS_PEDAL)
#define DRIVER_IO_MECHANICAL_BRAKE_PIN                                         \
  DRIVER_IO_GET_PIN(DRIVER_IO_MECHANICAL_BRAKE)

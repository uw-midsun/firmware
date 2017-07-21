#pragma once

// Driver IO pin definitions

#define DRIVER_IO_POWER_SWITCH_PIN                  0
#define DRIVER_IO_DIRECTION_SELECTOR_PIN_FORWARD    2
#define DRIVER_IO_DIRECTION_SELECTOR_PIN_REVERSE    3
#define DRIVER_IO_CRUISE_CONTROL_PIN                4
#define DRIVER_IO_CRUISE_CONTROL_INC_PIN            5
#define DRIVER_IO_CRUISE_CONTROL_DEC_PIN            6
#define DRIVER_IO_TURN_SIGNAL_PIN_RIGHT             7
#define DRIVER_IO_TURN_SIGNAL_PIN_LEFT              8
#define DRIVER_IO_HAZARD_LIGHT_PIN                  9

#define DRIVER_IO_GAS_PEDAL_PIN                     0
#define DRIVER_IO_MECHANICAL_BRAKE_PIN              1

// Digital address definitions

#define DRIVER_IO_POWER_SWITCH                      { 2, 0 }
#define DRIVER_IO_DIRECTION_SELECTOR_FORWARD        { 1, 2 }
#define DRIVER_IO_DIRECTION_SELECTOR_REVERSE        { 1, 3 }
#define DRIVER_IO_CRUISE_CONTROL_PORT               { 2, 4 }
#define DRIVER_IO_CRUISE_CONTROL_INC                { 2, 5 }
#define DRIVER_IO_CRUISE_CONTROL_DEC                { 2, 6 }
#define DRIVER_IO_TURN_SIGNAL_RIGHT                 { 2, 7 }
#define DRIVER_IO_TURN_SIGNAL_LEFT                  { 2, 8 }
#define DRIVER_IO_HAZARD_LIGHT                      { 2, 9 }

// Analog address definitions

#define DRIVER_IO_GAS_PEDAL                         { 0, 0 }
#define DRIVER_IO_MECHANICAL_BRAKE                  { 0, 1 }

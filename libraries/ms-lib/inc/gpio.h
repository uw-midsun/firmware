#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// Defines a structure for defining what port/pin is being set
struct IOMap {
  uint32_t PORT;
  uint32_t PIN;
};

// Defines the options for the direction of the pin
typedef enum {
  PIN_IN = 0,
  PIN_OUT,
  PIN_ALTF,
  PIN_ANLG,
} IOMode;

// Defines the options for the output state of the pin
typedef enum {
  IO_PP = 0,
  IO_OD,
} IOOutput;

// Defines the options for the output speed of the IO register
typedef enum {
  IO_SLOW = 0,
  IO_MED,
  IO_FAST,
} IOSpeed;

// Defines the options for the behavior of the pull-up pull-down resistor
typedef enum {
  RES_OFF = 0,
  RES_UP,
  RES_DOWN,
} IOResistor;

// Deinit pin to analog for power saving
void io_deinit(const struct IOMap*);

// Set mode for the pin
void io_set_mode(const struct IOMap* io_map, IOMode mode);

// Set the output type and speed of the pin
void io_set_output(const struct IOMap* io_map, IOOutput output, IOSpeed speed);

// Set the pull-up pull-down resistor
void io_set_resistor(const struct IOMap* io_map, IOResistor resistor);

// TODO(calderk): consider adding locks, and input and output register read/write
// TODO(calderk): interrupt configuration and edge

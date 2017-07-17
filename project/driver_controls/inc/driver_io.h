#pragma once

// Provides an interface to configure the GPIO pins for the driver control devices.
// This allows a cleaner device initialization than having to initialize each pin with
// different settings

#include "gpio.h"
#include "interrupt.h"
#include "input_event.h"

typedef void (*DriverIOCallback)(GPIOAddress *address, void *context);

// Driver IO pin definitions
typedef enum {
  DRIVER_IO_PIN_0 = 0,
  DRIVER_IO_PIN_1,
  DRIVER_IO_PIN_2,
  DRIVER_IO_PIN_3,
  DRIVER_IO_PIN_4,
  DRIVER_IO_PIN_5,
  DRIVER_IO_PIN_6,
  DRIVER_IO_PIN_7,
  DRIVER_IO_PIN_8,
  DRIVER_IO_PIN_9,
  DRIVER_IO_PIN_10,
  DRIVER_IO_PIN_11,
  DRIVER_IO_PIN_12,
  DRIVER_IO_PIN_13,
  DRIVER_IO_PIN_14,
  DRIVER_IO_PIN_15,
  NUM_DRIVER_IO_PIN
} DriverIOPins;

// Device identifiers
typedef enum {
  DRIVER_IO_POWER_SWITCH = 0,
  DRIVER_IO_GAS_PEDAL,
  DRIVER_IO_DIRECTION_SELECTOR,
  DRIVER_IO_CRUISE_CONTROL,
  DRIVER_IO_CRUISE_CONTROL_INC,
  DRIVER_IO_CRUISE_CONTROL_DEC,
  DRIVER_IO_TURN_SIGNAL,
  DRIVER_IO_HAZARD_LIGHT,
  DRIVER_IO_MECHANICAL_BRAKE,
  NUM_DRIVER_IO_INPUTS
} DriverIODevices;

// Store the id of the device as well as the intended event to raise
typedef struct DriverIOData {
  DriverIODevices id;
  InputEvent event;
} DriverIOData;

// Initialize gpio pins and interrupts
void driver_io_init();

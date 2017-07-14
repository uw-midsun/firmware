#pragma once

// Provides an interface to configure the GPIO pins for the driver control devices.
// This allows a cleaner device initialization than having to initialize each pin with
// different settings

#include "gpio.h"
#include "interrupt.h"

typedef void (*DriverDeviceCallback)(GPIOAddress *address, void *context);

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
} DriverIO;

// TODO: order for byte alignment
typedef struct DriverDevice {
  GPIOAddress address;
  GPIODir direction;
  GPIOAltFn alt_function;
  InterruptEdge edge;
  DriverDeviceCallback callback;
  void *context;
} DriverDevice;

// Initialize gpio pins and interrupts
void driver_device_init();

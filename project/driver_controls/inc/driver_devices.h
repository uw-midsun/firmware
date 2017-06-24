#pragma once

#include "gpio.h"
#include "interrupt.h"

#define INPUT_DEVICES 10
#define OUTPUT_DEVICES 1

typedef void (*DeviceCallback)(GPIOAddress *address, void *context);

typedef struct Device {
  GPIOAddress address;
  GPIODir direction;
  InterruptEdge edge;
  GPIOAltFn alt_function;
  DeviceCallback callback;
  void *context;
} Device;

// Initialize gpio pins and interrupts
void driver_controls_init();

// Configure a pin for a specific device
void driver_controls_add_device(Device *device);
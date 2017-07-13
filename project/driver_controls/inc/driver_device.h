#pragma once

#include "gpio.h"
#include "interrupt.h"

typedef void (*DriverDeviceCallback)(GPIOAddress *address, void *context);

typedef struct DriverDevice {
  GPIOAddress address;
  GPIODir direction;
  InterruptEdge edge;
  GPIOAltFn alt_function;
  DriverDeviceCallback callback;
  void *context;
} DriverDevice;

// Initialize gpio pins and interrupts
void driver_device_init();

// Configure a pin for a specific device
void driver_device_add_device(DriverDevice *driver_device);

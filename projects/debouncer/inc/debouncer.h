#pragma once
// The debouncer module
// Requires GPIO, interrupts and soft timer to be initialized
#include "gpio.h"
#include "gpio_it.h"
#include "status.h"

typedef struct DebouncerInfo {
  GPIOAddress address;
  GPIOState state;
  gpio_it_callback callback;
  void *context;
} DebouncerInfo;

// Inits the pin and sets up the debouncer for it.
StatusCode debouncer_init_pin(DebouncerInfo *debouncer_info, const GPIOAddress *address,
                              gpio_it_callback callback, void *context);

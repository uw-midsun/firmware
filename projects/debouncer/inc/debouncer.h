#pragma once

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
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

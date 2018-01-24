#pragma once

#include "gpio.h"
#include "interrupt_def.h"
#include "status.h"
#include "gpio_it.h"

typedef struct DebounceInfo {
  GPIOAddress address;
  GPIOState state;
  gpio_it_callback callback;
  void *context;
} DebounceInfo;


StatusCode debouncer_init_pin(DebounceInfo *debounce_info, const GPIOAddress *address, gpio_it_callback callback, void *context);

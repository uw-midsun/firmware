#pragma once

#include "event_queue.h"
#include "gpio_expander.h"

typedef struct {
  GpioExpanderPin bps_indicator;
  GpioExpanderPin power_indicator;
  GpioExpanderPin lights_drl;
  GpioExpanderPin lights_low_beams;
  GpioExpanderPin lights_hazards;
} ButtonLedGpioExpanderPins;

// Initializes the LED button FSMs
void button_led_init(GpioExpanderStorage *storage, ButtonLedGpioExpanderPins *pins);

// Updates the LED buttons based on an event
bool button_led_process_event(const Event *e);

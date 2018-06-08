#include "center_console.h"
#include "input_event.h"
// Inputs are as follows:
// * A0: Power
// * A1: Drive
// * A2: Neutral
// * A3: Reverse
// * A4: Hazards
// * A5: DRLs
// * A6: Lowbeams

static const EventID s_events[NUM_CENTER_CONSOLE_INPUTS] = {
  INPUT_EVENT_CENTER_CONSOLE_POWER,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED,
  INPUT_EVENT_CENTER_CONSOLE_DRL,
  INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS,
};

static void prv_raise_event_cb(GpioExpanderPin pin, GPIOState state, void *context) {
  if (state == GPIO_STATE_HIGH) {
    event_raise(s_events[pin], 0);
  } else if ((uint8_t)pin == (uint8_t)CENTER_CONSOLE_INPUT_HAZARDS) {
    // Only hazards is non-latching
    event_raise(INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED, 0);
  }
}

StatusCode center_console_init(GpioExpanderStorage *expander) {
  const GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN };

  for (size_t i = 0; i < NUM_CENTER_CONSOLE_INPUTS; i++) {
    gpio_expander_init_pin(expander, i, &gpio_settings);
    gpio_expander_register_callback(expander, i, prv_raise_event_cb, NULL);
  }

  return STATUS_CODE_OK;
}

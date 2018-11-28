#include "center_console.h"
#include "input_event.h"
#include "log.h"
// Inputs are as follows:
// * A0: Drive
// * A1: Neutral
// * A2: Reverse
// * A3: DRLs
// * A4: Lowbeams
// * A5: Hazards
// * A6: Power

static const EventId s_events[NUM_CENTER_CONSOLE_INPUTS] = {
  INPUT_EVENT_CENTER_CONSOLE_POWER,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE,
  INPUT_EVENT_CENTER_CONSOLE_DRL,
  INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED,
  INPUT_EVENT_CENTER_CONSOLE_POWER,
};

static void prv_raise_event_cb(GpioExpanderPin pin, GpioState state, void *context) {
  CenterConsoleStorage *storage = context;

  switch (pin) {
    case CENTER_CONSOLE_INPUT_HAZARDS:
      if (state == GPIO_STATE_HIGH) {
        // Only hazards is non-latching
        event_raise(INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED, 0);
      }
      // Fall-through
    default:
      if (state == GPIO_STATE_LOW) {
        event_raise(s_events[pin], 0);
      }
  }
}

StatusCode center_console_init(CenterConsoleStorage *storage, GpioExpanderStorage *expander) {
  const GpioSettings gpio_settings = { .direction = GPIO_DIR_IN };

  for (size_t i = 0; i < NUM_CENTER_CONSOLE_INPUTS; i++) {
    status_ok_or_return(gpio_expander_init_pin(input_expander, i, &in_settings));
    status_ok_or_return(
        gpio_expander_register_callback(input_expander, i, prv_raise_event_cb, storage));
  }

  for (size_t i = 0; i < NUM_CENTER_CONSOLE_OUTPUTS; i++) {
    status_ok_or_return(gpio_expander_init_pin(output_expander, i, &out_settings));
  }

  gpio_expander_init_pin(input_expander, GPIO_EXPANDER_PIN_7, &out_settings);

  return STATUS_CODE_OK;
}

bool center_console_process_event(CenterConsoleStorage *storage, const Event *e) {
  if (storage == NULL) {
    return false;
  }

  bool processed = false;

  processed |= prv_handle_power_state(storage, e);
  processed |= prv_handle_direction(storage, e);
  processed |= prv_handle_headlights(storage, e);
  processed |= prv_handle_hazards(storage, e);

  return processed;
}

#include "center_console.h"
#include "input_event.h"
// Inputs are as follows:
// * A0: Power
// * A1: Drive
// * A2: Neutral
// * A3: Reverse
// * A4: DRLs
// * A5: Lowbeams
// * A6: Hazards

static const EventID s_events[NUM_CENTER_CONSOLE_INPUTS] = {
  INPUT_EVENT_CENTER_CONSOLE_POWER,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE,
  INPUT_EVENT_CENTER_CONSOLE_DRL,
  INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED,
};

static void prv_hold_timeout(SoftTimerID timer_id, void *context) {
  event_raise(INPUT_EVENT_CENTER_CONSOLE_POWER, 0);
}

static void prv_raise_event_cb(GpioExpanderPin pin, GPIOState state, void *context) {
  CenterConsoleStorage *storage = context;

  switch (pin) {
    case CENTER_CONSOLE_INPUT_POWER:
      if (state == GPIO_STATE_LOW) {
        soft_timer_start(CENTER_CONSOLE_POWER_HOLD_MS, prv_hold_timeout, NULL,
                         &storage->power_hold);
      } else {
        // Power button released
        soft_timer_cancel(storage->power_hold);
        storage->power_hold = SOFT_TIMER_INVALID_TIMER;
      }
      break;
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
      break;
  }
}

StatusCode center_console_init(CenterConsoleStorage *storage, GpioExpanderStorage *expander) {
  storage->power_debounce = SOFT_TIMER_INVALID_TIMER;

  const GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN };

  for (size_t i = 0; i < NUM_CENTER_CONSOLE_INPUTS; i++) {
    gpio_expander_init_pin(expander, i, &gpio_settings);
    gpio_expander_register_callback(expander, i, prv_raise_event_cb, storage);
  }

  return STATUS_CODE_OK;
}

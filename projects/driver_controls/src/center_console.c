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

static const EventID s_events[NUM_CENTER_CONSOLE_INPUTS] = {
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE,
  INPUT_EVENT_CENTER_CONSOLE_DRL,
  INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED,
  INPUT_EVENT_CENTER_CONSOLE_POWER,
};

static void prv_raise_event_cb(GpioExpanderPin pin, GPIOState state, void *context) {
  CenterConsoleStorage *storage = context;

  switch (pin) {
    case CENTER_CONSOLE_INPUT_HAZARDS:
      if (state == GPIO_STATE_HIGH) {
        // Only hazards is non-latching
        event_raise(INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED, 0);
      }
      // Fall-through
    default:
      if (state == GPIO_STATE_LOW) {
        event_raise(s_events[pin], 0);
      }
  }
}

static bool prv_handle_power_state(CenterConsoleStorage *storage, const Event *e) {
  GPIOState output_states[NUM_CENTER_CONSOLE_OUTPUTS] = { 0 };
  GPIOState charge_led = GPIO_STATE_HIGH;
  // Power update - reset all LEDs
  for (size_t i = 0; i < NUM_CENTER_CONSOLE_OUTPUTS; i++) {
    output_states[i] = GPIO_STATE_HIGH;
  }

  switch (e->id) {
    case INPUT_EVENT_POWER_STATE_OFF:
      // Power off - no LEDs should be on
      break;
    case INPUT_EVENT_POWER_STATE_FAULT:
      // Fault LED on
      output_states[CENTER_CONSOLE_OUTPUT_BPS_FAULT_LED] = GPIO_STATE_LOW;
      break;
    case INPUT_EVENT_POWER_STATE_CHARGE:
      charge_led = GPIO_STATE_LOW;
      // Fall-though - power LED should also be on
    case INPUT_EVENT_POWER_STATE_DRIVE:
      // Power LED on
      output_states[CENTER_CONSOLE_OUTPUT_POWER_LED] = GPIO_STATE_LOW;
      break;
    default:
      // Unrecognized ID
      return false;
  }

  for (size_t i = 0; i < NUM_CENTER_CONSOLE_OUTPUTS; i++) {
    gpio_expander_set_state(storage->output_expander, i, output_states[i]);
  }
  gpio_expander_set_state(storage->input_expander, GPIO_EXPANDER_PIN_7, charge_led);

  return true;
}

static bool prv_handle_direction(CenterConsoleStorage *storage, const Event *e) {
  GPIOState drive_led = GPIO_STATE_HIGH, neutral_led = GPIO_STATE_HIGH,
            reverse_led = GPIO_STATE_HIGH;

  switch (e->id) {
    case INPUT_EVENT_DIRECTION_STATE_FORWARD:
      drive_led = GPIO_STATE_LOW;
      break;
    case INPUT_EVENT_DIRECTION_STATE_NEUTRAL:
      neutral_led = GPIO_STATE_LOW;
      break;
    case INPUT_EVENT_DIRECTION_STATE_REVERSE:
      reverse_led = GPIO_STATE_LOW;
      break;
    default:
      // Unrecognized ID
      return false;
  }

  gpio_expander_set_state(storage->output_expander, CENTER_CONSOLE_OUTPUT_DRIVE_LED, drive_led);
  gpio_expander_set_state(storage->output_expander, CENTER_CONSOLE_OUTPUT_NEUTRAL_LED, neutral_led);
  gpio_expander_set_state(storage->output_expander, CENTER_CONSOLE_OUTPUT_REVERSE_LED, reverse_led);

  return true;
}

static bool prv_handle_headlights(CenterConsoleStorage *storage, const Event *e) {
  GPIOState drl_led = GPIO_STATE_HIGH, lowbeam_led = GPIO_STATE_HIGH;

  switch (e->id) {
    case INPUT_EVENT_HEADLIGHT_STATE_DRL:
      drl_led = GPIO_STATE_LOW;
      break;
    case INPUT_EVENT_HEADLIGHT_STATE_LOWBEAM:
      lowbeam_led = GPIO_STATE_LOW;
      break;
    case INPUT_EVENT_HEADLIGHT_STATE_HIGHBEAM:
    case INPUT_EVENT_HEADLIGHT_STATE_OFF:
      // Highbeam has no indicator, but turn off the DRL and lowbeam indicators
      break;
    default:
      // Unrecognized ID
      return false;
  }

  gpio_expander_set_state(storage->output_expander, CENTER_CONSOLE_OUTPUT_DRL_LED, drl_led);
  gpio_expander_set_state(storage->output_expander, CENTER_CONSOLE_OUTPUT_LOWBEAM_LED, lowbeam_led);

  return true;
}

static bool prv_handle_hazards(CenterConsoleStorage *storage, const Event *e) {
  if (e->id != INPUT_EVENT_HAZARDS_STATE_ON && e->id != INPUT_EVENT_HAZARDS_STATE_OFF) {
    return false;
  }

  GPIOState state = (e->id == INPUT_EVENT_HAZARDS_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
  gpio_expander_set_state(storage->output_expander, CENTER_CONSOLE_OUTPUT_HAZARDS_LED, state);

  return true;
}

StatusCode center_console_init(CenterConsoleStorage *storage, GpioExpanderStorage *input_expander,
                               GpioExpanderStorage *output_expander) {
  if (storage == NULL || input_expander == NULL || output_expander == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->output_expander = output_expander;
  storage->input_expander = input_expander;

  const GPIOSettings in_settings = { .direction = GPIO_DIR_IN, .resistor = GPIO_RES_PULLUP };
  const GPIOSettings out_settings = { .direction = GPIO_DIR_OUT, .state = GPIO_STATE_HIGH };

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

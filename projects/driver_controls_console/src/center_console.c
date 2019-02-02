#include "center_console.h"
#include "cc_cfg.h"
#include "input_event.h"
#include "log.h"
#include "soft_timer.h"
// Inputs are as follows:
// * A0: Lowbeams
// * A1: Hazards
// * A4: DRLs
// * A5: Drive
// * A6: Neutral
// * A7: Reverse
// * B0: Power

static const GpioAddress s_gpio_addresses[NUM_CENTER_CONSOLE_INPUTS] = {
  CC_CFG_CONSOLE_POWER_PIN,
  CC_CFG_CONSOLE_DIRECTION_DRIVE_PIN,
  CC_CFG_CONSOLE_DIRECTION_NEUTRAL_PIN,
  CC_CFG_CONSOLE_DIRECTION_REVERSE_PIN,
  CC_CFG_CONSOLE_DRL_PIN,
  CC_CFG_CONSOLE_LOWBEAMS_PIN,
  CC_CFG_CONSOLE_HAZARDS_PIN
};

static const EventId s_events[NUM_CENTER_CONSOLE_INPUTS] = {
  INPUT_EVENT_CENTER_CONSOLE_POWER,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
  INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE,
  INPUT_EVENT_CENTER_CONSOLE_DRL,
  INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS,
  INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED,
};

static void prv_poll_timeout(SoftTimerId timer_id, void *context) {
  // force trigger an interrupt
  for (int i = 0; i < NUM_CENTER_CONSOLE_INPUTS; i++) {
    gpio_it_trigger_interrupt(&s_gpio_addresses[i]);
  }

  soft_timer_start_millis(CENTER_CONSOLE_POLL_PERIOD_MS, prv_poll_timeout, NULL, NULL);
}

static void prv_event_callback(const GpioAddress *address, void *context) {
  EventId *temp = context;
  EventId event = *temp;
  GpioState state;
  gpio_get_state(address, &state);

  switch (event) {
    case INPUT_EVENT_CENTER_CONSOLE_POWER:
      if (state == GPIO_STATE_HIGH) {
        // Only hazards is non-latching
        event_raise(INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED, 0);
      }
      // Fall-through
    default:
      if (state == GPIO_STATE_LOW) {
        event_raise(event, 0);
      }
  }
}

StatusCode center_console_init(CenterConsoleStorage *storage) {
  const GpioSettings gpio_settings = { .direction = GPIO_DIR_IN };
  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  // Initialize all pins with callback on INTERRUPT_EDGE_FALLING except for
  // CC_CFG_CONSOLE_HAZARDS_PIN
  for (size_t i = 0; i < NUM_CENTER_CONSOLE_INPUTS - 1; i++) {
    gpio_init_pin(&s_gpio_addresses[i], &gpio_settings);
    gpio_it_register_interrupt(&s_gpio_addresses[i], &interrupt_settings, INTERRUPT_EDGE_FALLING,
                               prv_event_callback, &s_events[i]);
  }

  GpioAddress hazards_address = CC_CFG_CONSOLE_HAZARDS_PIN;
  gpio_init_pin(&hazards_address, &gpio_settings);
  gpio_it_register_interrupt(&hazards_address, &interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_event_callback, &s_events[NUM_CENTER_CONSOLE_INPUTS - 1]);

  // periodically trigger interrupts
  soft_timer_start_millis(CENTER_CONSOLE_POLL_PERIOD_MS, prv_poll_timeout, NULL, NULL);

  return STATUS_CODE_OK;
}

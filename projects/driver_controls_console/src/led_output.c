#include "led_output.h"
#include "cc_cfg.h"
// Outputs are as follows:
// * GP0: Nothing
// * GP1: Power
// * GP2: Reverse
// * GP3: Neutral
// * GP4: Drive
// * GP5: DRL
// * GP6: Lowbeams
// * GP7: Hazards

static GpioExpanderStorage* s_expander;
static GpioState s_power_led = GPIO_STATE_LOW;
static GpioState s_drl_led = GPIO_STATE_LOW;
static GpioState s_low_beam_led = GPIO_STATE_LOW;

StatusCode led_output_init(GpioExpanderStorage* storage) {
  s_expander = storage;

  const GpioSettings gpio_settings = { .direction = GPIO_DIR_OUT };
  for (int i = 1; i < CC_CFG_NUM_LED_PINS + 1; i++) {
    gpio_expander_init_pin(s_expander, i, &gpio_settings);
  }
  return STATUS_CODE_OK;
}

StatusCode led_output_process_event(Event* e) {
  EventId id = e->id;

  switch(id) {
    case INPUT_EVENT_CENTER_CONSOLE_POWER:
      s_power_led ^= 1;
      gpio_expander_set_state(s_expander, CC_CFG_PWR_LED, s_power_led);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_DIRECTION_DRIVE:
      gpio_expander_set_state(s_expander, CC_CFG_DRIVE_LED, GPIO_STATE_HIGH);
      gpio_expander_set_state(s_expander, CC_CFG_NEUTRAL_LED, GPIO_STATE_LOW);
      gpio_expander_set_state(s_expander, CC_CFG_REVERSE_LED, GPIO_STATE_LOW);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_DIRECTION_NEUTRAL:
      gpio_expander_set_state(s_expander, CC_CFG_DRIVE_LED, GPIO_STATE_LOW);
      gpio_expander_set_state(s_expander, CC_CFG_NEUTRAL_LED, GPIO_STATE_HIGH);
      gpio_expander_set_state(s_expander, CC_CFG_REVERSE_LED, GPIO_STATE_LOW);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_DIRECTION_REVERSE:
      gpio_expander_set_state(s_expander, CC_CFG_DRIVE_LED, GPIO_STATE_LOW);
      gpio_expander_set_state(s_expander, CC_CFG_NEUTRAL_LED, GPIO_STATE_LOW);
      gpio_expander_set_state(s_expander, CC_CFG_REVERSE_LED, GPIO_STATE_HIGH);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_DRL:
      s_drl_led ^= 1;
      gpio_expander_set_state(s_expander, CC_CFG_DRL_LED, s_drl_led);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS:
      s_low_beam_led ^= 1;
      gpio_expander_set_state(s_expander, CC_CFG_LOW_BEAM_LED, s_low_beam_led);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_HAZARDS_PRESSED:
      gpio_expander_set_state(s_expander, CC_CFG_HAZARD_LED, GPIO_STATE_HIGH);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_HAZARDS_RELEASED:
      gpio_expander_set_state(s_expander, CC_CFG_HAZARD_LED, GPIO_STATE_LOW);
      break;
    default:
      return STATUS_CODE_OUT_OF_RANGE;
  }

  return STATUS_CODE_OK;
}

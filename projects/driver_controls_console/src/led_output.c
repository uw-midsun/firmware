#include "led_output.h"
#include "cc_cfg.h"

static GpioExpanderStorage* s_exapnder;

StatusCode led_output_init(GpioExpanderStorage* storage) {
  s_expander = storage;
  return STATUS_CODE_OK;
}

StatusCode led_output_process_event(Event* e) {
  EventId id = e->id;

  switch(id) {
    case INPUT_EVENT_CENTER_CONSOLE_POWER:
      gpio_expander_set_state(s_expander, CC_CFG_PWR_LED, GPIO_STATE_HIGH);
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
      gpio_expander_set_state(s_expander, CC_CFG_DRL_LED, GPIO_STATE_HIGH);
      break;
    case INPUT_EVENT_CENTER_CONSOLE_LOWBEAMS:
      gpio_expander_set_state(s_expander, CC_CFG_LOW_BEAM_LED, GPIO_STATE_HIGH);
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

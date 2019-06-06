#include "button_led.h"

#include "can.h"
#include "can_unpack.h"

#include "button_led_fsm.h"
#include "center_console_event.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "gpio.h"

#include "log.h"

static Fsm s_button_led_fsms[NUM_CENTER_CONSOLE_BUTTON_LEDS] = { 0 };

// Callback to update the Power LED given the Power State CAN Message
static StatusCode prv_power_state_rx_handler(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  uint8_t power_state = 0;
  CAN_UNPACK_POWER_STATE(msg, &power_state);

  switch (power_state) {
    case EE_POWER_STATE_DRIVE:
      event_raise(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER);
      break;
    default:
      event_raise(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER);
      break;
  }

  return STATUS_CODE_OK;
}

// Callback to update the Lights LEDs using CAN
static StatusCode prv_lights_state_rx_handler(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  uint8_t light_id = 0;
  uint8_t light_state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &light_state);

  if (light_state >= NUM_EE_LIGHT_STATES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }
  if (light_id >= NUM_EE_LIGHT_TYPES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  EventId state_to_local_event_map[] = {
    [EE_LIGHT_STATE_ON] = CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON,
    [EE_LIGHT_STATE_OFF] = CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF,
  };

  // If it's one of the Light statuses we care about then process it
  switch (light_id) {
    case EE_LIGHT_TYPE_LOW_BEAMS:
      event_raise(state_to_local_event_map[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM);
      break;
    case EE_LIGHT_TYPE_DRL:
      event_raise(state_to_local_event_map[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL);
      break;
    case EE_LIGHT_TYPE_SIGNAL_HAZARD:
      event_raise(state_to_local_event_map[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS);
      break;
    // Currently we don't process the strobe LED in here, since we use the BPS
    // heartbeat message instead to raise the event triggering a transition in
    // the LED FSM.
    default:
      return STATUS_CODE_OK;
  }

  return STATUS_CODE_OK;
}

StatusCode button_led_init(GpioExpanderStorage *storage, ButtonLedGpioExpanderPins *pins) {
  button_led_fsm_init();

  // Create FSMs for each LED output
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_BPS], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_BPS, pins->bps_indicator, "BPSIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_PWR], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER, pins->power_indicator,
                        "PowerIndicator");
  // These are separate because Lights sends an explicit ON/OFF for these,
  // unlike Power State
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_DRL], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL, pins->lights_drl, "DRLIndicator");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_LOW_BEAMS], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM, pins->lights_low_beams,
                        "LowBeams");
  button_led_fsm_create(&s_button_led_fsms[CENTER_CONSOLE_BUTTON_LED_HAZARDS], storage,
                        EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS, pins->lights_hazards, "Hazards");

  // Enable RX handler to update power state LED
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_STATE, prv_power_state_rx_handler, NULL));

  // Enable RX handler to update car Lights LEDs
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_lights_state_rx_handler, NULL));

  return STATUS_CODE_OK;
}

bool button_led_process_event(const Event *e) {
  bool processed = false;
  for (size_t i = 0; i < NUM_CENTER_CONSOLE_BUTTON_LEDS; ++i) {
    // These are handled in the Radio FSM
    if (i == CENTER_CONSOLE_BUTTON_LED_REVERSE || i == CENTER_CONSOLE_BUTTON_LED_NEUTRAL ||
        i == CENTER_CONSOLE_BUTTON_LED_DRIVE) {
      continue;
    }
    processed |= fsm_process_event(&s_button_led_fsms[i], e);
  }
  return processed;
}

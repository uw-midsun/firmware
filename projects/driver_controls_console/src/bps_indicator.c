#include "bps_indicator.h"
#include <stddef.h>
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "cc_input_event.h"
#include "debug_led.h"
#include "exported_enums.h"

static StatusCode prv_handle_heartbeat(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  uint8_t data = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &data);

  EEBpsHeartbeatState state = data;
  if (state != EE_BPS_HEARTBEAT_STATE_OK) {
    event_raise_priority(EVENT_PRIORITY_HIGHEST, INPUT_EVENT_POWER_STATE_FAULT, 0);
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_handle_powertrain_fault(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  event_raise_priority(EVENT_PRIORITY_HIGHEST, INPUT_EVENT_POWER_STATE_FAULT, 0);

  return STATUS_CODE_OK;
}

StatusCode bps_indicator_init(void) {
  debug_led_init(DEBUG_LED_RED);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_DISTRIBUTION_FAULT, prv_handle_powertrain_fault,
                          NULL);
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_handle_heartbeat, NULL);
}

StatusCode bps_indicator_set_fault(void) {
  debug_led_set_state(DEBUG_LED_RED, true);
  return CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_STROBE, EE_LIGHT_STATE_ON);
}

StatusCode bps_indicator_clear_fault(void) {
  debug_led_set_state(DEBUG_LED_RED, false);
  return CAN_TRANSMIT_LIGHTS_STATE(EE_LIGHT_TYPE_STROBE, EE_LIGHT_STATE_OFF);
}

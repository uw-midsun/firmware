#include "bps_indicator.h"

#include <stddef.h>

#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "debug_led.h"
#include "exported_enums.h"
#include "pedal_events.h"

static StatusCode prv_handle_heartbeat(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  uint8_t data = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &data);

  EEBpsHeartbeatState state = data;
  if (state != EE_BPS_HEARTBEAT_STATE_OK) {
    uint8_t strobe_mask =
        EE_BPS_HEARTBEAT_STATE_FAULT_LTC_AFE_CELL | EE_BPS_HEARTBEAT_STATE_FAULT_LTC_AFE_TEMP |
        EE_BPS_HEARTBEAT_STATE_FAULT_LTC_AFE_FSM | EE_BPS_HEARTBEAT_STATE_FAULT_LTC_ADC;
    // The data field for this Event is used to indicate whether or not the
    // strobe light should be turned on.
    //
    // Since this is a BPS fault, we must strobe.
    bool should_strobe = !!(state & strobe_mask);
    event_raise_priority(EVENT_PRIORITY_HIGHEST, PEDAL_EVENT_INPUT_BPS_FAULT, should_strobe);
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_handle_powertrain_fault(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  // The data field for this Event is used to indicate whether or not the
  // strobe light should be turned on.
  //
  // We don't need to strobe in this case since this isn't due to conditions
  // that the BMS is monitoring for.
  event_raise_priority(EVENT_PRIORITY_HIGHEST, PEDAL_EVENT_INPUT_BPS_FAULT, 0);

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

#include "bps_indicator.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "center_console_event.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "status.h"

// CanRxHandlerCb
static StatusCode prv_strobe_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;

  uint8_t light_id = 0;
  uint8_t light_state = 0;
  CAN_UNPACK_LIGHTS_STATE(msg, &light_id, &light_state);

  // Ignore anything that isn't addressed to the Strobe Light, since we want
  // to cause transitions in the BPS Fault Dash Indicator LED FSM.
  if (light_id != EE_LIGHT_TYPE_STROBE) {
    return STATUS_CODE_OK;
  }

  const EventId can_map_to_local[NUM_EE_LIGHT_STATES] = {
    [EE_LIGHT_STATE_OFF] = CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF,
    [EE_LIGHT_STATE_ON] = CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON,
  };

  if (light_state >= NUM_EE_LIGHT_STATES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  return event_raise(can_map_to_local[light_state], EE_CENTER_CONSOLE_DIGITAL_INPUT_BPS);
}

StatusCode bps_indicator_init(void) {
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS_STATE, prv_strobe_rx, NULL));

  return STATUS_CODE_OK;
}

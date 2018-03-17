#include "bps_heartbeat.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

#define BPS_HEARTBEAT_EXPECTED_PERIOD_MS 1000

static SoftTimerID s_watchdog_id = SOFT_TIMER_INVALID_TIMER;

// SoftTimerCallback
static void prv_bps_watchdog(SoftTimerID id, void *context) {
  (void)id;
  (void)context;
  event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, 0);
}

static void prv_kick_watchdog(void) {
  if (s_watchdog_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_watchdog_id);
  }
  soft_timer_start_millis(BPS_HEARTBEAT_EXPECTED_PERIOD_MS, prv_bps_watchdog, NULL, &s_watchdog_id);
}

// CANRxHandlerCb
static StatusCode prv_bps_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  uint8_t state = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &state);
  // TODO(ELEC-105): Check state in expected enums.
  if (!state) {
    event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, 0);
  } else {
    prv_kick_watchdog();
  }
}

StatusCode bps_heartbeat_init(void) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_bps_rx, NULL);
  prv_kick_watchdog();
}

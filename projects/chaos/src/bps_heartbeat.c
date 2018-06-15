#include "bps_heartbeat.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

static SoftTimerID s_watchdog_id = SOFT_TIMER_INVALID_TIMER;

// SoftTimerCallback
static void prv_bps_watchdog(SoftTimerID id, void *context) {
  (void)id;
  (void)context;
  LOG_DEBUG("Emergency: BPS Watchdog\n");
  event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, 0);
  s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
}

static StatusCode prv_kick_watchdog(void) {
  if (s_watchdog_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_watchdog_id);
    s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
  }
  status_ok_or_return(soft_timer_start_millis(BPS_HEARTBEAT_EXPECTED_PERIOD_MS, prv_bps_watchdog,
                                              NULL, &s_watchdog_id));
  return STATUS_CODE_OK;
}

// CANRxHandlerCb
static StatusCode prv_bps_rx(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  (void)context;
  (void)ack_reply;
  uint8_t state = 0;
  CAN_UNPACK_BPS_HEARTBEAT(msg, &state);
  if (state != EE_BPS_HEARTBEAT_STATE_OK) {
    LOG_DEBUG("Emergency: BPS Fault\n");
    if (s_watchdog_id != SOFT_TIMER_INVALID_TIMER) {
      soft_timer_cancel(s_watchdog_id);
      s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
    }
    event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, 0);
  } else {
    prv_kick_watchdog();
  }
  return STATUS_CODE_OK;
}

StatusCode bps_heartbeat_init(void) {
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_bps_rx, NULL));
  return STATUS_CODE_OK;
}

StatusCode bps_heartbeat_start(void) {
  status_ok_or_return(prv_kick_watchdog());
  return STATUS_CODE_OK;
}

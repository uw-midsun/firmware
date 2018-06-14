#include "powertrain_heartbeat.h"

#include <stdbool.h>
#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

// Implementation Details
//
// There are two timers:
// - A heartbeat which loops to repeatedly signal POWERTRAIN_HEARTBEAT.
// - A watchdog which after a period of no successful acks will signal for the Emergency state.
//
// The heartbeat if successfully ACK'd by all expected devices kicks the watchdog for another
// period. Starting the heartbeat immediately sends a heartbeat before starting the periodic loop.
// Although a counter could be substituted for the watchdog timer this has the added benefit of
// signalling in the event the CAN bus starts faulting and the message cannot be sent.

static SoftTimerID s_interval_id = SOFT_TIMER_INVALID_TIMER;
static SoftTimerID s_watchdog_id = SOFT_TIMER_INVALID_TIMER;

// SoftTimerCallback
static void prv_hb_watchdog(SoftTimerID timer_id, void *context) {
  (void)timer_id;
  (void)context;
  s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
  event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, 0);
  if (s_interval_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_interval_id);
    s_interval_id = SOFT_TIMER_INVALID_TIMER;
  }
}

static void prv_kick_watchdog(void) {
  if (s_watchdog_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_watchdog_id);
    s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
  }
  soft_timer_start_millis(POWERTRAIN_HEARTBEAT_WATCHDOG_MS, prv_hb_watchdog, NULL, &s_watchdog_id);
}

// CANAckRequestCb
static StatusCode prv_ack_cb(CANMessageID id, uint16_t device, CANAckStatus status,
                             uint16_t num_remaining, void *context) {
  // The watchdog will handle the failure of subsequent ACKs so ignore the statuses.
  (void)id;
  (void)device;
  (void)context;
  (void)status;
  if (num_remaining == 0) {
    prv_kick_watchdog();
  }
  return STATUS_CODE_OK;
}

// SoftTimerCallback
static void prv_send_hb_request(SoftTimerID timer_id, void *context) {
  (void)timer_id;
  (void)context;
  CANAckRequest ack_req = {
    .callback = prv_ack_cb,
    .context = NULL,
    .expected_bitset =
        CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_PLUTUS, SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,
                                 SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER),
  };
  CAN_TRANSMIT_POWERTRAIN_HEARTBEAT(&ack_req);
  soft_timer_start_millis(POWERTRAIN_HEARTBEAT_MS, prv_send_hb_request, NULL, &s_interval_id);
}

StatusCode powertrain_heartbeat_init(void) {
  if (s_watchdog_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_watchdog_id);
    s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
  }
  if (s_interval_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(s_interval_id);
    s_interval_id = SOFT_TIMER_INVALID_TIMER;
  }

  return STATUS_CODE_OK;
}

bool powertrain_heartbeat_process_event(const Event *e) {
  if (e->id == CHAOS_EVENT_SEQUENCE_DRIVE_DONE) {
    if (s_interval_id != SOFT_TIMER_INVALID_TIMER) {
      return false;
    }
    prv_kick_watchdog();
    prv_send_hb_request(SOFT_TIMER_INVALID_TIMER, NULL);
    return true;
  } else if ((e->id > NUM_CHAOS_EVENTS_FSM && e->id < NUM_CHAOS_EVENT_SEQUENCES) ||
             e->id == CHAOS_EVENT_SEQUENCE_EMERGENCY) {
    if (s_interval_id != SOFT_TIMER_INVALID_TIMER) {
      soft_timer_cancel(s_interval_id);
      s_interval_id = SOFT_TIMER_INVALID_TIMER;
    }
    if (s_watchdog_id != SOFT_TIMER_INVALID_TIMER) {
      soft_timer_cancel(s_watchdog_id);
      s_watchdog_id = SOFT_TIMER_INVALID_TIMER;
    }
    return true;
  }
  return false;
}

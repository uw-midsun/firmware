#include "notify.h"

#include <stddef.h>

#include "can_interval.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "can_unpack.h"
#include "charger_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

static uint32_t s_watchdog_period_s;
static CanInterval *s_interval;
static SoftTimerID s_watchdog = SOFT_TIMER_INVALID_TIMER;

// Watchdog activated when commanded to start. Requires reaffirmation every minute or it will cease
// charging.
static void prv_command_watchdog(SoftTimerID id, void *context) {
  (void)id;
  (void)context;
  event_raise(CHARGER_EVENT_STOP_CHARGING, 0);
}

static void prv_kick_watchdog(void) {
  soft_timer_cancel(s_watchdog);
  soft_timer_start_seconds(s_watchdog_period_s, prv_command_watchdog, NULL, &s_watchdog);
}

// GenericCanRx
static void prv_command_rx(const GenericCanMsg *msg, void *context) {
  (void)context;
  CANMessage can_msg = { 0 };
  generic_can_msg_to_can_message(msg, &can_msg);
  EEChargerSetRelayState relay_state = 0;
  CAN_UNPACK_CHARGER_SET_RELAY_STATE(&can_msg, (uint8_t *)&relay_state);
  if (relay_state == EE_CHARGER_SET_RELAY_STATE_CLOSE) {
    event_raise(CHARGER_EVENT_START_CHARGING, 0);
    prv_kick_watchdog();
  } else {
    event_raise(CHARGER_EVENT_STOP_CHARGING, 0);
  }
}

static StatusCode prv_pack_generic_notify_msg(EEChargerConnState conn_status,
                                              GenericCanMsg *generic_msg) {
  CANMessage msg = { 0 };
  CAN_PACK_CHARGER_CONN_STATE(&msg, (uint8_t)conn_status);
  return can_message_to_generic_can_message(&msg, generic_msg);
}

StatusCode notify_init(GenericCan *can, uint32_t send_period_s, uint32_t watchdog_period_s) {
  s_watchdog_period_s = watchdog_period_s;
  soft_timer_cancel(s_watchdog);
  s_watchdog = SOFT_TIMER_INVALID_TIMER;

  GenericCanMsg msg;
  status_ok_or_return(prv_pack_generic_notify_msg(EE_CHARGER_CONN_STATE_DISCONNECTED, &msg));
  status_ok_or_return(can_interval_factory(can, &msg, send_period_s * 1000000, &s_interval));
  return generic_can_register_rx(can, prv_command_rx, GENERIC_CAN_EMPTY_MASK,
                                 SYSTEM_CAN_MESSAGE_CHARGER_SET_RELAY_STATE, false, NULL);
}

void notify_post(void) {
  // Post always sends a connected message.
  prv_pack_generic_notify_msg(EE_CHARGER_CONN_STATE_CONNECTED, &s_interval->msg);
  can_interval_enable(s_interval);
  // Doesn't start the watchdog until the first command message is received.
}

void notify_cease(void) {
  // Send a disconnect message before stopping.
  prv_pack_generic_notify_msg(EE_CHARGER_CONN_STATE_DISCONNECTED, &s_interval->msg);
  can_interval_send_now(s_interval);

  // Clear the interval and watchdog as the charger is no longer connected.
  can_interval_disable(s_interval);
  soft_timer_cancel(s_watchdog);
  s_watchdog = SOFT_TIMER_INVALID_TIMER;
}

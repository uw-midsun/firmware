#include "relay.h"

#include <stdbool.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "status.h"

static bool prv_guard(const FSM *fsm, const Event *e, void *context) {
  CANAckRequest *ack = context;
  RelayId *id = ack->context;
  return e->data == *id;
}

static StatusCode prv_ack_open_callback(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                        uint16_t num_remaining, void *context) {
  RelayId *id = context;
  if (status != CAN_ACK_STATUS_OK) {
    event_raise(CHAOS_EVENT_RETRY_RELAY, *id);
    return STATUS_CODE_OK;
  }

  event_raise(CHAOS_EVENT_RELAY_OPENED, *id);
  return STATUS_CODE_OK;
}

static StatusCode prv_ack_close_callback(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                         uint16_t num_remaining, void *context) {
  RelayId *id = context;
  if (status != CAN_ACK_STATUS_OK) {
    event_raise(CHAOS_EVENT_RETRY_RELAY, *id);
    return STATUS_CODE_OK;
  }

  event_raise(CHAOS_EVENT_RELAY_CLOSED, *id);
  return STATUS_CODE_OK;
}

#define RELAY_FSM_SETUP(name, id)                                                   \
  static FSM s_##name##_fsm;                                                        \
  static const RelayId s_##name##_id = (id);                                        \
  static volatile CANAckRequest s_##name##_req = {                                  \
    .context = (void *)&s_##name##_id,                                              \
  };                                                                                \
  FSM_DECLARE_STATE(name##_opened);                                                 \
  FSM_DECLARE_STATE(closing_##name);                                                \
  FSM_DECLARE_STATE(name##_closed);                                                 \
  FSM_DECLARE_STATE(opening_##name);                                                \
  FSM_STATE_TRANSITION(name##_opened) {                                             \
    FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_CLOSE_RELAY, prv_guard, closing_##name); \
  }                                                                                 \
  FSM_STATE_TRANSITION(closing_##name) {                                            \
    FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_CLOSED, prv_guard, name##_closed); \
    FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RETRY_RELAY, prv_guard, closing_##name); \
  }                                                                                 \
  FSM_STATE_TRANSITION(name##_closed) {                                             \
    FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_OPEN_RELAY, prv_guard, opening_##name);  \
  }                                                                                 \
  FSM_STATE_TRANSITION(opening_##name) {                                            \
    FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RELAY_OPENED, prv_guard, name##_opened); \
    FSM_ADD_GUARDED_TRANSITION(CHAOS_EVENT_RETRY_RELAY, prv_guard, opening_##name); \
  }

RELAY_FSM_SETUP(solar_front, RELAY_ID_SOLAR_MASTER_FRONT);
RELAY_FSM_SETUP(solar_rear, RELAY_ID_SOLAR_MASTER_REAR);
RELAY_FSM_SETUP(battery, RELAY_ID_BATTERY);
RELAY_FSM_SETUP(main_power, RELAY_ID_MAIN_POWER);

static void prv_relay_transmit(RelayId id, RelayState state, const CANAckRequest *ack_request) {
  switch (id) {
    case RELAY_ID_SOLAR_MASTER_FRONT:
      // TODO: Replace with solar front/back.
      CAN_TRANSMIT_SOLAR_RELAY(ack_request, state);
      return;
    case RELAY_ID_SOLAR_MASTER_REAR:
      CAN_TRANSMIT_SOLAR_RELAY(ack_request, state);
      return;
    case RELAY_ID_BATTERY:
      CAN_TRANSMIT_BATTERY_RELAY(ack_request, state);
      return;
    case RELAY_ID_MAIN_POWER:
      CAN_TRANSMIT_MAIN_RELAY(ack_request, state);
      return;
    case NUM_RELAY_IDS:
    default:
      return;
  }
}

static void prv_opening(FSM *fsm, const Event *e, void *context) {
  CANAckRequest *ack_request = context;
  ack_request->callback = prv_ack_open_callback;
  prv_relay_transmit(*(RelayId *)ack_request->context, RELAY_STATE_OPEN, ack_request);
}

static void prv_closing(FSM *fsm, const Event *e, void *context) {
  CANAckRequest *ack_request = context;
  ack_request->callback = prv_ack_close_callback;
  prv_relay_transmit(*(RelayId *)ack_request->context, RELAY_STATE_CLOSE, ack_request);
}

void relay_init(bool loopback) {
  if (loopback) {
    s_solar_front_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
    s_solar_rear_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
    s_main_power_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
    s_battery_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
  } else {
    s_solar_front_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_SOLAR_MASTER_FRONT);
    s_solar_rear_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_SOLAR_MASTER_REAR);
    s_main_power_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_MOTOR_CONTROLLER);
    s_battery_req.expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_PLUTUS);
  }
  fsm_state_init(opening_solar_front, prv_opening);
  fsm_state_init(opening_solar_rear, prv_opening);
  fsm_state_init(opening_battery, prv_opening);
  fsm_state_init(opening_main_power, prv_opening);

  fsm_state_init(closing_solar_front, prv_closing);
  fsm_state_init(closing_solar_rear, prv_closing);
  fsm_state_init(closing_battery, prv_closing);
  fsm_state_init(closing_main_power, prv_closing);

  fsm_init(&s_solar_front_fsm, "SolarFrontRelay", &solar_front_opened, &s_solar_front_req);
  fsm_init(&s_solar_rear_fsm, "SolarRearRelay", &solar_rear_opened, &s_solar_rear_req);
  fsm_init(&s_battery_fsm, "BatteryRelay", &battery_opened, &s_battery_req);
  fsm_init(&s_main_power_fsm, "MainRelay", &main_power_opened, &s_main_power_req);
}

bool relay_process_event(const Event *e) {
  bool ret = false;
  ret |= fsm_process_event(&s_solar_front_fsm, e);
  ret |= fsm_process_event(&s_solar_rear_fsm, e);
  ret |= fsm_process_event(&s_battery_fsm, e);
  ret |= fsm_process_event(&s_main_power_fsm, e);
  return ret;
}

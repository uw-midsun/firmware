#include "relay.h"

#include <stdbool.h>

#include "can_msg_defs.h"
#include "event_queue.h"
#include "relay_fsm.h"
#include "relay_id.h"

static FSM s_solar_front_relay_fsm;
static FSM s_solar_rear_relay_fsm;
static FSM s_battery_relay_fsm;
static FSM s_main_relay_fsm;

void relay_init(bool loopback) {
  relay_fsm_init();

  uint32_t solar_front_bitset;
  uint32_t solar_rear_bitset;
  uint32_t main_bitset;
  uint32_t battery_bitset;

  if (loopback) {
    solar_front_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
    solar_rear_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
    main_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
    battery_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_CHAOS);
  } else {
    solar_front_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_SOLAR_MASTER_FRONT);
    solar_rear_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_SOLAR_MASTER_REAR);
    main_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_MOTOR_CONTROLLER);
    battery_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_PLUTUS);
  }

  relay_fsm_create(&s_solar_front_relay_fsm, RELAY_ID_SOLAR_MASTER_FRONT, "SolarFrontRelay",
                   solar_front_bitset);
  relay_fsm_create(&s_solar_rear_relay_fsm, RELAY_ID_SOLAR_MASTER_REAR, "SolarRearRelay",
                   solar_rear_bitset);
  relay_fsm_create(&s_battery_relay_fsm, RELAY_ID_BATTERY, "BatteryRelay", battery_bitset);
  relay_fsm_create(&s_main_relay_fsm, RELAY_ID_MAIN_POWER, "MainRelay", main_bitset);
}

bool relay_process_event(const Event *e) {
  bool ret = false;
  ret |= fsm_process_event(&s_solar_front_relay_fsm, e);
  ret |= fsm_process_event(&s_solar_rear_relay_fsm, e);
  ret |= fsm_process_event(&s_battery_relay_fsm, e);
  ret |= fsm_process_event(&s_main_relay_fsm, e);
  return ret;
}

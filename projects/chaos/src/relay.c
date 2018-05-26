#include "relay.h"

#include <stdbool.h>

#include "can_msg_defs.h"
#include "event_queue.h"
#include "relay_fsm.h"
#include "relay_id.h"

static FSM s_relay_fsms[NUM_RELAY_IDS];

void relay_init(bool loopback) {
  relay_fsm_init();

  uint32_t solar_front_bitset;
  uint32_t solar_rear_bitset;
  uint32_t motor_bitset;
  uint32_t battery_main_bitset;
  uint32_t battery_slave_bitset;

  if (loopback) {
    battery_main_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS);
    battery_slave_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS);
    motor_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS);
    solar_front_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS);
    solar_rear_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS);
  } else {
    battery_main_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_PLUTUS);
    // TODO(ELEC-428): Add slave device? May require extended IDs...
    battery_slave_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_PLUTUS);
    motor_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER);
    solar_front_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT);
    solar_rear_bitset = CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR);
  }

  relay_fsm_create(&s_relay_fsms[RELAY_ID_BATTERY_MAIN], RELAY_ID_BATTERY_MAIN, "BatteryMainRelay",
                   battery_main_bitset);
  relay_fsm_create(&s_relay_fsms[RELAY_ID_BATTERY_SLAVE], RELAY_ID_BATTERY_SLAVE,
                   "BatterySlaveRelay", battery_slave_bitset);
  relay_fsm_create(&s_relay_fsms[RELAY_ID_SOLAR_MASTER_FRONT], RELAY_ID_SOLAR_MASTER_FRONT,
                   "SolarFrontRelay", solar_front_bitset);
  relay_fsm_create(&s_relay_fsms[RELAY_ID_SOLAR_MASTER_REAR], RELAY_ID_SOLAR_MASTER_REAR,
                   "SolarRearRelay", solar_rear_bitset);
  relay_fsm_create(&s_relay_fsms[RELAY_ID_MOTORS], RELAY_ID_MOTORS, "MotorRelay", motor_bitset);
}

bool relay_process_event(const Event *e) {
  bool ret = false;
  for (uint16_t i = 0; i < NUM_RELAY_IDS; i++) {
    ret |= fsm_process_event(&s_relay_fsms[i], e);
  }
  return ret;
}

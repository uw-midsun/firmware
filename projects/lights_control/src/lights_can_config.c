#include "lights_can_config.h"
#include "can_msg_defs.h"
#include "lights_can.h"

const LightsCanSettings s_settings_rear = {
  // clang-format off
  .event_lookup = {
    [LIGHTS_CAN_ACTION_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_RIGHT,
    [LIGHTS_CAN_ACTION_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_LEFT,
    [LIGHTS_CAN_ACTION_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_HAZARD,
    [LIGHTS_CAN_ACTION_HORN] = LIGHTS_EVENT_HORN,
    [LIGHTS_CAN_ACTION_HIGH_BEAMS] = LIGHTS_EVENT_HIGH_BEAMS,
    [LIGHTS_CAN_ACTION_LOW_BEAMS] = LIGHTS_EVENT_LOW_BEAMS,
    [LIGHTS_CAN_ACTION_DRL] = LIGHTS_EVENT_DRL,
    [LIGHTS_CAN_ACTION_BRAKES] = LIGHTS_EVENT_BRAKES,
    [LIGHTS_CAN_ACTION_STROBE] = LIGHTS_EVENT_STROBE,
    [LIGHTS_CAN_ACTION_SYNC] = LIGHTS_EVENT_SYNC,
  },
  // clang-format on
  .loopback = false,
  // TODO(ELEC-372):  figure these out
  .rx_addr = { .port = 1, .pin = 1 },
  .tx_addr = { .port = 1, .pin = 1 },
  .device_id = SYSTEM_CAN_DEVICE_LIGHTS_REAR
};

const LightsCanSettings s_settings_front = {
  // clang-format off
  .event_lookup = {
    [LIGHTS_CAN_ACTION_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_RIGHT,
    [LIGHTS_CAN_ACTION_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_LEFT,
    [LIGHTS_CAN_ACTION_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_HAZARD,
    [LIGHTS_CAN_ACTION_HORN] = LIGHTS_EVENT_HORN,
    [LIGHTS_CAN_ACTION_HIGH_BEAMS] = LIGHTS_EVENT_HIGH_BEAMS,
    [LIGHTS_CAN_ACTION_LOW_BEAMS] = LIGHTS_EVENT_LOW_BEAMS,
    [LIGHTS_CAN_ACTION_DRL] = LIGHTS_EVENT_DRL,
    [LIGHTS_CAN_ACTION_BRAKES] = LIGHTS_EVENT_BRAKES,
    [LIGHTS_CAN_ACTION_STROBE] = LIGHTS_EVENT_STROBE,
    [LIGHTS_CAN_ACTION_SYNC] = LIGHTS_EVENT_SYNC,
  },
  // clang-format on
  .loopback = false,
  // TODO(ELEC-372):  figure these out
  .rx_addr = { .port = 1, .pin = 1 },
  .tx_addr = { .port = 1, .pin = 1 },
  .device_id = SYSTEM_CAN_DEVICE_LIGHTS_FRONT
};

const LightsCanSettings *s_settings;

void lights_can_config_init(LightsCanConfigBoardType can_board) {
  s_settings =
      (can_board == LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT) ? &s_settings_front : &s_settings_rear;
}

LightsCanSettings *lights_can_config_load() {
  return s_settings;
}

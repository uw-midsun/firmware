#include "lights_can_config.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "lights_can.h"

LightsCanSettings s_settings = {
  // clang-format off
  .event_id_lookup = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [EE_LIGHT_TYPE_LOW_BEAMS] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [EE_LIGHT_TYPE_DRL] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [EE_LIGHT_TYPE_BRAKES] = { LIGHTS_EVENT_GPIO_OFF, LIGHTS_EVENT_GPIO_ON },
    [EE_LIGHT_TYPE_SIGNAL_RIGHT] = { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_ON },
    [EE_LIGHT_TYPE_SIGNAL_LEFT] = { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_ON },
    [EE_LIGHT_TYPE_SIGNAL_HAZARD] = { LIGHTS_EVENT_SIGNAL_OFF, LIGHTS_EVENT_SIGNAL_ON },
  },
  .event_data_lookup = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [EE_LIGHT_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [EE_LIGHT_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
    [EE_LIGHT_TYPE_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_MODE_RIGHT,
    [EE_LIGHT_TYPE_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_MODE_LEFT,
    [EE_LIGHT_TYPE_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_MODE_HAZARD
  },
  // clang-format on
  .loopback = false,
  // TODO(ELEC-372): Change this once we decide a network bitrate.
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .rx_addr = { .port = GPIO_PORT_A, .pin = 11 },
  .tx_addr = { .port = GPIO_PORT_A, .pin = 12 },
};

void lights_can_config_init(LightsCanConfigBoardType can_board) {
  s_settings.device_id = (can_board == LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT)
                             ? SYSTEM_CAN_DEVICE_LIGHTS_FRONT
                             : SYSTEM_CAN_DEVICE_LIGHTS_REAR;
}

LightsCanSettings *lights_can_config_load(void) {
  return &s_settings;
}

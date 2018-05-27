#include "lights_can_config.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "lights_can.h"

const LightsCanSettings s_settings_rear = {
  // clang-format off
  .peripheral_lookup = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [EE_LIGHT_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [EE_LIGHT_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
  },
  // clang-format on
  .loopback = false,
  // TODO (ELEC-372): Change this once we decide a network bitrate.
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .rx_addr = { .port = GPIO_PORT_A, .pin = 11 },
  .tx_addr = { .port = GPIO_PORT_A, .pin = 12 },
  .device_id = SYSTEM_CAN_DEVICE_LIGHTS_REAR
};

const LightsCanSettings s_settings_front = {
  // clang-format off
  .peripheral_lookup = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS,
    [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
    [EE_LIGHT_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
    [EE_LIGHT_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
  },
  // clang-format on
  .loopback = false,
  // TODO (ELEC-372): Change this once we decide a network bitrate.
  .bitrate = CAN_HW_BITRATE_125KBPS,
  .rx_addr = { .port = GPIO_PORT_A, .pin = 11 },
  .tx_addr = { .port = GPIO_PORT_A, .pin = 12 },
  .device_id = SYSTEM_CAN_DEVICE_LIGHTS_FRONT
};

const LightsCanSettings *s_settings;

void lights_can_config_init(LightsCanConfigBoardType can_board) {
  s_settings =
      (can_board == LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT) ? &s_settings_front : &s_settings_rear;
}

LightsCanSettings *lights_can_config_load(void) {
  return s_settings;
}

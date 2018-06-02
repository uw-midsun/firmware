#include "lights_can_config.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "lights_can.h"

// TODO(ELEC-372): Change rx/tx address once we decide a network bitrate.
#define LIGHTS_CAN_DECLARE_SETTINGS(name, id)                                                    \
  const LightsCanSettings s_settings_##name = {                                                  \
    .event_type =                                                                                \
        {                                                                                        \
            [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_CAN_EVENT_TYPE_GPIO,                             \
            [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_CAN_EVENT_TYPE_GPIO,                              \
            [EE_LIGHT_TYPE_DRL] = LIGHTS_CAN_EVENT_TYPE_GPIO,                                    \
            [EE_LIGHT_TYPE_BRAKES] = LIGHTS_CAN_EVENT_TYPE_GPIO,                                 \
            [EE_LIGHT_TYPE_SIGNAL_RIGHT] = LIGHTS_CAN_EVENT_TYPE_SIGNAL,                         \
            [EE_LIGHT_TYPE_SIGNAL_LEFT] = LIGHTS_CAN_EVENT_TYPE_SIGNAL,                          \
            [EE_LIGHT_TYPE_SIGNAL_HAZARD] = LIGHTS_CAN_EVENT_TYPE_SIGNAL,                        \
            [EE_LIGHT_TYPE_STROBE] = LIGHTS_CAN_EVENT_TYPE_STROBE,                               \
        },                                                                                       \
    .event_data_lookup = { [EE_LIGHT_TYPE_HIGH_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS, \
                           [EE_LIGHT_TYPE_LOW_BEAMS] = LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,   \
                           [EE_LIGHT_TYPE_DRL] = LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,               \
                           [EE_LIGHT_TYPE_BRAKES] = LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,         \
                           [EE_LIGHT_TYPE_SIGNAL_RIGHT] = LIGHTS_EVENT_SIGNAL_MODE_RIGHT,        \
                           [EE_LIGHT_TYPE_SIGNAL_LEFT] = LIGHTS_EVENT_SIGNAL_MODE_LEFT,          \
                           [EE_LIGHT_TYPE_SIGNAL_HAZARD] = LIGHTS_EVENT_SIGNAL_MODE_HAZARD },    \
  }

LIGHTS_CAN_DECLARE_SETTINGS(front, SYSTEM_CAN_DEVICE_LIGHTS_FRONT);
LIGHTS_CAN_DECLARE_SETTINGS(rear, SYSTEM_CAN_DEVICE_LIGHTS_REAR);

const LightsCanSettings *s_settings;

void lights_can_config_init(LightsCanConfigBoardType can_board) {
  s_settings =
      (can_board == LIGHTS_CAN_CONFIG_BOARD_TYPE_FRONT) ? &s_settings_front : &s_settings_rear;
}

const LightsCanSettings *lights_can_config_load(void) {
  return s_settings;
}

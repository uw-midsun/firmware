#include "lights_can_config.h"
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "lights_can.h"

const LightsCanSettings s_settings = {
  // clang-format off
  .event_type = {
    [EE_LIGHT_TYPE_HIGH_BEAMS] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_LOW_BEAMS] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_DRL] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_BRAKES] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_GPIO,
    [EE_LIGHT_TYPE_SIGNAL_RIGHT] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_SIGNAL,
    [EE_LIGHT_TYPE_SIGNAL_LEFT] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_SIGNAL,
    [EE_LIGHT_TYPE_SIGNAL_HAZARD] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_SIGNAL,
    [EE_LIGHT_TYPE_STROBE] = (LightsEvent) LIGHTS_CAN_EVENT_TYPE_STROBE,
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
};

const LightsCanSettings *lights_can_config_load(void) {
  return &s_settings;
}

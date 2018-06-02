#include "lights_config.h"
#include "lights_gpio.h"

#define LIGHTS_CONFIG_SIGNAL_BLINKER_DURATION 500;
#define LIGHTS_CONFIG_SYNC_COUNT 5;

static CANSettings s_can_settings = { 
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = LIGHTS_EVENT_CAN_RX,
  .tx = {
    .port = GPIO_PORT_A,
    .pin = 12,
  },
  .rx = {
    .port = GPIO_PORT_A,
    .pin = 11,
  },
  .tx_event = LIGHTS_EVENT_CAN_TX,
  .fault_event = LIGHTS_EVENT_CAN_FAULT,
  .loopback = false,
};

static LightsConfig s_lights_config = { 0 };

static LightsSignalFsm s_signal_fsm = { 0 };

static const GPIOAddress s_board_type_address = {
  .port = GPIO_PORT_B,
  .pin = 13
};

StatusCode lights_config_init(void) {
  // Getting board type.
  GPIOState state = 0;
  status_ok_or_return(gpio_get_state(&s_board_type_address, &state));
  LightsConfigBoardType board_type = (state == GPIO_STATE_HIGH) ?
          LIGHTS_CONFIG_BOARD_TYPE_FRONT : LIGHTS_CONFIG_BOARD_TYPE_REAR;

  // Setting GPIO configuration.
  status_ok_or_return(lights_gpio_config_init(s_board_type));
  s_lights_config.LightsGpio = lights_gpio_config_load();

  // Setting lights_can configuration.
  s_lights_config.lights_can_settings = lights_can_config_load();

  // Setting can configuration.
  s_can_settings.device_id = (board_type == LIGHTS_CONFIG_BOARD_TYPE_FRONT) ?
          SYSTEM_CAN_DEVICE_SOLAR_MASTER_FRONT : SYSTEM_CAN_DEVICE_SOLAR_MASTER_REAR;
  s_lights_config.can_settings = &s_can_settings;

  // Setting signal FSM configuration.
  s_lights_config.signal_fsm = &s_signal_fsm;
  s_lights_config.signal_blinker_duration = LIGHTS_CONFIG_SIGNAL_BLINKER_DURATION;
  s_lights_config.sync_count = LIGHTS_CONFIG_SYNC_COUNT;

}

LightsConfig *lights_config_load(void) {
  return &s_lights_config;
}


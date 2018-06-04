#include <stdint.h>

#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "status.h"
#include "wait.h"

#include "lights_blinker.h"
#include "lights_board_type.h"
#include "lights_can.h"
#include "lights_can_config.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"
#include "lights_gpio_config_front.h"
#include "lights_signal_fsm.h"
#include "lights_strobe.h"

#define LIGHTS_SIGNAL_BLINKER_DURATION 500
#define LIGHTS_SIGNAL_SYNC_COUNT 5
#define LIGHTS_STROBE_BLINKER_DURATION 500

static CANSettings s_can_settings = {
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = LIGHTS_EVENT_CAN_RX,
  // clang-format off
  .tx = {
    .port = GPIO_PORT_A,
    .pin = 12,
  },
  .rx = {
    .port = GPIO_PORT_A,
    .pin = 11,
  },
  // clang-format on
  .tx_event = LIGHTS_EVENT_CAN_TX,
  .fault_event = LIGHTS_EVENT_CAN_FAULT,
  .loopback = false,
};

static LightsSignalFsm s_signal_fsm = { 0 };

static LightsCanStorage s_lights_can_storage = { 0 };

static LightsStrobeStorage s_lights_strobe = { 0 };

static const GPIOAddress s_board_type_address = { .port = GPIO_PORT_B, .pin = 13 };

int main(void) {
  // Initialize the libraries.
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  // Getting board type.
  GPIOState state = 0;
  status_ok_or_return(gpio_get_state(&s_board_type_address, &state));
  LightsBoardType board_type =
      (state == GPIO_STATE_HIGH) ? LIGHTS_BOARD_TYPE_FRONT : LIGHTS_BOARD_TYPE_REAR;
  LOG_DEBUG("Board Type: %s\n", (board_type == LIGHTS_BOARD_TYPE_FRONT) ? "Front" : "Back");
  // Initialize lights_gpio.
  status_ok_or_return(lights_gpio_config_init(board_type));
  LightsGpio *lights_gpio = lights_gpio_config_load();
  lights_gpio_init(lights_gpio);

  // Initialize lights_can.
  s_can_settings.device_id = (board_type == LIGHTS_BOARD_TYPE_FRONT)
                                 ? SYSTEM_CAN_DEVICE_LIGHTS_FRONT
                                 : SYSTEM_CAN_DEVICE_LIGHTS_REAR;
  lights_can_init(&s_lights_can_storage, lights_can_config_load(), &s_can_settings);

  // Initialize lights_signal_fsm.
  lights_signal_fsm_init(&s_signal_fsm, LIGHTS_SIGNAL_BLINKER_DURATION,
                         (board_type == LIGHTS_BOARD_TYPE_FRONT) ? 0 : LIGHTS_SIGNAL_SYNC_COUNT);

  // Initialize lights_strobe.
  lights_strobe_init(&s_lights_strobe, LIGHTS_STROBE_BLINKER_DURATION);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      fsm_process_event(CAN_FSM, &e);
      lights_can_process_event(&e);
      lights_gpio_process_event(lights_gpio, &e);
      lights_signal_fsm_process_event(&s_signal_fsm, &e);
      lights_strobe_process_event(&s_lights_strobe, &e);
    }
    wait();
  }
  return STATUS_CODE_OK;
}

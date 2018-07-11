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

#define LIGHTS_CONTROL_SIGNAL_BLINKER_DURATION 500
#define LIGHTS_CONTROL_SIGNAL_SYNC_COUNT 5
#define LIGHTS_CONTROL_STROBE_BLINKER_DURATION 500

static LightsSignalFsm s_signal_fsm = { 0 };

static LightsCanStorage s_lights_can_storage = { 0 };

static LightsStrobeStorage s_lights_strobe = { 0 };

static const GpioAddress s_board_type_address = { .port = GPIO_PORT_B, .pin = 13 };

int main(void) {
  CanSettings can_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = { .port = GPIO_PORT_A, .pin = 12 },
    .rx = { .port = GPIO_PORT_A, .pin = 11 },
    .rx_event = SOLAR_SENSE_EVENT_CAN_RX,
    .tx_event = SOLAR_SENSE_EVENT_CAN_TX,
    .fault_event = SOLAR_SENSE_EVENT_CAN_FAULT,
    .loopback = false,
  };

  uint16_t device_id_lookup[NUM_LIGHTS_BOARD_TYPES] = {
    [LIGHTS_BOARD_TYPE_FRONT] = SYSTEM_CAN_DEVICE_LIGHTS_FRONT,
    [LIGHTS_BOARD_TYPE_REAR] = SYSTEM_CAN_DEVICE_LIGHTS_REAR,
  };

  uint8_t sync_count_lookup[NUM_LIGHTS_BOARD_TYPES] = {
    [LIGHTS_BOARD_TYPE_FRONT] = LIGHTS_SIGNAL_FSM_NO_SYNC,
    [LIGHTS_BOARD_TYPE_REAR] = LIGHTS_CONTROL_SIGNAL_SYNC_COUNT,
  };

  // Initialize the libraries.
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  // Getting board type.
  GpioState state = NUM_GPIO_STATES;

  StatusCode status = gpio_get_state(&s_board_type_address, &state);

  if (!status_ok(status)) {
    LOG_DEBUG("Error getting board type.\n");
  }

  LightsBoardType board_type =
      (state == GPIO_STATE_HIGH) ? LIGHTS_BOARD_TYPE_FRONT : LIGHTS_BOARD_TYPE_REAR;
  LOG_DEBUG("Board Type: %s\n", (board_type == LIGHTS_BOARD_TYPE_FRONT) ? "Front" : "Back");
  // Initialize lights_gpio.

  lights_gpio_config_init(board_type);

  LightsGpio *lights_gpio = lights_gpio_config_load();
  status = lights_gpio_init(lights_gpio);

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing lights GPIO.\n");
  }

  // Initialize lights_can.
  can_settings.device_id = device_id_lookup[board_type];

  status = lights_can_init(&s_lights_can_storage, lights_can_config_load(), &can_settings);

  if (!status_ok(status)) {
    LOG_DEBUG("Error initializing lights CAN.\n");
  }

  // Initialize lights_signal_fsm.
  lights_signal_fsm_init(&s_signal_fsm, LIGHTS_CONTROL_SIGNAL_BLINKER_DURATION,
                         sync_count_lookup[board_type]);

  // Initialize lights_strobe.
  lights_strobe_init(&s_lights_strobe, LIGHTS_CONTROL_STROBE_BLINKER_DURATION);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
      if (board_type == LIGHTS_BOARD_TYPE_REAR) {
        lights_can_process_event(&e);
      }
      lights_gpio_process_event(lights_gpio, &e);
      lights_signal_fsm_process_event(&s_signal_fsm, &e);
      lights_strobe_process_event(&s_lights_strobe, &e);
    }
    wait();
  }
  return STATUS_CODE_OK;
}

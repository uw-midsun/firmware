#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "can.h"
#include "can_msg_defs.h"
#include "crc32.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"

#include "calib.h"
#include "control_stalk.h"

#include "debug_led.h"
#include "event_queue.h"
#include "flash.h"
#include "heartbeat_rx.h"
#include "sc_cfg.h"
#include "sc_input_event.h"
#include "soft_timer.h"
#include "steering_output.h"


static CanStorage s_can;

static ControlStalk s_stalk;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  crc32_init();
  flash_init();

  const CanSettings can_settings = {
    .device_id = SC_CFG_CAN_DEVICE_ID,
    .bitrate = SC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_STEERING_CAN_RX,
    .tx_event = INPUT_EVENT_STEERING_CAN_TX,
    .fault_event = INPUT_EVENT_STEERING_CAN_FAULT,
    .tx = SC_CFG_CAN_TX,
    .rx = SC_CFG_CAN_RX,
    .loopback = false,
  };

  can_init(&s_can, &can_settings);

  // Powertrain heartbeat
  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  control_stalk_init(&s_stalk);
  steering_output_init(steering_output_global(), INPUT_EVENT_STEERING_WATCHDOG_FAULT,
                       INPUT_EVENT_STEERING_UPDATE_REQUESTED);
  steering_output_set_enabled(steering_output_global(), true);

  LOG_DEBUG("Steering Controls initialized\n");

  Event e;
  while (true) {
    if (status_ok(event_process(&e))) {
#ifdef SC_CFG_DEBUG_PRINT_EVENTS
      switch (e.id) {
        case INPUT_EVENT_STEERING_UPDATE_REQUESTED:
        case INPUT_EVENT_STEERING_CAN_RX:
        case INPUT_EVENT_STEERING_CAN_TX:
          break;
        default:
          LOG_DEBUG("e %d %d\n", e.id, e.data);
      }
#endif
      can_process_event(&e);
    }
  }
}

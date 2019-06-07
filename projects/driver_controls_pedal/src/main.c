#include <stdbool.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "event_queue.h"
#include "pedal_events.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"

#include "exported_enums.h"

static CanStorage s_can_storage = { 0 };

int main() {
  // Standard module inits
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // CAN initialization
  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_EVENT_CAN_RX,
    .tx_event = PEDAL_EVENT_CAN_TX,
    .fault_event = PEDAL_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}

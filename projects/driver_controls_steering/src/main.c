#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"

#include "config.h"
#include "control_stalk.h"
#include "input_event.h"

#include "log.h"
static CanStorage s_can_storage;

// We only need the following inputs from the Steering Board:
// * CC_SPEED: Used to increase/decrease the current cruise target
// * CC_SET: Used to set the cruise target to the current speed
// * CC_ON/OFF: Used to enable/disable cruise control
// * TURN_SIGNAL_STALK: Used to designate left/right turn signals

int main() {
  // Standard module inits
  status_ok_or_return(gpio_init());
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // CAN initialization
  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = STEERING_EVENT_CAN_RX,
    .tx_event = STEERING_EVENT_CAN_TX,
    .fault_event = STEERING_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  adc_init(ADC_MODE_CONTINUOUS);

  // Enable Control Stalk
  control_stalk_init();

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}

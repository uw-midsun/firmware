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
#include "exported_enums.h"
#include "steering_events.h"

#include "log.h"
static CanStorage s_can_storage;

// We only need the following inputs from the Steering Board:
// * CC_SPEED: Used to increase/decrease the current cruise target
// * CC_SET: Used to set the cruise target to the current speed
// * CC_ON/OFF: Used to enable/disable cruise control
// * TURN_SIGNAL_STALK: Used to designate left/right turn signals

int main() {
  // Standard module inits
  gpio_init();
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
  can_init(&s_can_storage, &can_settings);

  adc_init(ADC_MODE_SINGLE);

  // Enable Control Stalk
  ControlStalkStorage stalk = {
    .digital_config =
        {
            //
            [CONTROL_STALK_DIGITAL_INPUT_ID_HORN] =
                {
                    .pin = STEERING_CONFIG_PIN_HORN,
                    .can_event =
                        {
                            [GPIO_STATE_HIGH] = EE_STEERING_INPUT_HORN_PRESSED,  //
                            [GPIO_STATE_LOW] = EE_STEERING_INPUT_HORN_RELEASED   //
                        },
                },
            [CONTROL_STALK_DIGITAL_INPUT_ID_CC_ON_OFF] =
                {
                    .pin = STEERING_CONFIG_PIN_CC_ON_OFF,
                    .can_event =
                        {
                            [GPIO_STATE_HIGH] = EE_STEERING_INPUT_CC_ON_OFF_PRESSED,  //
                            [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_ON_OFF_RELEASED   //
                        },
                },
            [CONTROL_STALK_DIGITAL_INPUT_ID_SET] =
                {
                    .pin = STEERING_CONFIG_PIN_CC_SET,
                    .can_event =
                        {
                            [GPIO_STATE_HIGH] = EE_STEERING_INPUT_CC_SET_PRESSED,  //
                            [GPIO_STATE_LOW] = EE_STEERING_INPUT_CC_SET_RELEASED   //
                        },
                }  //
        },
    .analog_config =
        {
            //
            [CONTROL_STALK_ANALOG_INPUT_ID_CC_SPEED] =
                {
                    .address = STEERING_CONFIG_PIN_CC_SPEED,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] = EE_STEERING_INPUT_CC_SPEED_NEUTRAL,  //
                            [CONTROL_STALK_STATE_681_OHMS] = EE_STEERING_INPUT_CC_SPEED_MINUS,    //
                            [CONTROL_STALK_STATE_2181_OHMS] = EE_STEERING_INPUT_CC_SPEED_PLUS     //
                        },
                },
            [CONTROL_STALK_ANALOG_INPUT_ID_CC_CANCEL_RESUME] =
                {
                    .address = STEERING_CONFIG_PIN_CC_CANCEL_RESUME,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] =
                                EE_STEERING_INPUT_CC_CANCEL_RESUME_NEUTRAL,  //
                            [CONTROL_STALK_STATE_681_OHMS] =
                                EE_STEERING_INPUT_CC_CANCEL_RESUME_CANCEL,  //
                            [CONTROL_STALK_STATE_2181_OHMS] =
                                EE_STEERING_INPUT_CC_CANCEL_RESUME_RESUME  //
                        },
                },
            [CONTROL_STALK_ANALOG_INPUT_ID_TURN_SIGNAL_STALK] =
                {
                    .address = STEERING_CONFIG_PIN_TURN_SIGNAL_STALK,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] =
                                EE_STEERING_INPUT_TURN_SIGNAL_STALK_NONE,  //
                            [CONTROL_STALK_STATE_681_OHMS] =
                                EE_STEERING_INPUT_TURN_SIGNAL_STALK_RIGHT,  //
                            [CONTROL_STALK_STATE_2181_OHMS] =
                                EE_STEERING_INPUT_TURN_SIGNAL_STALK_LEFT  //
                        },
                },
            [CONTROL_STALK_ANALOG_INPUT_ID_CC_DISTANCE] =
                {
                    .address = STEERING_CONFIG_PIN_CC_DISTANCE,
                    .can_event =
                        {
                            [CONTROL_STALK_STATE_FLOATING] =
                                EE_STEERING_INPUT_EVENT_CC_DISTANCE_NEUTRAL,  //
                            [CONTROL_STALK_STATE_681_OHMS] =
                                EE_STEERING_INPUT_EVENT_CC_DISTANCE_MINUS,  //
                            [CONTROL_STALK_STATE_2181_OHMS] =
                                EE_STEERING_INPUT_EVENT_CC_DISTANCE_PLUS  //
                        },
                }  //
        },
  };
  control_stalk_init(&stalk);

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}

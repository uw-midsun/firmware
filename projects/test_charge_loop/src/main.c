#include "charger.h"
#include "chaos_events.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#include "event_queue.h"
#include "gpio.h"
#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "chaos_events.h"
#include "dc_cfg.h"
#include "delay.h"
#include "gpio.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"
#include "unity_internals.h"

static CanStorage s_can;

int main(void) {

  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .loopback = false,
  };
  Event e = {};

  gpio_init();
  gpio_it_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  can_init(&s_can, &can_settings);
  //charger_init();

  GpioAddress pwm_address = {
    .port = GPIO_PORT_A,  //
    .pin = 5     //
  };

  GpioSettings pwm_gpio = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_PULLUP,
    .alt_function = GPIO_ALTFN_2
  };

  pwm_init(PWM_TIMER_3, 1000);

  gpio_init_pin(pwm_address, pwm_gpio);

  while (true) {
    LOG_DEBUG("SUCCESS: %d\n", CAN_TRANSMIT_CHARGER_SET_RELAY_STATE(EE_CHARGER_SET_RELAY_STATE_CLOSE));

    MS_TEST_HELPER_AWAIT_EVENT(e);
    event_process(&e);
    can_process_event(&e);
    LOG_DEBUG("Sent_event_id %d\n", e.id);
    delay_ms(250);

    LOG_DEBUG("SUCCESS: %d\n", CAN_TRANSMIT_CHARGER_SET_RELAY_STATE(EE_CHARGER_SET_RELAY_STATE_OPEN));

    MS_TEST_HELPER_AWAIT_EVENT(e);
    event_process(&e);
    can_process_event(&e);
    LOG_DEBUG("Sent_event_id %d\n", e.id);
    delay_ms(250);

  }
}

/*
  CAN_TRANSMIT_POWER_STATE((uint8_t)EE_POWER_STATE_DRIVE);
  CAN_TRANSMIT_POWER_STATE((uint8_t)EE_POWER_STATE_CHARGE);
  CAN_TRANSMIT_POWER_STATE((uint8_t)EE_POWER_STATE_IDLE);
---------
  CAN_TRANSMIT_CHARGER_SET_RELAY_STATE((uint8_t)EE_CHARGER_SET_RELAY_STATE_OPEN);
  CAN_TRANSMIT_CHARGER_SET_RELAY_STATE((uint8_t)EE_CHARGER_SET_RELAY_STATE_CLOSE);
*/

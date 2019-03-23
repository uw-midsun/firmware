// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"       // For real-time delays
#include "gpio.h"        // General Purpose I/O control.
#include "gpio_it.h"
#include "interrupt.h"   // For enabling interrupts.
#include "misc.h"        // Various helper functions/macros.
#include "soft_timer.h"  // Software timers for scheduling future events.


#include "can.h"
#include "can_interval.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "event_queue.h"

#include "log.h" 

typedef enum {
  TEST_EVENT_CAN_RX = 1, 
  TEST_EVENT_CAN_TX = 2,
  TEST_EVENT_CAN_FAULT = 3, 
} TestEvent;

static CanStorage s_can_storage = { 0 };

uint16_t LED_GREEN = 0x1; 

static void periodic_blink_led(SoftTimerId timer_id, void *context) {
  LED_GREEN = LED_GREEN ^ 0x1 << 0; 
  CAN_TRANSMIT_LED(LED_GREEN); 
  soft_timer_start_millis(500, periodic_blink_led, NULL, NULL);
}

int main(void) {
  gpio_init();
  // Enable various peripherals
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  
  Event e = { 0 };

 const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_TEST_MASTER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_EVENT_CAN_RX,
    .tx_event = TEST_EVENT_CAN_TX,
    .fault_event = TEST_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  StatusCode status = can_init(&s_can_storage, &can_settings);

  soft_timer_start_millis(500, periodic_blink_led, NULL, NULL); 

  while (true) {
  
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}

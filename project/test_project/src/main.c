#include <stdio.h>
#include <stdint.h>

#include "fsm.h"
#include "interrupt.h"
#include "gpio.h"
#include "gpio_it.h"
#include "event_queue.h"
#include "input_interrupt.h"
#include "driver_state.h"
#include "stm32f0xx.h"

#define INPUT_DEVICES 4
#define OUTPUT_DEVICES 4
#define CAN_PINS 2

int main() {
  FSM fsm;
  fsm_init(&fsm, "driver_fsm", &state_off, &fsm);

  gpio_init();
  interrupt_init();
  gpio_it_init();

  // Input pins
  GPIOAddress input[INPUT_DEVICES] = {
    { GPIO_PORT_C, 4 },
	{ GPIO_PORT_C, 5 },
    { GPIO_PORT_C, 6 },
	{ GPIO_PORT_A, 0 }
  };

  // Test output pins
  GPIOAddress led[OUTPUT_DEVICES] = {
    { GPIO_PORT_C, 6 },
    { GPIO_PORT_C, 7 },
    { GPIO_PORT_C, 8 },
    { GPIO_PORT_C, 9 }
  };

  // CAN Tx and Rx pins
  GPIOAddress can_address[CAN_PINS] = {
    { GPIO_PORT_B, 8 },
    { GPIO_PORT_B, 9 }
  };

  GPIOSettings gpio_settings = { GPIO_DIR_OUT, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  for (int i=0; i < OUTPUT_DEVICES; i++) {
    gpio_init_pin(&led[i], &gpio_settings);
  }

  gpio_settings = (GPIOSettings){ GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (int i=0; i < INPUT_DEVICES; i++) {
    gpio_init_pin(&input[i], &gpio_settings);
    gpio_it_register_interrupt(&input[i], &it_settings, INTERRUPT_EDGE_RISING_FALLING, input_callback, &led[0]);
  }

  for (;;) {}
}

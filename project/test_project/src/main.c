#include <stdio.h>
#include <stdint.h>

#include "interrupt.h"
#include "gpio.h"
#include "gpio_it.h"
#include "input_interrupt.h"
#include "driver_state.h"
#include "soft_timer.h"

typedef struct Input {
	GPIOAddress address;
	InterruptEdge edge;
	GPIOAltFn alt_function;	
} Input;

int main() {
  FSMGroup fsm_group;

  fsm_init(&fsm_group.pedal_fsm, "pedal_fsm", &state_off, &fsm_group.pedal_fsm);
  fsm_init(&fsm_group.direction_fsm, "direction_fsm", &state_neutral, &fsm_group.direction_fsm);
  fsm_init(&fsm_group.turn_signal_fsm, "turn_signal_fsm", &state_no_signal, &fsm_group.turn_signal_fsm);

  gpio_init();
  interrupt_init();
  gpio_it_init();
  adc_init(ADC_MODE_CONTINUOUS);

  // List of inputs
  Input input[INPUT_DEVICES] = {
    { { GPIO_PORT_A, 0 }, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_A, 1 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_ANALOG },
		{ { GPIO_PORT_B, 2 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_B, 3 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_B, 4 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_B, 5 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_C, 6 }, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_C, 7 }, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_C, 8 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
		{ { GPIO_PORT_C, 9 }, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE }	
  };

  // Test output pins
  GPIOAddress led[OUTPUT_DEVICES] = {
    { GPIO_PORT_C, 6 },
    { GPIO_PORT_C, 7 },
    { GPIO_PORT_C, 8 },
    { GPIO_PORT_C, 9 }
  };

  // CAN Tx and Rx pins
  GPIOAddress can_address[2] = {
    { GPIO_PORT_B, 8 },
    { GPIO_PORT_B, 9 }
  };

  GPIOSettings gpio_settings = { GPIO_DIR_OUT, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  for (uint8_t i=0; i < OUTPUT_DEVICES; i++) {
    gpio_init_pin(&led[i], &gpio_settings);
  }

  gpio_settings = (GPIOSettings){ GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i=0; i < INPUT_DEVICES; i++) {
    gpio_settings.alt_function = input[i].alt_function;
		gpio_init_pin(&input[i].address, &gpio_settings);
    gpio_it_register_interrupt(&input[i].address, &it_settings, input[i].edge, input_callback, &fsm_group);
  }

  for (;;) {
		//printf(BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(GPIOC->IDR & (GPIO_IDR_7 | GPIO_IDR_8)));
		input_callback(&input[1].address, &fsm_group);
		for (uint32_t i = 0; i < 2000000; i++) {}
  }
}

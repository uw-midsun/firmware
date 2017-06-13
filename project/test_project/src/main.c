#include <stdio.h>
#include <stdint.h>

#include "interrupt.h"
#include "gpio.h"
#include "gpio_it.h"
#include "input_interrupt.h"
#include "event_queue.h"
#include "driver_state.h"
#include "soft_timer.h"

/*	TODO:
					- See if the setup code can be moved to another function or file
*/

typedef struct Device {
  GPIOAddress address;
  GPIODir direction;
  InterruptEdge edge;
  GPIOAltFn alt_function;
} Device;

int main() {
	
	// Initialize the state machines to be used, along with their default settings
  FSMGroup fsm_group;
	FSMState default_state[] = { STATE_OFF, STATE_NEUTRAL, STATE_NO_SIGNAL, STATE_HAZARD_OFF };

	state_init(&fsm_group, default_state);

  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };
	ADCSettings adc_settings = { ADC_MODE_CONTINUOUS, ADC_RESOLUTION_12 };

	// Initialize other devices to be used
  gpio_init();
  interrupt_init();
  gpio_it_init();
	event_queue_init();
  adc_init(&adc_settings);

  Device input[INPUT_DEVICES] = {
    { { GPIO_PORT_C, 0 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 1 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_ANALOG },
    { { GPIO_PORT_B, 2 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_B, 3 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 4 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 5 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 6 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 7 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 8 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 9 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
  	{ { GPIO_PORT_C, 10 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE }
	};
	
	Device output[OUTPUT_DEVICES] = {
		{ { GPIO_PORT_C, 11 }, GPIO_DIR_OUT, 0, GPIO_ALTFN_NONE } 
	};

  for (uint8_t i=0; i < INPUT_DEVICES; i++) {
    gpio_settings.direction = input[i].direction;
    gpio_settings.alt_function = input[i].alt_function;
    gpio_init_pin(&input[i].address, &gpio_settings);
    gpio_it_register_interrupt(&input[i].address,
            &it_settings,
            input[i].edge,
            input_callback,
            &fsm_group);
  }

	adc_init_pin(&input[1].address, ADC_SAMPLE_RATE_1);	

	for (uint8_t i=0; i < OUTPUT_DEVICES; i++) {
		gpio_settings.direction = output[i].direction;
		gpio_settings.alt_function = output[i].alt_function;
  	gpio_init_pin(&output[i].address, &gpio_settings);
	}

	// Main program loop

	Event e;	

  for (;;) {
    input_callback(&input[1].address, &fsm_group);
		for (uint8_t i = 0; i < 10; i++) {
			if (!event_process(&e)) {
				state_process_event(&fsm_group, &e);
				  printf("Event = %d : Car Status = %s : Direction = %s : Turn = %s : Hazard = %s : %d \n",
     				e.id,
     				fsm_group.pedal_fsm.current_state->name,
     				fsm_group.direction_fsm.current_state->name,
     				fsm_group.turn_signal_fsm.current_state->name,
     				fsm_group.hazard_light_fsm.current_state->name,
     				ADC1->DR);
			}
		}
  }

}

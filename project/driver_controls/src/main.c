#include <stdio.h>
#include <stdint.h>

#include "input_interrupt.h"
#include "event_queue.h"
#include "driver_state.h"
#include "soft_timer.h"
#include "driver_devices.h"

#define INPUT_DEVICES 10
#define OUTPUT_DEVICES 1

void device_init() {
  driver_controls_init();

  Device inputs[INPUT_DEVICES] = {
    { { GPIO_PORT_C, 0 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 1 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_ANALOG, input_callback },
    { { GPIO_PORT_B, 2 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_B, 3 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 4 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 5 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 6 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 7 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 8 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 9 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, input_callback },
    { { GPIO_PORT_C, 10 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, input_callback }
  };

  Device outputs[OUTPUT_DEVICES] = {
    { { GPIO_PORT_C, 11 }, GPIO_DIR_OUT, 0, GPIO_ALTFN_NONE } 
  };

  for (uint8_t i = 0; i < INPUT_DEVICES; i++) {
    driver_controls_add_device(&inputs[i]);
  }

  for (uint8_t i = 0; i < OUTPUT_DEVICES; i++) {
    driver_controls_add_device(&outputs[i]);
  }
}

int main() {
	// Initialize the state machines to be used, along with their default settings
  FSMGroup fsm_group;
	state_init(&fsm_group);

  device_init();

	// Initialize other devices to be used
	event_queue_init();
  soft_timer_init();

	Event e;	

  for (;;) {
		if (!event_process(&e)) {
			state_process_event(&fsm_group, &e);
		  printf("Event = %d : %s : %s : %s : %s : %s : %d \n",
    			e.id,
          fsm_group.power.current_state->name,
    			fsm_group.pedal.current_state->name,
    			fsm_group.direction.current_state->name,
    			fsm_group.turn_signal.current_state->name,
    			fsm_group.hazard_light.current_state->name,
    			0);
		}
  }

}

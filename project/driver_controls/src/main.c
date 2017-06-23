#include <stdio.h>
#include <stdint.h>

#include "input_interrupt.h"
#include "event_queue.h"
#include "driver_state.h"
#include "soft_timer.h"
#include "driver_devices.h"

typedef struct Device {
  GPIOAddress address;
  GPIODir direction;
  InterruptEdge edge;
  GPIOAltFn alt_function;
} Device;

int main() {
	
	// Initialize the state machines to be used, along with their default settings
  FSMGroup fsm_group;
	state_init(&fsm_group);

  Devices devices;
  device_init(&devices);

	// Initialize other devices to be used
	event_queue_init();
  soft_timer_init();

	Event e;	

  for (;;) {
    input_callback(&devices.inputs[1], &fsm_group);
		for (uint8_t i = 0; i < 10; i++) {
			if (!event_process(&e)) {
				state_process_event(&fsm_group, &e);
				printf("Event = %d : Car Status = %d : Direction = %d : Turn = %d : Hazard = %d : %d \n",
     				e.id,
     				fsm_group.pedal.state,
     				fsm_group.direction.state,
     				fsm_group.turn_signal.state,
     				fsm_group.hazard_light.state,
     				ADC1->DR);
			}
		}
  }

}

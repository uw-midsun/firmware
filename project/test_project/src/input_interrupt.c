#include "input_interrupt.h"

#include <stdio.h>
#include <string.h>

// Replace the switch statement with a jump table if they get too big

static InputEvent prv_get_event(GPIOAddress* address, GPIOState key_pressed, FSMGroup* fsm_group) {

	switch (address->pin) {
		case 0:
			return (!strcmp(fsm_group->pedal_fsm.current_state->name, "state_off")) ? INPUT_EVENT_POWER_ON : INPUT_EVENT_POWER_OFF; 

		case 1:
			return (key_pressed) ? INPUT_EVENT_GAS_PRESSED : INPUT_EVENT_GAS_RELEASED; 
			break;

		case 2:
			return (key_pressed) ? INPUT_EVENT_BRAKE_PRESSED : INPUT_EVENT_BRAKE_RELEASED; 
			break;

		case 3:
			return (key_pressed) ? INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL : INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
			break;
	}
}

void input_callback (GPIOAddress* address, FSMGroup* fsm_group) {
	GPIOState key_pressed; 
	gpio_get_value(address, &key_pressed);
	debounce(address, &key_pressed);

	printf("P%c%d has been %s\n", address->port + 65, address->pin, (key_pressed) ? "pressed" : "released");
	
	Event e = { prv_get_event(address, key_pressed, fsm_group), 0 };

	switch (e.id) {
		case INPUT_EVENT_POWER_OFF:
			if (strcmp(fsm_group->pedal_fsm.current_state->name, "state_idle")) {
				printf("Cannot power off while pedals are pressed\n");
				break;
			}
			
			if (strcmp(fsm_group->direction_fsm.current_state->name, "state_neutral")) {
				printf("Cannot power off until in neutral\n");
				break;
			}
				
			fsm_process_event(&fsm_group->pedal_fsm, &e);
			break;
		
		case INPUT_EVENT_GAS_PRESSED:
            if (strcmp(fsm_group->direction_fsm.current_state->name, "state_forward") && 
				strcmp(fsm_group->direction_fsm.current_state->name, "state_reverse")) {
				printf("Cannot start moving unless in forward or reverse\n");
				break;
			}
			
			fsm_process_event(&fsm_group->pedal_fsm, &e);
			break;
		
		case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:		
		case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:		
		case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
			if (!(strcmp(fsm_group->pedal_fsm.current_state->name, "state_brake"))) {
				fsm_process_event(&fsm_group->direction_fsm, &e);
			}
            break;
	
		default:		
			if (e.id >= 9 && e.id <= 11) {
				fsm_process_event(&fsm_group->direction_fsm, &e);
			} else if ( e.id >=14 && e.id <= 16) {
				fsm_process_event(&fsm_group->turn_signal_fsm, &e);
			} else {			
				fsm_process_event(&fsm_group->pedal_fsm, &e);
			}
	}
	
	printf("P%c%d : Event %d : Pedal State - %s : Direction State - %s : Turn State - %s\n", 
			(uint8_t)(address->port+65), 
			address->pin, 
			e.id,
			fsm_group->pedal_fsm.current_state->name,
			fsm_group->direction_fsm.current_state->name,
			fsm_group->turn_signal_fsm.current_state->name
	);
	
	return;
}

static uint8_t prv_get_direction_selector(GPIOAddress* address) {
	GPIOState key_pressed[2];
	gpio_get_value(&address[0], &key_pressed[0]);
	gpio_get_value(&address[1], &key_pressed[1]);
	
	uint8_t dir_state = (key_pressed[0] << 1) + (key_pressed[1]);
	return dir_state;
}


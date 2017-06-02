#include "input_interrupt.h"

#include <stdio.h>
#include <string.h>

// Replace the switch statement with a jump table if they get too big

static InputEvent prv_get_event(GPIOAddress* address, FSMGroup* fsm_group) {
	GPIOState key_pressed;
  gpio_get_value(address, &key_pressed);
	debounce(address, &key_pressed);

	switch (address->pin) {
		case 0:
			return (!strcmp(fsm_group->pedal_fsm.current_state->name, "state_off")) ? INPUT_EVENT_POWER_ON : INPUT_EVENT_POWER_OFF; 

		case 1:
			return (adc_read(address, MAX_SPEED) > PEDAL_THRESHOLD) ? INPUT_EVENT_GAS_PRESSED : INPUT_EVENT_GAS_RELEASED; 
			break;

		case 2:
			return (key_pressed) ? INPUT_EVENT_BRAKE_PRESSED : INPUT_EVENT_BRAKE_RELEASED; 
			break;

		case 3:
		case 4:
			switch ((GPIOB->IDR & (GPIO_IDR_3 | GPIO_IDR_4)) >> 3) {
				case 0:
					return INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
				case 1:
					return INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
				case 2: 
					return INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
			}
			break;
	}
}

void input_callback (GPIOAddress* address, FSMGroup* fsm_group) {
	Event e = { prv_get_event(address, fsm_group), 0 };
	bool transitioned = 0;

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
				
			transitioned = fsm_process_event(&fsm_group->pedal_fsm, &e);
			break;
		
		case INPUT_EVENT_GAS_PRESSED:
            if (strcmp(fsm_group->direction_fsm.current_state->name, "state_forward") && 
				strcmp(fsm_group->direction_fsm.current_state->name, "state_reverse")) {
				printf("Cannot start moving unless in forward or reverse\n");
				break;
			}
			
			transitioned = fsm_process_event(&fsm_group->pedal_fsm, &e);
			break;
		
		case INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL:		
		case INPUT_EVENT_DIRECTION_SELECTOR_DRIVE:		
		case INPUT_EVENT_DIRECTION_SELECTOR_REVERSE:
			if (!(strcmp(fsm_group->pedal_fsm.current_state->name, "state_brake"))) {
				transitioned = fsm_process_event(&fsm_group->direction_fsm, &e);
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
	
	printf("P%c%d : Event = %d : Transitioned = %d : Pedal State = %s : Direction State = %s : Turn State = %s \n", 
			(uint8_t)(address->port+65), 
			address->pin, 
			e.id,
			transitioned,
			fsm_group->pedal_fsm.current_state->name,
			fsm_group->direction_fsm.current_state->name,
			fsm_group->turn_signal_fsm.current_state->name
	);
	
	return;
}


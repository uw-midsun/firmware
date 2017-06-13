#include "input_interrupt.h"
#include "event_queue.h"

#include <stdio.h>
#include <string.h>

// Replace the switch statement with a jump table if they get too big

static InputEvent prv_get_event(GPIOAddress* address, FSMGroup* fsm_group, uint16_t reading) {
  GPIOState key_pressed;
  gpio_get_value(address, &key_pressed);
  debounce(address, &key_pressed);

  switch (address->pin) {
    case 0:
      return (GPIO_STATE_HIGH) ? INPUT_EVENT_POWER_ON : INPUT_EVENT_POWER_OFF;

  case 1:
      if (reading < COAST_THRESHOLD) {
        return INPUT_EVENT_GAS_BRAKE;
      } else if (reading > DRIVE_THRESHOLD) {
        return INPUT_EVENT_GAS_PRESSED;
      } else {
        return INPUT_EVENT_GAS_COAST;
      }
      break;

    case 2:
    case 3:
      switch ((GPIOB->IDR & (GPIO_IDR_2 | GPIO_IDR_3)) >> 2) {
        case 0:
          return INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
        case 1:
          return INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
        case 2:
          return INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
      }
        break;

    case 4:
      return (strcmp(fsm_group->pedal_fsm.current_state->name,
              "state_cruise_control")) ? INPUT_EVENT_CRUISE_CONTROL_ON : INPUT_EVENT_CRUISE_CONTROL_OFF;
      break;

    case 5:
      if (!strcmp(fsm_group->pedal_fsm.current_state->name, "state_cruise_control")) {
        printf("Cruise control increase speed\n");
      }
      return INPUT_EVENT_CRUISE_CONTROL_INC;
      break;

    case 6:
      if (!strcmp(fsm_group->pedal_fsm.current_state->name, "state_cruise_control")) {
        printf("Cruise control decrease speed\n");
      }
      return INPUT_EVENT_CRUISE_CONTROL_DEC;
      break;

    case 7:
    case 8:
      switch ((GPIOC->IDR & (GPIO_IDR_7 | GPIO_IDR_8)) >> 7) {
        case 0:
          return INPUT_EVENT_TURN_SIGNAL_NONE;
        case 1:
          return INPUT_EVENT_TURN_SIGNAL_LEFT;
        case 2:
          return INPUT_EVENT_TURN_SIGNAL_RIGHT;
      }
      break;

    case 9:
      return (!strcmp(fsm_group->hazard_light_fsm.current_state->name,
              "state_hazard_off")) ? INPUT_EVENT_HAZARD_LIGHT_ON : INPUT_EVENT_HAZARD_LIGHT_OFF;
  }
}

void input_callback(GPIOAddress* address, FSMGroup* fsm_group) {
	GPIOAddress pedal = { 2, 1 };
	uint16_t reading = adc_read(&pedal, MAX_SPEED);
  
	Event e = { prv_get_event(address, fsm_group, reading), 0 };
  event_raise(&e);

  //printf("Device Pin = P%c%d | Returning %d\n", (uint8_t)(address->port+65), address->pin, e.data); 

  return;
}
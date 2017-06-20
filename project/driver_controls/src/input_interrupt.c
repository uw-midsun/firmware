#include "input_interrupt.h"
#include "event_queue.h"

#include <stdio.h>
#include <string.h>

static InputEvent prv_get_event(GPIOAddress* address, FSMGroup* fsm_group) {
  GPIOState key_pressed;
  uint16_t reading;

  gpio_get_value(address, &key_pressed);
  debounce(address, &key_pressed);

  switch (address->pin) {
    case 0:
      return (fsm_group->pedal.state == STATE_OFF) ? INPUT_EVENT_POWER_ON : INPUT_EVENT_POWER_OFF;

  case 1:
      reading = adc_read(address, MAX_SPEED);
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
      return (fsm_group->pedal.state == STATE_CRUISE_CONTROL) ? INPUT_EVENT_CRUISE_CONTROL_OFF : INPUT_EVENT_CRUISE_CONTROL_ON;
      break;

    case 5:
      if (fsm_group->pedal.state == STATE_CRUISE_CONTROL) {
        printf("Cruise control increase speed\n");
      }
      return INPUT_EVENT_CRUISE_CONTROL_INC;
      break;

    case 6:
      if (fsm_group->pedal.state == STATE_CRUISE_CONTROL) {
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
      return (fsm_group->hazard_light.state == STATE_HAZARD_OFF) ? INPUT_EVENT_HAZARD_LIGHT_ON : INPUT_EVENT_HAZARD_LIGHT_OFF;
  }
}

void input_callback(GPIOAddress* address, FSMGroup* fsm_group) {
	Event e = { prv_get_event(address, fsm_group), 0 };
  event_raise(&e);
  
  //printf("Device Pin = P%c%d\n", (uint8_t)(address->port+65), address->pin); 
  
  return;
}
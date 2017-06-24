#include "adc.h"
#include "input_interrupt.h"
#include "event_queue.h"
#include "stm32f0xx.h"

static InputEvent prv_get_event(GPIOAddress *address) {
  GPIOState key_pressed;
  uint16_t reading;

  gpio_get_value(address, &key_pressed);
  debounce(address, &key_pressed);

  switch (address->pin) {
    case 0:
      return INPUT_EVENT_POWER;

  case 1:
      reading = 0;
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
      return INPUT_EVENT_CRUISE_CONTROL;
      break;

    case 5:
      return INPUT_EVENT_CRUISE_CONTROL_INC;
      break;

    case 6:
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
      return INPUT_EVENT_HAZARD_LIGHT;
  }
}

void input_callback(GPIOAddress *address) {
	event_raise(prv_get_event(address), 0);  
  return;
}
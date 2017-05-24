#include <stdio.h>
#include <stdint.h>

#include "fsm.h"
#include "interrupt.h"
#include "gpio.h"
#include "gpio_it.h"
#include "event_queue.h"
#include "input_interrupt.h"

#define INPUT_DEVICES 17
#define CAN_PINS 2

/* TODO: 
		- Keep lookup table of the states and their allowed devices
		- May need lookup table to make sure (Put that in input_interrupt.h) 
		- Define the output functions for events
		- Once you can ensure that this works, for the love of God, get the FSM daclarations into another own file. 
		- For now, just get the CAN messages printed to stdout. We'll deal with bus communication later.
*/

typedef enum {
  INPUT_EVENT_POWER_ON,
  INPUT_EVENT_POWER_OFF,
  INPUT_EVENT_GAS_PRESSED,
  INPUT_EVENT_GAS_RELEASED,
  INPUT_EVENT_BRAKE_PRESSED,
  INPUT_EVENT_BRAKE_RELEASED,
  INPUT_EVENT_HORN_PRESSED,
  INPUT_EVENT_HORN_RELEASED,
  INPUT_EVENT_EMERGENCY_STOP,
  INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,
  INPUT_EVENT_DIRECTION_SELECTOR_DRIVE,
  INPUT_EVENT_DIRECTION_SELECTOR_REVERSE,
  INPUT_EVENT_HEADLIGHTS_ON,
  INPUT_EVENT_HEADLIGHTS_OFF,
  INPUT_EVENT_TURN_SIGNAL_LEFT,
  INPUT_EVENT_TURN_SIGNAL_RIGHT,
  INPUT_EVENT_HAZARD_LIGHT_ON,
  INPUT_EVENT_HAZARD_LIGHT_OFF,
  INPUT_EVENT_CRUISE_CONTROL_ON,
  INPUT_EVENT_CRUISE_CONTROL_OFF,
  INPUT_EVENT_REGEN_STRENGTH_OFF,
  INPUT_EVENT_REGEN_STRENGTH_WEAK,
  INPUT_EVENT_REGEN_STRENGTH_ON
} InputEvent;

// Off State: when the car is not receiving power
FSM_DECLARE_STATE(state_off);

// Idle State: when neither of the gas pedals are pressed
FSM_DECLARE_STATE(state_idle_neutral);
FSM_DECLARE_STATE(state_idle_forward);
FSM_DECLARE_STATE(state_idle_reverse);

// Brake State: when the driver is holding down the brake pedal
FSM_DECLARE_STATE(state_brake_neutral);
FSM_DECLARE_STATE(state_brake_forward);
FSM_DECLARE_STATE(state_brake_reverse);

// Driving State: when the car is in motion due to the gas pedal
FSM_DECLARE_STATE(state_driving_forward);
FSM_DECLARE_STATE(state_driving_reverse);

// State table for off state
FSM_STATE_TRANSITION(state_off) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_ON, state_idle_neutral);
}

// State table for idle superstate
FSM_STATE_TRANSITION(state_idle_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_POWER_OFF, state_off);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_idle_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_idle_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_PRESSED, state_driving_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

// State table for brake superstate
FSM_STATE_TRANSITION(state_brake_neutral) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_brake_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_brake_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_brake_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, state_brake_neutral);
  FSM_ADD_TRANSITION(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_RELEASED, state_idle_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

// State table for driving superstate
FSM_STATE_TRANSITION(state_driving_forward) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_RELEASED, state_idle_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_forward);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

FSM_STATE_TRANSITION(state_driving_reverse) {
  FSM_ADD_TRANSITION(INPUT_EVENT_GAS_RELEASED, state_idle_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_BRAKE_PRESSED, state_brake_reverse);
  FSM_ADD_TRANSITION(INPUT_EVENT_EMERGENCY_STOP, state_off);
}

int main() {
  FSM fsm;
  fsm_init(&fsm, "driver_fsm", &state_off, &fsm);

  gpio_init();
  interrupt_init();
  gpio_it_init();

  GPIOAddress input[INPUT_DEVICES] = { 
	{ 0, 10 },	// Soft Power switch 
	{ 1, 3 },	// Soft Power switch LED
    { 1, 4 },	// Direction selector 0
    { 1, 5 },   // Direction selector 1
	{ 1, 6 },   // Headlight toggle
	{ 1, 7 },   // Hazardlight toggle 
	{ 2, 13 },  // Hazardlight indicator light
	{ 0, 10 },  // Regen low 
    { 0, 9 },   // Regen high
	{ 0, 8 },   // BMS fault LED
	{ 0, 15 },  // Left-turn signal
	{ 0, 14 },  // Right-turn signal
	{ 0, 0 },   // Push-to-talk
    { 1, 13 },  // Cruise control toggle
    { 1, 12 },  // Cruise control up
    { 1, 11 },  // Cruise control down
    { 1, 10 }   // Horn
  };

  GPIOAddress output = { 2, 6 };

  GPIOAddress can_address[CAN_PINS] = { { 1, 8 }, { 1, 9 } };

  GPIOSettings gpio_settings = { GPIO_DIR_OUT, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  gpio_init_pin(&output, &gpio_settings);

  gpio_settings = (GPIOSettings){ GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };
 
  for (volatile uint8_t i=0; i < INPUT_DEVICES; i++) {
    gpio_init_pin(&input[i], &gpio_settings);
    gpio_it_register_interrupt(&input[i], &it_settings, INTERRUPT_EDGE_RISING, input_callback, &output);
  }

  for (;;) {}
}

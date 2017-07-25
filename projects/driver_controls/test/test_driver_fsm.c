#include "event_arbiter.h"
#include "unity.h"
#include "status.h"
#include "log.h"
#include "input_event.h"
#include "test_helpers.h"
#include "event_queue.h"

#include "power_fsm.h"
#include "pedal_fsm.h"
#include "direction_fsm.h"
#include "turn_signal_fsm.h"
#include "hazard_light_fsm.h"
#include "mechanical_brake_fsm.h"
#include "can_fsm.h"

typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
  FSM can;
} FSMGroup;

static FSMGroup s_fsm_group;
static bool powered = false;

static void prv_toggle_power(bool new_state) {
  Event e;
  if (new_state != powered) {
    e.id = INPUT_EVENT_POWER;
    event_arbiter_process_event(&e);
    powered = new_state;
  }
}

void setup_test() {
  event_queue_init();
}

void teardown_test(void) {
  Event e;

  e.id = INPUT_EVENT_PEDAL_BRAKE;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  event_arbiter_process_event(&e);

  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);
}

void test_driver_fsm_setup() {
  TEST_ASSERT_OK(power_fsm_init(&s_fsm_group.power));
  TEST_ASSERT_OK(pedal_fsm_init(&s_fsm_group.pedal));
  TEST_ASSERT_OK(direction_fsm_init(&s_fsm_group.direction));
  TEST_ASSERT_OK(turn_signal_fsm_init(&s_fsm_group.turn_signal));
  TEST_ASSERT_OK(hazard_light_fsm_init(&s_fsm_group.hazard_light));
  TEST_ASSERT_OK(mechanical_brake_fsm_init(&s_fsm_group.mechanical_brake));
  TEST_ASSERT_OK(can_fsm_init(&s_fsm_group.can));

  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_can_transmit", s_fsm_group.can.current_state->name);
}

// Test that the power fsm correctly generates CAN events
/*
  printf("Event data = %d [%d:%d]\n",
          e.data, data->components.state,
          data->components.data);
*/
void test_driver_fsm_can_power() {
  Event e;
  
  InputEventData *data = &e.data;

  prv_toggle_power(true);
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_POWER, e.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_ON, data->components.state);

  prv_toggle_power(false);
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_POWER, e.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_OFF, data->components.state);
}

void test_driver_fsm_can_mechanical_brake() {
  Event e = { .data = 0xdef };

  InputEventData *data = &e.data;

  // Test that pressing the brake state generates the correct event
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_MECHANICAL_BRAKE, e.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_ENGAGED, data->components.state);
  TEST_ASSERT_EQUAL(0xdef, data->components.data);

  // Test that releasing the brake generates the correct event
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_MECHANICAL_BRAKE, e.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_DISENGAGED, data->components.state);
  TEST_ASSERT_EQUAL(0xdef, data->components.data);
}

void test_driver_fsm_can_hazard_light() {
  Event e;
  InputEventData *data = &e.data;

  // Turn on the power and clean up the event queue
  prv_toggle_power(true);
  event_process(&e);

  // Test that the transition to the ON state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_HAZARD_LIGHT, e.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_ON, data->components.state);

  // Test that the transition to the OFF state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_HAZARD_LIGHT, e.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_OFF, data->components.state);
}

void test_driver_fsm_can_turn_signal() {
  Event e;
  InputEventData *data = &e.data;

  // Turn on the power and clean up the event queue
  prv_toggle_power(true);
  event_process(&e);

  // Test that a left turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_LEFT_SIGNAL, data->components.state);

  // Test that a right turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_RIGHT_SIGNAL, data->components.state);

  // Test that turning the signals off generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_NO_SIGNAL, data->components.state);
}


void test_driver_fsm_can_direction() {
  Event e;
  InputEventData *data = &e.data;

  // Setup for the direction selector to be used
  prv_toggle_power(true);
  event_process(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  // Test that a forward shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_DIRECTION_SELECTOR, e.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_FORWARD, data->components.state);

  // Test that a reverse shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_DIRECTION_SELECTOR, e.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_REVERSE, data->components.state);

  // Test that a neutral shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_DIRECTION_SELECTOR, e.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_NEUTRAL, data->components.state);

  // Release mechanical brake
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_driver_fsm_can_pedal() {
  Event e = { .data = 0xdef };
  InputEventData *data = &e.data;

  // Setup for the pedals to be used
  prv_toggle_power(true);
  event_process(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  // Test that coasting generates the correct event
  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_COAST, data->components.state);

  // Test that pressing the gas generates the correct event
  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_DRIVING, data->components.state);

  // Test that cruise control generates the correct event
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_CRUISE_CONTROL, data->components.state);

  // Exit cruise control
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  // Test that regen brakes generate the correct event
  e.id = INPUT_EVENT_PEDAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_BRAKE, data->components.state);
}


// Check that nothing can happen while the car is powered off
void test_driver_fsm_power_off() {
  Event e;

  prv_toggle_power(false);
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);

  // Shift to forward gear and move the car
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
}

// Check that the car does not move when the mechanical brake is pressed
void test_driver_fsm_mechanical_brake() {
  Event e;

  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Shift to forward gear and move the car
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}


// Move the car forward and then back up
void test_driver_fsm_move_car() {
  Event e;

  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Shift to forward gear and move the car
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Shift to reverse gear and move the car
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

// Turn on the car and then move into cruise control
void test_driver_fsm_cruise_control() {
  Event e;

  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Change gear to forward
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Coast and enter cruise control
  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Increase and decrease cruise control speed
  e.id = INPUT_EVENT_CRUISE_CONTROL_INC;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_CRUISE_CONTROL_DEC;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Get out of cruise control
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Ensure that cruise control does not happen in reverse
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
}

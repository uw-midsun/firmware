#include "driver_state.h"
#include "unity.h"

static FSMGroup s_fsm_group;

void prv_process_event(EventID id, bool expected) {
  Event e = {
    .id = id,
    .data = 0
  };

  if (expected) {
    TEST_ASSERT_TRUE(state_process_event(&s_fsm_group, &e));
  } else {
    TEST_ASSERT_FALSE(state_process_event(&s_fsm_group, &e));
  }
}

void setup_test() {
  state_init(&s_fsm_group);
}

void teardown_test(void) { }

void test_driver_init() {
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
}

void test_power() {
  // Test that the FSM will transition in and out of the off state 
  prv_process_event(INPUT_EVENT_POWER, 1);
  prv_process_event(INPUT_EVENT_POWER, 1);

  // Test that the FSM will not turn off when not in neutral
  prv_process_event(INPUT_EVENT_POWER, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 1);
  prv_process_event(INPUT_EVENT_POWER, 0);

  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, 1);
  prv_process_event(INPUT_EVENT_POWER, 0);

  // Turn off car
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);
  prv_process_event(INPUT_EVENT_POWER, 1);
}

void test_pedal_forward() {
  // Turn on car
  prv_process_event(INPUT_EVENT_POWER, 1);
  
  // Ensure that the car cannot be moved while in neutral
  prv_process_event(INPUT_EVENT_GAS_COAST, 0);
  prv_process_event(INPUT_EVENT_GAS_PRESSED, 0);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 0);

  // Test that the car can be moved in forward gear

  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 1);

  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_GAS_PRESSED, 1);
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);

  prv_process_event(INPUT_EVENT_GAS_PRESSED, 1);
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);

  // Turn off car
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);
  prv_process_event(INPUT_EVENT_POWER, 1);
}

void test_pedal_reverse() {
  // Turn on car
  prv_process_event(INPUT_EVENT_POWER, 1);
  
  // Ensure that the car cannot be moved while in neutral
  prv_process_event(INPUT_EVENT_GAS_COAST, 0);
  prv_process_event(INPUT_EVENT_GAS_PRESSED, 0);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 0);

  // Test that the car can be moved in forward gear
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, 1);
  
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_GAS_PRESSED, 1);
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);

  prv_process_event(INPUT_EVENT_GAS_PRESSED, 1);
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);

  // Turn off car
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);
  prv_process_event(INPUT_EVENT_POWER, 1);  
}

void test_cruise_control() {
  // Turn on car
  prv_process_event(INPUT_EVENT_POWER, 1);
  
  // Ensure that cruise control cannot be activated from brake
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 0);

  // Switch to drive
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 1);


  // Test cruise control from coast state

  // Exit via brake
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 1);
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);
  
  // Exit via cruise control shutoff
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 1);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 1);
  
  // Test cruise control from driving state

  // Exit via brake
  prv_process_event(INPUT_EVENT_GAS_PRESSED, 1);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 1);
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);
  
  // Exit via cruise control shutoff
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 1);
  prv_process_event(INPUT_EVENT_CRUISE_CONTROL, 1);

  // Turn off car
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);
  prv_process_event(INPUT_EVENT_POWER, 1);    
}

void test_direction_selector() {
  // Turn on car
  prv_process_event(INPUT_EVENT_POWER, 1);

  // Test that transitions can be made when braked
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);

  // Test that the direction cannot be changed if not in neutral
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 1);
  prv_process_event(INPUT_EVENT_GAS_COAST, 1);

  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 0);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, 0);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 0);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_REVERSE, 0);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, 0);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 0);

  // Turn off car
  prv_process_event(INPUT_EVENT_GAS_BRAKE, 1);
  prv_process_event(INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL, 1);
  prv_process_event(INPUT_EVENT_POWER, 1);    

}

void test_turn_signal() {
  // Turn on car
  prv_process_event(INPUT_EVENT_POWER, 1);

  // Test that turn signals work when car is powered
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_LEFT, 1);
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_NONE, 1);
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_RIGHT, 1);
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_NONE, 1);

  // Turn off car
  prv_process_event(INPUT_EVENT_POWER, 1);

  // Test that turn signals don't work when car isn't powered
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_LEFT, 0);
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_NONE, 0);
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_RIGHT, 0);
  prv_process_event(INPUT_EVENT_TURN_SIGNAL_NONE, 0);
}

void test_hazard_light() {
  // Turn on car
  prv_process_event(INPUT_EVENT_POWER, 1);

  // Test that hazard lights work when car is powered
  prv_process_event(INPUT_EVENT_HAZARD_LIGHT, 1);

  // Turn off car
  prv_process_event(INPUT_EVENT_POWER, 1); 

  // Test that hazard lights don't work when car isn't powered
  prv_process_event(INPUT_EVENT_HAZARD_LIGHT, 0);
}

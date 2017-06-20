#include "driver_state.h"
#include "unity.h"

static FSMGroup s_fsm_group;

void setup_test() {
  state_init(&fsm_group);
}

void test_driver_init() {
  TEST_ASSERT_EQUAL(STATE_OFF, s_fsm_group.pedal.state);
  TEST_ASSERT_EQUAL(STATE_NEUTRAL, s_fsm_group.pedal.state);
  TEST_ASSERT_EQUAL(STATE_NO_SIGNAL, s_fsm_group.pedal.state);
  TEST_ASSERT_EQUAL(STATE_HAZARD_OFF, s_fsm_group.pedal.state);  
}

void test_power() {
  // Ensure that the state machine transitions out of the off state 
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_POWER_ON);
  TEST_ASSERT_EQUAL(STATE_BRAKE, s_fsm_group.pedal.state);

  // Ensure that the FSM does not transition into the off state when not in brake
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_GAS_COAST);
  TEST_ASSERT_EQUAL(STATE_COAST, s_fsm_group.pedal.state);
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_POWER_OFF);
  TEST_ASSERT_EQUAL(STATE_COAST, s_fsm_group.pedal.state);     

  state_process_event(s_fsm_group.pedal, INPUT_EVENT_GAS_PRESSED);
  TEST_ASSERT_EQUAL(STATE_DRIVE, s_fsm_group.pedal.state);
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_POWER_OFF);
  TEST_ASSERT_EQUAL(STATE_DRIVE, s_fsm_group.pedal.state);

  // Ensure that the FSM does not transition into the off state when not in neutral
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_GAS_BRAKE);
  TEST_ASSERT_EQUAL(STATE_BRAKE, s_fsm_group.pedal.state);

  state_process_event(s_fsm_group.direction, INPUT_EVENT_DIRECTION_SELECTOR_DRIVE);
  TEST_ASSERT_EQUAL(STATE_FORWARD, s_fsm_group.direction.state);
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_POWER_OFF);
  TEST_ASSERT_EQUAL(STATE_FORWARD, s_fsm_group.pedal.state);

  state_process_event(s_fsm_group.direction, INPUT_EVENT_DIRECTION_SELECTOR_REVERSE);
  TEST_ASSERT_EQUAL(STATE_REVERSE, s_fsm_group.direction.state);
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_POWER_OFF);
  TEST_ASSERT_EQUAL(STATE_REVERSE, s_fsm_group.pedal.state);

  // Ensure the car turns off when both brake and neutral are active
  state_process_event(s_fsm_group.direction, INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL);
  TEST_ASSERT_EQUAL(STATE_NEUTRAL, s_fsm_group.direction.state);
  TEST_ASSERT_EQUAL(STATE_BRAKE, s_fsm_group.direction.state);
  state_process_event(s_fsm_group.pedal, INPUT_EVENT_POWER_OFF);
  TEST_ASSERT_EQUAL(STATE_OFF, s_fsm_group.pedal.state);

}
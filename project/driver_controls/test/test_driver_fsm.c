#include "driver_state.h"
#include "unity.h"
#include "status.h"

#include "power_state.h"
#include "pedal_state.h"
#include "direction_state.h"
#include "turn_signal_state.h"
#include "hazard_light_state.h"
#include "mechanical_brake.h"

typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
} FSMGroup;

static FSMGroup s_fsm_group;

void setup_test() { }

void teardown_test(void) {
  Event e;

  // Release gas pedal
  e.id = INPUT_EVENT_GAS_BRAKE;
  driver_state_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Shift gear to neutral
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  driver_state_process_event(&e);
 
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  driver_state_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  driver_state_process_event(&e);
}

void test_setup() {
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, driver_state_add_fsm(&s_fsm_group.power, power_state_init));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, driver_state_add_fsm(&s_fsm_group.pedal, pedal_state_init));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, driver_state_add_fsm(&s_fsm_group.direction, direction_state_init));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, driver_state_add_fsm(&s_fsm_group.turn_signal, turn_signal_state_init));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, driver_state_add_fsm(&s_fsm_group.hazard_light, hazard_light_state_init));
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, driver_state_add_fsm(&s_fsm_group.mechanical_brake, mechanical_brake_state_init));

  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);
}

void test_power_fsm() {
  Event e ;

  // Ensure that the FSM does not transition to the on state with the incorrect event 
  for (uint8_t i = INPUT_EVENT_GAS_BRAKE; i < NUM_INPUT_EVENT; i++) {
    e.id = i;
    TEST_ASSERT_FALSE(driver_state_process_event(&e));
    TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  }

  // Check that the power FSM tranisitions when receiving INPUT_EVENT_POWER
  e.id = INPUT_EVENT_POWER;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);

}

void test_pedal_fsm() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Set power to the ON state
  e.id = INPUT_EVENT_POWER;
  driver_state_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Transitions to the driving state should not happen since the since the direction fsm initializes in neutral,
  e.id = INPUT_EVENT_GAS_COAST;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Check that the car moves after switching to the forward direction
  TEST_ASSERT_TRUE(driver_state_process_event(&mech_brake));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_forward", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_TRUE(driver_state_process_event(&mech_brake));

  // Check that the car moves after switching to the forward direction
  e.id = INPUT_EVENT_GAS_COAST;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_coast", s_fsm_group.pedal.current_state->name);
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_driving", s_fsm_group.pedal.current_state->name);
    e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Switch to reverse gear 
  TEST_ASSERT_TRUE(driver_state_process_event(&mech_brake));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_TRUE(driver_state_process_event(&mech_brake));

  // Check that the car moves after switching to the reverse direction
  e.id = INPUT_EVENT_GAS_COAST;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_coast", s_fsm_group.pedal.current_state->name);
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
}

void test_cruise_control() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Check that cruise control cannot be transitioned to while in neutral
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);


  // Switch to forward direction
  driver_state_process_event(&mech_brake);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  driver_state_process_event(&e);
  driver_state_process_event(&mech_brake);

  // Check that cruise control can be entered from the coast state
  e.id = INPUT_EVENT_GAS_COAST;
  driver_state_process_event(&e);

  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_cruise_control", s_fsm_group.pedal.current_state->name);

  // Exit cruise control
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Check that cruise control can be entered from the driving state
  e.id = INPUT_EVENT_GAS_PRESSED;
  driver_state_process_event(&e);

  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_cruise_control", s_fsm_group.pedal.current_state->name);

  // Exit cruise control
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
}

void test_direction_fsm() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Test that transitions can be made while brake is active
  driver_state_process_event(&mech_brake);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_forward", s_fsm_group.direction.current_state->name);
  driver_state_process_event(&mech_brake);
  
  driver_state_process_event(&mech_brake);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_reverse", s_fsm_group.direction.current_state->name);
  driver_state_process_event(&mech_brake);

  driver_state_process_event(&mech_brake);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  driver_state_process_event(&mech_brake);
  

  // Check that transitions are not made when not braked

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  driver_state_process_event(&e); 
  e.id = INPUT_EVENT_GAS_COAST;
  driver_state_process_event(&e);

  // Test that transitions can be made while brake is active
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
}

void test_turn_signal_fsm() {
  Event e;

  // Check that the turn signals do not work when the car is powered off
  e.id = INPUT_EVENT_POWER;
  driver_state_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);

  // Test that turn signals work when car is powered
  e.id = INPUT_EVENT_POWER;
  driver_state_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_left_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_right_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
}

void test_hazard_signal_fsm() {
  Event power = { .id = INPUT_EVENT_POWER };
  Event hazard_light = { .id = INPUT_EVENT_HAZARD_LIGHT };

  driver_state_process_event(&power);
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);

  // Test that hazard lights don't work when the car isn't powered
  TEST_ASSERT_FALSE(driver_state_process_event(&hazard_light));
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);

  // Test that hazard lights work when the car is powered
  driver_state_process_event(&power);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  TEST_ASSERT_TRUE(driver_state_process_event(&hazard_light));
  TEST_ASSERT_EQUAL_STRING("state_hazard_on", s_fsm_group.hazard_light.current_state->name);

  TEST_ASSERT_TRUE(driver_state_process_event(&hazard_light));
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
}

void test_mechanical_brake_fsm() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Check that gear shifts cannot be made while the brake is disengaged
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_FALSE(driver_state_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
}
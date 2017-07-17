#include "event_arbiter.h"
#include "unity.h"
#include "status.h"
#include "log.h"
#include "input_event.h"

#include "power_fsm.h"
#include "pedal_fsm.h"
#include "direction_fsm.h"
#include "turn_signal_fsm.h"
#include "hazard_light_fsm.h"
#include "mechanical_brake_fsm.h"

typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
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

void setup_test() { }

void teardown_test(void) {
  Event e;

  // Release gas pedal
  e.id = INPUT_EVENT_GAS_BRAKE;
  event_arbiter_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Shift gear to neutral
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  event_arbiter_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  event_arbiter_process_event(&e);

  prv_toggle_power(false);
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
}

void test_setup() {
  TEST_ASSERT_OK(event_arbiter_add_fsm(&s_fsm_group.power, power_fsm_init));
  TEST_ASSERT_OK(event_arbiter_add_fsm(&s_fsm_group.pedal, pedal_fsm_init));
  TEST_ASSERT_OK(event_arbiter_add_fsm(&s_fsm_group.direction, direction_fsm_init));
  TEST_ASSERT_OK(event_arbiter_add_fsm(&s_fsm_group.turn_signal, turn_signal_fsm_init));
  TEST_ASSERT_OK(event_arbiter_add_fsm(&s_fsm_group.hazard_light, hazard_light_fsm_init));
  TEST_ASSERT_OK(event_arbiter_add_fsm(&s_fsm_group.mechanical_brake, mechanical_brake_fsm_init));

  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);
}

void test_power_fsm() {
  Event e;

  // Ensure that the FSM does not transition to the on state with the incorrect event
  for (e.id = INPUT_EVENT_GAS_BRAKE; e.id < NUM_INPUT_EVENT; e.id++) {
    if (e.id == INPUT_EVENT_MECHANICAL_BRAKE) {
      continue;
    }

    TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
    TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  }

  // Check that the power FSM tranisitions when receiving INPUT_EVENT_POWER
  e.id = INPUT_EVENT_POWER;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
}

void test_mechanical_brake_fsm() {
  Event e = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_engaged", s_fsm_group.mechanical_brake.current_state->name);

  // Check that the correct events are blocked when the brake is engaged
  InputEvent forbidden_engaged[] = { INPUT_EVENT_GAS_COAST, INPUT_EVENT_GAS_PRESSED,
    INPUT_EVENT_CRUISE_CONTROL, INPUT_EVENT_CRUISE_CONTROL_INC, INPUT_EVENT_CRUISE_CONTROL_DEC };

  for (uint8_t i = 0; i < 5; i++) {
    e.id = forbidden_engaged[i];
    TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
  }

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);

  // Check that the correct events are blocked when the brake is disengaged
  InputEvent forbidden_disengaged[] = { INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL,
    INPUT_EVENT_DIRECTION_SELECTOR_DRIVE, INPUT_EVENT_DIRECTION_SELECTOR_REVERSE };

  for (uint8_t i = 0; i < 3; i++) {
    e.id = forbidden_disengaged[i];
    TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
  }
}

void test_direction_fsm() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Turn on the power
  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Check that the car cannot accelerate in neutral
  e.id = INPUT_EVENT_GAS_COAST;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Check that gear shifts can be made while mechanical brake is engaged, and that the power
  // cannot be turned off when not in neutral
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  event_arbiter_process_event(&e);
  TEST_ASSERT_EQUAL_STRING("state_engaged", s_fsm_group.mechanical_brake.current_state->name);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_forward", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_POWER;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_reverse", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_POWER;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  event_arbiter_process_event(&e);
}

void test_pedal_fsm() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Turn on the power
  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);


  // Check that the car moves after switching to the forward direction
  TEST_ASSERT_TRUE(event_arbiter_process_event(&mech_brake));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_forward", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&mech_brake));

  // Check that the car moves after switching to the forward direction
  e.id = INPUT_EVENT_GAS_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_coast", s_fsm_group.pedal.current_state->name);
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_driving", s_fsm_group.pedal.current_state->name);
    e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
}

void test_cruise_control() {
  Event e;
  Event mech_brake = { .id = INPUT_EVENT_MECHANICAL_BRAKE };

  // Turn on the power
  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Switch to forward direction
  event_arbiter_process_event(&mech_brake);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  event_arbiter_process_event(&e);
  event_arbiter_process_event(&mech_brake);

  // Check that cruise control can be entered from the coast state
  e.id = INPUT_EVENT_GAS_COAST;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_cruise_control", s_fsm_group.pedal.current_state->name);

  // Exit cruise control
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);

  // Check that cruise control can be entered from the driving state
  e.id = INPUT_EVENT_GAS_PRESSED;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_cruise_control", s_fsm_group.pedal.current_state->name);

  // Exit cruise control
  e.id = INPUT_EVENT_GAS_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
}

void test_turn_signal_fsm() {
  Event e;

  // Turn on the power
  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_left_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_right_signal", s_fsm_group.turn_signal.current_state->name);

  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
}

void test_hazard_signal_fsm() {
  Event e;
  Event hazard_light = { .id = INPUT_EVENT_HAZARD_LIGHT };

  // Turn on the power
  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  TEST_ASSERT_TRUE(event_arbiter_process_event(&hazard_light));
  TEST_ASSERT_EQUAL_STRING("state_hazard_on", s_fsm_group.hazard_light.current_state->name);

  TEST_ASSERT_TRUE(event_arbiter_process_event(&hazard_light));
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
}

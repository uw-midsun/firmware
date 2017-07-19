#include "event_arbiter.h"
#include "unity.h"
#include "status.h"
#include "log.h"
#include "input_event.h"
#include "test_helpers.h"

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

  e.id = INPUT_EVENT_PEDAL_BRAKE;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  event_arbiter_process_event(&e);

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
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

  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);
}

void test_driver_fsm_move_car() {
  Event e;

  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Shift to forward gear and move the car
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Shift to reverse gear and move the car
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_driver_fsm_cruise_control() {
  Event e;

  prv_toggle_power(true);
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Change gear to forward
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
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
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_MECHANICAL_BRAKE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Ensure that cruise control does not happen in reverse
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
}


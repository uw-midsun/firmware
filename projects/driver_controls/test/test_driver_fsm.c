#include "event_arbiter.h"
#include "event_queue.h"
#include "input_event.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#include "direction_fsm.h"
#include "hazard_light_fsm.h"
#include "horn_fsm.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"
#include "turn_signal_fsm.h"

typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
  FSM horn;
} FSMGroup;

static FSMGroup s_fsm_group;
static bool powered = false;
static bool mech_brake = false;

static void prv_toggle_power(void) {
  Event e = { .data = 0 };

  e.id = INPUT_EVENT_POWER;
  event_arbiter_process_event(&e);
}

static void prv_toggle_mech_brake(bool new_state) {
  Event e = { .data = 0 };

  e.id = (new_state == true) ? INPUT_EVENT_MECHANICAL_BRAKE_PRESSED
                             : INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;

  event_arbiter_process_event(&e);
  mech_brake = new_state;
}

void setup_test() {
  event_arbiter_init(NULL);

  power_fsm_init(&s_fsm_group.power);
  pedal_fsm_init(&s_fsm_group.pedal);
  direction_fsm_init(&s_fsm_group.direction);
  turn_signal_fsm_init(&s_fsm_group.turn_signal);
  hazard_light_fsm_init(&s_fsm_group.hazard_light);
  mechanical_brake_fsm_init(&s_fsm_group.mechanical_brake);
  horn_fsm_init(&s_fsm_group.horn);

  powered = false;
  mech_brake = false;

  event_queue_init();
}

void teardown_test(void) {}

void test_driver_fsm_setup() {
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_brake", s_fsm_group.pedal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_neutral", s_fsm_group.direction.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_no_signal", s_fsm_group.turn_signal.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_hazard_off", s_fsm_group.hazard_light.current_state->name);
  TEST_ASSERT_EQUAL_STRING("state_disengaged", s_fsm_group.mechanical_brake.current_state->name);
}

// Check that nothing can happen while the car is powered off
void test_driver_fsm_power_off() {
  Event e;

  // Shift to forward gear and move the car
  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  prv_toggle_mech_brake(false);

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
}

// Check that the power FSM behaves properly regardign charge
void test_driver_fsm_power_charge() {
  Event e;

  // Power the car on and off normally
  prv_toggle_mech_brake(true);
  TEST_ASSERT_EQUAL_STRING("state_off_brake", s_fsm_group.power.current_state->name);

  prv_toggle_power();
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  prv_toggle_power();
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);

  // Start charging the car and ensure that the car is not able to move
  prv_toggle_power();
  TEST_ASSERT_EQUAL_STRING("state_charging", s_fsm_group.power.current_state->name);

  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  // Turn off the car
  prv_toggle_mech_brake(false);
  prv_toggle_power();
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);
}

// Check that the car does not move when the mechanical brake is pressed
void test_driver_fsm_mechanical_brake() {
  Event e;

  prv_toggle_mech_brake(true);
  prv_toggle_power();

  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Shift to forward gear and move the car
  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  prv_toggle_mech_brake(false);

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

// Move the car forward and then back up
void test_driver_fsm_move_car() {
  Event e;

  prv_toggle_mech_brake(true);
  prv_toggle_power();
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Shift to forward gear and move the car
  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  prv_toggle_mech_brake(false);

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Shift to reverse gear and move the car
  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  prv_toggle_mech_brake(false);

  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

// Turn on the car and then move into cruise control
void test_driver_fsm_cruise_control() {
  Event e;

  prv_toggle_mech_brake(true);
  prv_toggle_power();
  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);

  // Change gear to forward
  prv_toggle_mech_brake(true);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  prv_toggle_mech_brake(false);

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
  prv_toggle_mech_brake(true);
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  prv_toggle_mech_brake(false);

  // Ensure that cruise control does not happen in reverse
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));
}

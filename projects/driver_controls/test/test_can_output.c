// Test that the CAN output routines are properly generated once the correct events occur

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
#include "push_to_talk_fsm.h"
#include "turn_signal_fsm.h"

#include "can_output.h"

typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
  FSM horn;
  FSM push_to_talk;
} FSMGroup;

static FSMGroup s_fsm_group;
static bool s_mech_brake = false;

static EventArbiterOutputData s_can_output;

static void prv_output(EventArbiterOutputData data) {
  s_can_output.id = data.id;
  s_can_output.state = data.state;
  s_can_output.data = data.data;

  LOG_DEBUG("[ .id = %x, .state = %x, .data = %x ]\n",
          s_can_output.id,
          s_can_output.state,
          s_can_output.data);
}

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
  s_mech_brake = new_state;
}

void setup_test(void) {
  event_arbiter_init(prv_output);

  power_fsm_init(&s_fsm_group.power);
  pedal_fsm_init(&s_fsm_group.pedal);
  direction_fsm_init(&s_fsm_group.direction);
  turn_signal_fsm_init(&s_fsm_group.turn_signal);
  hazard_light_fsm_init(&s_fsm_group.hazard_light);
  mechanical_brake_fsm_init(&s_fsm_group.mechanical_brake);
  horn_fsm_init(&s_fsm_group.horn);
  push_to_talk_fsm_init(&s_fsm_group.push_to_talk);

  s_mech_brake = false;

  event_queue_init();
}

void teardown_test(void) { }

// Test that the power fsm correctly generates CAN events
void test_can_output_power(void) {
  TEST_ASSERT_EQUAL_STRING("state_off", s_fsm_group.power.current_state->name);

  prv_toggle_power();

  TEST_ASSERT_EQUAL_STRING("state_charging", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_POWER, s_can_output.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_CHARGING, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  prv_toggle_mech_brake(true);
  prv_toggle_power();

  TEST_ASSERT_EQUAL_STRING("state_on", s_fsm_group.power.current_state->name);
  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_POWER, s_can_output.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_ON, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  prv_toggle_power();

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_POWER, s_can_output.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_OFF, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

void test_can_output_mechanical_brake(void) {
  // Test that pressing the brake state generates the correct event
  prv_toggle_mech_brake(true);

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_MECHANICAL_BRAKE, s_can_output.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_ENGAGED, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that releasing the brake generates the correct event
  prv_toggle_mech_brake(false);

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_MECHANICAL_BRAKE, s_can_output.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_DISENGAGED, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

void test_can_output_hazard_light(void) {
  Event e = { 0 };

  // Turn on the power and clean up the event queue
  prv_toggle_mech_brake(true);
  prv_toggle_power();

  // Test that the transition to the ON state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;

  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_HAZARD_LIGHT, s_can_output.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_ON, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that the transition to the OFF state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_HAZARD_LIGHT, s_can_output.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_OFF, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

void test_can_output_turn_signal(void) {
  Event e = { 0 };

  // Turn on the power and clean up the event queue
  prv_toggle_mech_brake(true);
  prv_toggle_power();

  // Test that a left turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_TURN_SIGNAL, s_can_output.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_LEFT_SIGNAL, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that a right turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_TURN_SIGNAL, s_can_output.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_RIGHT_SIGNAL, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that turning the signals off generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_TURN_SIGNAL, s_can_output.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_NO_SIGNAL, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

void test_can_output_direction(void) {
  Event e = { 0 };

  // Setup for the direction selector to be used
  prv_toggle_mech_brake(true);
  prv_toggle_power();

  prv_toggle_mech_brake(true);

  // Test that a forward shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_DIRECTION_SELECTOR, s_can_output.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_FORWARD, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that a reverse shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_DIRECTION_SELECTOR, s_can_output.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_REVERSE, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that a neutral shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_DIRECTION_SELECTOR, s_can_output.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_NEUTRAL, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

void test_can_output_pedal(void) {
  Event e = {.data = 0 };

  // Setup for the pedals to be used
  prv_toggle_mech_brake(true);
  prv_toggle_power();

  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  prv_toggle_mech_brake(false);

  // Test that coasting generates the correct event
  e = (Event){.id = INPUT_EVENT_PEDAL_COAST, .data = 0xabcd };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_PEDAL, s_can_output.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_COAST, s_can_output.state);
  TEST_ASSERT_EQUAL(0xabcd, s_can_output.data);

  // Test that pressing the gas generates the correct event
  e = (Event){.id = INPUT_EVENT_PEDAL_PRESSED, .data = 0xabcd };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_PEDAL, s_can_output.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_DRIVING, s_can_output.state);
  TEST_ASSERT_EQUAL(0xabcd, s_can_output.data);

  // Test that cruise control generates the correct event
  e = (Event){.id = INPUT_EVENT_CRUISE_CONTROL, .data = 0xabcd };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_PEDAL, s_can_output.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_CRUISE_CONTROL, s_can_output.state);
  TEST_ASSERT_EQUAL(0xabcd, s_can_output.data);

  // Exit cruise control
  e = (Event){.id = INPUT_EVENT_CRUISE_CONTROL, .data = 0xabcd };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that regen brakes generate the correct event
  e = (Event){.id = INPUT_EVENT_PEDAL_BRAKE, .data = 0xabcd };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_PEDAL, s_can_output.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_BRAKE, s_can_output.state);
  TEST_ASSERT_EQUAL(0xabcd, s_can_output.data);
}

void test_can_output_horn(void) {
  Event e = { 0 };

  prv_toggle_mech_brake(true);
  prv_toggle_power();

  // Test that pressing the horn generates the correct event
  e.id = INPUT_EVENT_HORN;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_HORN, s_can_output.id);
  TEST_ASSERT_EQUAL(HORN_FSM_STATE_ON, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that releasing the horn generates the correct event
  e.id = INPUT_EVENT_HORN;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_HORN, s_can_output.id);
  TEST_ASSERT_EQUAL(HORN_FSM_STATE_OFF, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

void test_can_output_push_to_talk(void) {
  Event e = { 0 };

  prv_toggle_mech_brake(true);
  prv_toggle_power();

  // Test that pressing the horn generates the correct event
  e.id = INPUT_EVENT_PUSH_TO_TALK;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_PUSH_TO_TALK, s_can_output.id);
  TEST_ASSERT_EQUAL(PUSH_TO_TALK_FSM_STATE_ACTIVE, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);

  // Test that releasing the horn generates the correct event
  e.id = INPUT_EVENT_PUSH_TO_TALK;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_MESSAGE_PUSH_TO_TALK, s_can_output.id);
  TEST_ASSERT_EQUAL(PUSH_TO_TALK_FSM_STATE_INACTIVE, s_can_output.state);
  TEST_ASSERT_EQUAL(0, s_can_output.data);
}

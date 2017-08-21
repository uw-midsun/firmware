
// Test that the CAN events can be correctly generated and processed

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
#include "push_to_talk_fsm.h"
#include "horn_fsm.h"

#include "can_fsm.h"

typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
  FSM horn;
  FSM push_to_talk;
  FSM can;
} FSMGroup;

static FSMGroup s_fsm_group;
static bool powered = false;
static bool mech_brake = false;

static void prv_toggle_power(bool new_state) {
  Event e;
  if (new_state != powered) {
    e.id = INPUT_EVENT_POWER;
    event_arbiter_process_event(&e);
    powered = new_state;
  }
}

static void prv_toggle_mech_brake(bool new_state) {
  Event e = { .data = 0 };

  e.id = (new_state == true) ?
            INPUT_EVENT_MECHANICAL_BRAKE_PRESSED :
            INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;

  event_arbiter_process_event(&e);
  mech_brake = new_state;
}

void setup_test() {
  event_arbiter_init();

  power_fsm_init(&s_fsm_group.power);
  pedal_fsm_init(&s_fsm_group.pedal);
  direction_fsm_init(&s_fsm_group.direction);
  turn_signal_fsm_init(&s_fsm_group.turn_signal);
  hazard_light_fsm_init(&s_fsm_group.hazard_light);
  mechanical_brake_fsm_init(&s_fsm_group.mechanical_brake);
  horn_fsm_init(&s_fsm_group.horn);
  push_to_talk_fsm_init(&s_fsm_group.push_to_talk);
  can_fsm_init(&s_fsm_group.can);

  powered = false;
  mech_brake = false;

  event_queue_init();
}

void teardown_test(void) {
  Event e;

  e.id = INPUT_EVENT_PEDAL_BRAKE;
  event_arbiter_process_event(&e);

  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  event_arbiter_process_event(&e);

  prv_toggle_mech_brake(false);
}

// Test that the power fsm correctly generates CAN events
void test_can_fsm_power() {
  Event e = { 0 };

  InputEventData *data = (InputEventData *)&e.data;

  prv_toggle_power(true);
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_POWER, e.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_ON, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  prv_toggle_power(false);
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_POWER, e.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_OFF, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_can_fsm_mechanical_brake() {
  Event e = { 0 };
  InputEventData *data = (InputEventData *)&e.data;

  // Test that pressing the brake state generates the correct event
  prv_toggle_mech_brake(true);
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_MECHANICAL_BRAKE, e.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_ENGAGED, data->components.state);
  TEST_ASSERT_EQUAL(0, data->components.data);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that releasing the brake generates the correct event
  prv_toggle_mech_brake(false);
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_MECHANICAL_BRAKE, e.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_DISENGAGED, data->components.state);
  TEST_ASSERT_EQUAL(0, data->components.data);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_can_fsm_hazard_light() {
  Event e = { 0 };
  InputEventData *data = (InputEventData *)&e.data;

  // Turn on the power and clean up the event queue
  prv_toggle_power(true);
  event_process(&e);

  // Test that the transition to the ON state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_HAZARD_LIGHT, e.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_ON, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that the transition to the OFF state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_HAZARD_LIGHT, e.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_OFF, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_can_fsm_turn_signal() {
  Event e = { 0 };
  InputEventData *data = (InputEventData *)&e.data;

  // Turn on the power and clean up the event queue
  prv_toggle_power(true);
  event_process(&e);

  // Test that a left turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_LEFT_SIGNAL, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that a right turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_RIGHT_SIGNAL, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that turning the signals off generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_TURN_SIGNAL, e.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_NO_SIGNAL, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}


void test_can_fsm_direction() {
  Event e = { 0 };
  InputEventData *data = (InputEventData *)&e.data;

  // Setup for the direction selector to be used
  prv_toggle_power(true);
  event_process(&e);

  prv_toggle_mech_brake(true);
  event_process(&e);

  // Test that a forward shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_DIRECTION_SELECTOR, e.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_FORWARD, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that a reverse shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_DIRECTION_SELECTOR, e.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_REVERSE, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that a neutral shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_DIRECTION_SELECTOR, e.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_NEUTRAL, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_can_fsm_pedal() {
  Event e = { .data = 0xdef };
  InputEventData *data = (InputEventData *)&e.data;

  // Setup for the pedals to be used
  prv_toggle_power(true);
  event_process(&e);

  prv_toggle_mech_brake(true);
  event_process(&e);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  prv_toggle_mech_brake(false);
  event_process(&e);

  // Test that coasting generates the correct event
  e.id = INPUT_EVENT_PEDAL_COAST;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_COAST, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that pressing the gas generates the correct event
  e.id = INPUT_EVENT_PEDAL_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_DRIVING, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that cruise control generates the correct event
  e.id = INPUT_EVENT_CRUISE_CONTROL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_PEDAL, e.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_CRUISE_CONTROL, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

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
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_can_fsm_horn() {
  Event e = { 0 };
  InputEventData *data = (InputEventData *)&e.data;

  prv_toggle_power(true);
  event_process(&e);

  // Test that pressing the horn generates the correct event
  e.id = INPUT_EVENT_HORN;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_HORN, e.id);
  TEST_ASSERT_EQUAL(HORN_FSM_STATE_ON, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that releasing the horn generates the correct event
  e.id = INPUT_EVENT_HORN;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_ID_HORN, e.id);
  TEST_ASSERT_EQUAL(HORN_FSM_STATE_OFF, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

void test_can_fsm_push_to_talk() {
  Event e = { 0 };
  InputEventData *data = (InputEventData *)&e.data;

  prv_toggle_power(true);
  event_process(&e);

  // Test that pressing the horn generates the correct event
  e.id = INPUT_EVENT_PUSH_TO_TALK;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_PUSH_TO_TALK, e.id);
  TEST_ASSERT_EQUAL(PUSH_TO_TALK_FSM_STATE_ACTIVE, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that releasing the horn generates the correct event
  e.id = INPUT_EVENT_PUSH_TO_TALK;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
  event_process(&e);

  TEST_ASSERT_EQUAL(INPUT_EVENT_CAN_PUSH_TO_TALK, e.id);
  TEST_ASSERT_EQUAL(PUSH_TO_TALK_FSM_STATE_INACTIVE, data->components.state);
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));
}

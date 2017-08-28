// Test that the CAN output routines are properly generated once the correct events occur

// Since there isn't a way to view CAN event data directly, the printf statements must be observed
// to see that they are behaving correctly

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
static bool s_powered = false;
static bool s_mech_brake = false;
static Event s_can_event;

static void prv_output(FSM *fsm, EventArbiterOutputData data) {
  union EventData {
    uint16_t raw;
    struct {
      uint16_t data:12;
      uint8_t state:4;
    } components;
  } event_data;

  event_data.components.data = data.data;
  event_data.components.state = data.state;

  s_can_event.id = data.id;
  s_can_event.data = event_data.raw;
}

static void prv_toggle_power(bool new_state) {
  Event e = { .data = 0 };
  if (new_state != s_powered) {
    e.id = INPUT_EVENT_POWER;
    event_arbiter_process_event(&e);
    s_powered = new_state;
  }
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

  s_powered = false;
  s_mech_brake = false;

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
  prv_toggle_power(true);

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_POWER, s_can_event.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_ON, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  prv_toggle_power(false);

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_POWER, s_can_event.id);
  TEST_ASSERT_EQUAL(POWER_FSM_STATE_OFF, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

void test_can_fsm_mechanical_brake() {
  // Test that pressing the brake state generates the correct event
  prv_toggle_mech_brake(true);

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_MECHANICAL_BRAKE, s_can_event.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_ENGAGED, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that releasing the brake generates the correct event
  prv_toggle_mech_brake(false);

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_MECHANICAL_BRAKE, s_can_event.id);
  TEST_ASSERT_EQUAL(MECHANICAL_BRAKE_FSM_STATE_DISENGAGED, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

void test_can_fsm_hazard_light() {
  Event e = { 0 };

  // Turn on the power and clean up the event queue
  prv_toggle_power(true);

  // Test that the transition to the ON state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_HAZARD_LIGHT, s_can_event.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_ON, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that the transition to the OFF state generates the correct event
  e.id = INPUT_EVENT_HAZARD_LIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_HAZARD_LIGHT, s_can_event.id);
  TEST_ASSERT_EQUAL(HAZARD_LIGHT_FSM_STATE_OFF, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

void test_can_fsm_turn_signal() {
  Event e = { 0 };

  // Turn on the power and clean up the event queue
  prv_toggle_power(true);

  // Test that a left turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_LEFT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_TURN_SIGNAL, s_can_event.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_LEFT_SIGNAL, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that a right turn signal generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_TURN_SIGNAL, s_can_event.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_RIGHT_SIGNAL, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that turning the signals off generates the correct event
  e.id = INPUT_EVENT_TURN_SIGNAL_NONE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_TURN_SIGNAL, s_can_event.id);
  TEST_ASSERT_EQUAL(TURN_SIGNAL_FSM_STATE_NO_SIGNAL, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

void test_can_fsm_direction() {
  Event e = { 0 };

  // Setup for the direction selector to be used
  prv_toggle_power(true);

  prv_toggle_mech_brake(true);

  // Test that a forward shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_DIRECTION_SELECTOR, s_can_event.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_FORWARD, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that a reverse shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_DIRECTION_SELECTOR, s_can_event.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_REVERSE, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that a neutral shift generates the correct event
  e.id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_DIRECTION_SELECTOR, s_can_event.id);
  TEST_ASSERT_EQUAL(DIRECTION_FSM_STATE_NEUTRAL, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

void test_can_fsm_pedal() {
  Event e = {.data = 0 };

  // Setup for the pedals to be used
  prv_toggle_power(true);

  prv_toggle_mech_brake(true);

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  prv_toggle_mech_brake(false);

  // Test that coasting generates the correct event
  e = (Event){.id = INPUT_EVENT_PEDAL_COAST, .data = 0xdef };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_PEDAL, s_can_event.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_COAST, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0xdef, (s_can_event.data & 0xFFF));

  // Test that pressing the gas generates the correct event
  e = (Event){.id = INPUT_EVENT_PEDAL_PRESSED, .data = 0xdef };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_PEDAL, s_can_event.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_DRIVING, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0xdef, (s_can_event.data & 0xFFF));

  // Test that cruise control generates the correct event
  e = (Event){.id = INPUT_EVENT_CRUISE_CONTROL, .data = 0xdef };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_PEDAL, s_can_event.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_CRUISE_CONTROL, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0xdef, (s_can_event.data & 0xFFF));

  // Exit cruise control
  e = (Event){.id = INPUT_EVENT_CRUISE_CONTROL, .data = 0xdef };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  // Test that regen brakes generate the correct event
  e = (Event){.id = INPUT_EVENT_PEDAL_BRAKE, .data = 0xdef };
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_PEDAL, s_can_event.id);
  TEST_ASSERT_EQUAL(PEDAL_FSM_STATE_BRAKE, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0xdef, (s_can_event.data & 0xFFF));
}

void test_can_fsm_horn() {
  Event e = { 0 };

  prv_toggle_power(true);

  // Test that pressing the horn generates the correct event
  e.id = INPUT_EVENT_HORN;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_HORN, s_can_event.id);
  TEST_ASSERT_EQUAL(HORN_FSM_STATE_ON, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that releasing the horn generates the correct event
  e.id = INPUT_EVENT_HORN;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_HORN, s_can_event.id);
  TEST_ASSERT_EQUAL(HORN_FSM_STATE_OFF, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

void test_can_fsm_push_to_talk() {
  Event e = { 0 };

  prv_toggle_power(true);

  // Test that pressing the horn generates the correct event
  e.id = INPUT_EVENT_PUSH_TO_TALK;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_PUSH_TO_TALK, s_can_event.id);
  TEST_ASSERT_EQUAL(PUSH_TO_TALK_FSM_STATE_ACTIVE, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));

  // Test that releasing the horn generates the correct event
  e.id = INPUT_EVENT_PUSH_TO_TALK;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));

  TEST_ASSERT_EQUAL(CAN_OUTPUT_DEVICE_PUSH_TO_TALK, s_can_event.id);
  TEST_ASSERT_EQUAL(PUSH_TO_TALK_FSM_STATE_INACTIVE, (s_can_event.data >> 12));
  TEST_ASSERT_EQUAL(0, (s_can_event.data & 0xFFF));
}

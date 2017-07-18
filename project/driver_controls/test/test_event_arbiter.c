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

void setup_test() {
  event_arbiter_init();
}

void teardown_test() {}

void test_event_arbiter_add() {
  FSM fsm;
  for (uint8_t i = 0; i < MAX_FSMS; i++) {
    TEST_ASSERT_OK(power_fsm_init(&fsm));
  }
  TEST_ASSERT_NOT_OK(power_fsm_init(&fsm));
}

void test_event_arbiter_process() {
  power_fsm_init(&s_fsm_group.power);
  pedal_fsm_init(&s_fsm_group.pedal);
  direction_fsm_init(&s_fsm_group.direction);

  Event e;

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_POWER;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e));  

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_FALSE(event_arbiter_process_event(&e));

  e.id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e)); 

  e.id = INPUT_EVENT_GAS_PRESSED;
  TEST_ASSERT_TRUE(event_arbiter_process_event(&e)); 
}
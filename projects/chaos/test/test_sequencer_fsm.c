#include "sequencer_fsm.h"

#include "unity.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "log.h"
#include "test_helpers.h"

void setup_test(void) {
  event_queue_init();
  sequencer_fsm_init();
}

void teardown_test(void) {}

void test_sequencer_fsm_run(void) {
  // Just walk all the states to make sure they work and emit events.
  const Event state_events[] = {
    { .id = CHAOS_EVENT_SEQUENCE_DRIVE },
    { .id = CHAOS_EVENT_SEQUENCE_IDLE },
    { .id = CHAOS_EVENT_SEQUENCE_CHARGE },
    { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY },
  };
  Event prev_event = { 0 };
  Event present_event = { 0 };

  for (uint16_t i = 0; i < SIZEOF_ARRAY(state_events); i++) {
    LOG_DEBUG("State Event: %d\n", state_events[i].id);
    TEST_ASSERT_OK(event_raise(state_events[i].id, 0));
    StatusCode seq_status = STATUS_CODE_OK;
    StatusCode event_status = STATUS_CODE_OK;

    // While there are still events to emit iterate and mock relay responses. Otherwise just
    // feedback the events.
    while (seq_status != STATUS_CODE_RESOURCE_EXHAUSTED) {
      TEST_ASSERT_OK(seq_status);
      event_status = event_process(&present_event);
      if (event_status != STATUS_CODE_OK) {
        if (prev_event.id == CHAOS_EVENT_CLOSE_RELAY) {
          present_event.id = CHAOS_EVENT_RELAY_CLOSED;
          present_event.data = prev_event.data;
        } else if (prev_event.id == CHAOS_EVENT_OPEN_RELAY) {
          present_event.id = CHAOS_EVENT_RELAY_OPENED;
          present_event.data = prev_event.data;
        }
        LOG_DEBUG("No event raised. Responding with: %d : %d\n", present_event.id,
                  present_event.data);
      } else {
        LOG_DEBUG("Sequencer Raised Event: %d : %d\n", present_event.id, present_event.data);
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      prev_event = present_event;
      seq_status = sequencer_fsm_publish_next_event(&prev_event);
    }
  }
}

void test_sequencer_fsm_transitions(void) {
  // Valid: idle -> charge
  Event event = { .id = CHAOS_EVENT_SEQUENCE_CHARGE };
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Invalid: charge -> drive
  event.id = CHAOS_EVENT_SEQUENCE_DRIVE;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sequencer_fsm_publish_next_event(&event));

  // Valid: charge -> charge
  event.id = CHAOS_EVENT_SEQUENCE_RESET;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: charge -> idle
  event.id = CHAOS_EVENT_SEQUENCE_IDLE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: idle -> idle
  event.id = CHAOS_EVENT_SEQUENCE_RESET;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: idle -> charge
  event.id = CHAOS_EVENT_SEQUENCE_CHARGE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: charge -> emergency
  event.id = CHAOS_EVENT_SEQUENCE_EMERGENCY;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: emergency -> emergency
  event.id = CHAOS_EVENT_SEQUENCE_RESET;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Invalid: emergency -> charge
  event.id = CHAOS_EVENT_SEQUENCE_CHARGE;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sequencer_fsm_publish_next_event(&event));

  // Invalid: emergency -> drive
  event.id = CHAOS_EVENT_SEQUENCE_DRIVE;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sequencer_fsm_publish_next_event(&event));

  // Valid: emergency -> idle
  event.id = CHAOS_EVENT_SEQUENCE_IDLE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: idle -> drive
  event.id = CHAOS_EVENT_SEQUENCE_DRIVE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Invalid: drive -> charge
  event.id = CHAOS_EVENT_SEQUENCE_CHARGE;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sequencer_fsm_publish_next_event(&event));

  // Valid: drive -> drive
  event.id = CHAOS_EVENT_SEQUENCE_RESET;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: drive -> idle
  event.id = CHAOS_EVENT_SEQUENCE_IDLE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: idle -> drive
  event.id = CHAOS_EVENT_SEQUENCE_DRIVE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: drive -> emergency
  event.id = CHAOS_EVENT_SEQUENCE_EMERGENCY;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: emergency -> idle
  event.id = CHAOS_EVENT_SEQUENCE_IDLE;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));

  // Valid: idle -> emergency
  event.id = CHAOS_EVENT_SEQUENCE_EMERGENCY;
  TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&event));
}

// Validate that the reset mechanism works as intended.
void test_sequencer_fsm_reset(void) {
  const Event state_event = { .id = CHAOS_EVENT_SEQUENCE_DRIVE };
  Event prev_event = { 0 };
  Event present_event = { 0 };

  bool reset = true;

  TEST_ASSERT_OK(event_raise(state_event.id, 0));
  StatusCode seq_status = STATUS_CODE_OK;
  StatusCode event_status = STATUS_CODE_OK;
  while (seq_status != STATUS_CODE_RESOURCE_EXHAUSTED) {
    TEST_ASSERT_OK(seq_status);
    event_status = event_process(&present_event);
    if (event_status != STATUS_CODE_OK) {
      if (prev_event.id == CHAOS_EVENT_CLOSE_RELAY) {
        if (reset) {
          // Use junk data to force a reset.
          present_event.id = CHAOS_EVENT_RELAY_OPENED;
          present_event.data = prev_event.data;
          reset = false;
        }
        present_event.id = CHAOS_EVENT_RELAY_CLOSED;
        present_event.data = prev_event.data;
      } else if (prev_event.id == CHAOS_EVENT_OPEN_RELAY) {
        present_event.id = CHAOS_EVENT_RELAY_OPENED;
        present_event.data = prev_event.data;
      }
    }
    // At this point |present_event| is considered to be |prev_event| as it has passed through all
    // the other FSMs in the event loop.
    prev_event = present_event;
    seq_status = sequencer_fsm_publish_next_event(&present_event);
  }
}

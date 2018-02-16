#include "sequencer.h"

#include "unity.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "log.h"
#include "test_helpers.h"

void setup_test(void) {
  event_queue_init();
  sequencer_init();
}

void teardown_test(void) {}

void test_sequencer_run(void) {
  const Event state_events[] = {
    { .id = CHAOS_EVENT_SEQUENCE_DRIVE },
    { .id = CHAOS_EVENT_SEQUENCE_IDLE },
    { .id = CHAOS_EVENT_SEQUENCE_CHARGE },
  };
  Event prev_event = { 0 };
  Event present_event = { 0 };

  for (uint16_t i = 0; i < SIZEOF_ARRAY(state_events); i++) {
    TEST_ASSERT_OK(event_raise(state_events[i].id, 0));
    StatusCode seq_status = STATUS_CODE_OK;
    StatusCode event_status = STATUS_CODE_OK;
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
        } else {
          prev_event.id = CHAOS_EVENT_NO_OP;
        }
      }
      seq_status = sequencer_publish_next_event(&present_event);
      prev_event = present_event;
    }
  }
}

void test_sequencer_bad_transition(void) {
  const Event start_event = { .id = CHAOS_EVENT_SEQUENCE_CHARGE };
  TEST_ASSERT_OK(sequencer_publish_next_event(&start_event));
  // Can't drive in charge state.
  const Event bad_event = { .id = CHAOS_EVENT_SEQUENCE_DRIVE };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sequencer_publish_next_event(&bad_event));
}

void test_sequencer_reset(void) {
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
    seq_status = sequencer_publish_next_event(&present_event);
    prev_event = present_event;
  }
}

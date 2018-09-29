#include "sequencer_fsm.h"

#include "unity.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"

static void prv_handle_awaiting(const Event *prev_event, Event *present_event) {
  if (prev_event->id == CHAOS_EVENT_CLOSE_RELAY) {
    present_event->id = CHAOS_EVENT_RELAY_CLOSED;
    present_event->data = prev_event->data;
  } else if (prev_event->id == CHAOS_EVENT_OPEN_RELAY) {
    present_event->id = CHAOS_EVENT_RELAY_OPENED;
    present_event->data = prev_event->data;
  } else if (prev_event->id == CHAOS_EVENT_DELAY_MS) {
    present_event->id = CHAOS_EVENT_DELAY_DONE;
    present_event->data = 0;
  }
}

static void prv_raise_with_delay_cb(SoftTimerId id, void *context) {
  (void)id;
  Event *e = context;
  event_raise(e->id, e->data);
  LOG_DEBUG("Raised: %d:%d\n", e->id, e->data);
}

void setup_test(void) {
  interrupt_init();
  event_queue_init();
  sequencer_fsm_init();
  soft_timer_init();
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
        prv_handle_awaiting(&prev_event, &present_event);
        LOG_DEBUG("No event raised. Responding with: %d : %d\n", present_event.id,
                  present_event.data);
      } else {
        LOG_DEBUG("Sequencer Raised Event: %d : %d\n", present_event.id, present_event.data);
      }

      if (present_event.id <= NUM_CHAOS_EVENTS_CAN) {
        // Skip ignored events.
        continue;
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      prev_event = present_event;
      seq_status = sequencer_fsm_publish_next_event(&prev_event);
    }
  }
}

// Validate that the reset mechanism works as intended for relays.
void test_sequencer_fsm_reset_relay(void) {
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
      } else {
        prv_handle_awaiting(&prev_event, &present_event);
      }
    }
    if (present_event.id <= NUM_CHAOS_EVENTS_CAN) {
      // Skip ignored events.
      continue;
    }
    // At this point |present_event| is considered to be |prev_event| as it has passed through all
    // the other FSMs in the event loop.
    prev_event = present_event;
    seq_status = sequencer_fsm_publish_next_event(&present_event);
  }
  // Ensure the reset actually happened.
  TEST_ASSERT_FALSE(reset);
}

// Validate that the reset mechanism works as intended for sequences.
void test_sequencer_fsm_reset_events(void) {
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
      prv_handle_awaiting(&prev_event, &present_event);
    }
    // Cause a major error!
    if (reset && present_event.id == CHAOS_EVENT_GPIO_DRIVE) {
      present_event.id = CHAOS_EVENT_GPIO_IDLE;
      TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&present_event));
      // Check reset event.
      TEST_ASSERT_OK(event_process(&present_event));
      // Raise some garbage data to check that flushing is working.
      TEST_ASSERT_OK(event_raise(NUM_CHAOS_EVENTS_FSM, 99));
      TEST_ASSERT_OK(
          event_raise_priority(EVENT_PRIORITY_HIGH, present_event.id, present_event.data));
      reset = false;
    } else {
      if (present_event.id <= NUM_CHAOS_EVENTS_CAN) {
        // Skip ignored events.
        continue;
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      prev_event = present_event;
      seq_status = sequencer_fsm_publish_next_event(&present_event);
    }
  }
  // Ensure the reset actually happened.
  TEST_ASSERT_FALSE(reset);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_DRIVE_DONE, present_event.id);
}

// Validate that a transition partway through a transition resolves correctly
void test_sequencer_fsm_interrupted(void) {
  const Event state_event = { .id = CHAOS_EVENT_SEQUENCE_DRIVE };
  Event prev_event = { 0 };
  Event present_event = { 0 };

  bool transition = true;

  TEST_ASSERT_OK(event_raise(state_event.id, 0));
  StatusCode seq_status = STATUS_CODE_OK;
  StatusCode event_status = STATUS_CODE_OK;
  while (seq_status != STATUS_CODE_RESOURCE_EXHAUSTED) {
    TEST_ASSERT_OK(seq_status);
    event_status = event_process(&present_event);
    if (event_status != STATUS_CODE_OK) {
      prv_handle_awaiting(&prev_event, &present_event);
    }
    // Cause a transition.
    if (transition && present_event.id == CHAOS_EVENT_GPIO_DRIVE) {
      present_event.id = CHAOS_EVENT_SEQUENCE_IDLE;
      TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&present_event));
      // Check reset event.
      TEST_ASSERT_OK(event_process(&present_event));
      // Raise some garbage data to check that flushing is working.
      TEST_ASSERT_OK(event_raise(NUM_CHAOS_EVENTS_FSM, 99));
      TEST_ASSERT_OK(
          event_raise_priority(EVENT_PRIORITY_HIGH, present_event.id, present_event.data));
      transition = false;
    } else {
      if (present_event.id <= NUM_CHAOS_EVENTS_CAN) {
        // Skip ignored events.
        continue;
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      prev_event = present_event;
      seq_status = sequencer_fsm_publish_next_event(&present_event);
    }
  }
  // Ensure the transition actually happened.
  TEST_ASSERT_FALSE(transition);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_IDLE_DONE, present_event.id);
}

// Validate that a transition partway through an awaiting transition works.
void test_sequencer_fsm_interrupted_awaiting(void) {
  const Event state_event = { .id = CHAOS_EVENT_SEQUENCE_DRIVE };
  Event prev_event = { 0 };
  Event present_event = { 0 };

  bool transition = true;
  Event delayed_event = { .id = CHAOS_EVENT_RELAY_CLOSED };

  TEST_ASSERT_OK(event_raise(state_event.id, 0));
  StatusCode seq_status = STATUS_CODE_OK;
  StatusCode event_status = STATUS_CODE_OK;
  while (seq_status != STATUS_CODE_RESOURCE_EXHAUSTED) {
    TEST_ASSERT_OK(seq_status);
    event_status = event_process(&present_event);
    if (event_status != STATUS_CODE_OK) {
      prv_handle_awaiting(&prev_event, &present_event);
    }
    // Cause a transition.
    if (transition && present_event.id == CHAOS_EVENT_CLOSE_RELAY) {
      TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&present_event));
      present_event.id = CHAOS_EVENT_SEQUENCE_IDLE;
      // Raise some garbage data to check that flushing is working.
      TEST_ASSERT_OK(event_raise(NUM_CHAOS_EVENTS_FSM, 99));
      TEST_ASSERT_OK(event_raise_priority(EVENT_PRIORITY_HIGH, present_event.id, 0));
      delayed_event.data = present_event.data;
      TEST_ASSERT_OK(soft_timer_start_millis(15, prv_raise_with_delay_cb, &delayed_event, NULL));
      transition = false;
    } else {
      if (present_event.id <= NUM_CHAOS_EVENTS_CAN) {
        // Skip ignored events.
        continue;
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      prev_event = present_event;
      seq_status = sequencer_fsm_publish_next_event(&present_event);
    }
  }
  // Ensure the transition actually happened.
  TEST_ASSERT_FALSE(transition);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_IDLE_DONE, present_event.id);
}

// Validate that a transition partway through an awaiting transition works in the case of an error.
void test_sequencer_fsm_interrupted_awaiting_error(void) {
  const Event state_event = { .id = CHAOS_EVENT_SEQUENCE_DRIVE };
  Event prev_event = { 0 };
  Event present_event = { 0 };

  bool transition = true;
  Event delayed_event = { .id = CHAOS_EVENT_RELAY_ERROR };

  TEST_ASSERT_OK(event_raise(state_event.id, 0));
  StatusCode seq_status = STATUS_CODE_OK;
  StatusCode event_status = STATUS_CODE_OK;
  while (seq_status != STATUS_CODE_RESOURCE_EXHAUSTED) {
    TEST_ASSERT_OK(seq_status);
    event_status = event_process(&present_event);
    if (event_status != STATUS_CODE_OK) {
      prv_handle_awaiting(&prev_event, &present_event);
    }
    // Cause a transition.
    if (transition && present_event.id == CHAOS_EVENT_RELAY_CLOSED) {
      TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&present_event));
      present_event.id = CHAOS_EVENT_SEQUENCE_IDLE;
      // Raise some garbage data to check that flushing is working.
      TEST_ASSERT_OK(event_raise(NUM_CHAOS_EVENTS_FSM, 99));
      TEST_ASSERT_OK(event_raise_priority(EVENT_PRIORITY_HIGH, present_event.id, 0));
      delayed_event.data = present_event.data;
      TEST_ASSERT_OK(soft_timer_start_millis(15, prv_raise_with_delay_cb, &delayed_event, NULL));
      transition = false;
    } else {
      if (present_event.id <= NUM_CHAOS_EVENTS_CAN && present_event.id != CHAOS_EVENT_RELAY_ERROR) {
        // Skip ignored events.
        continue;
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      prev_event = present_event;
      seq_status = sequencer_fsm_publish_next_event(&present_event);
    }
  }
  // Ensure the transition actually happened.
  TEST_ASSERT_FALSE(transition);
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_IDLE_DONE, present_event.id);
}

// Validate that emergency filters work.
void test_sequencer_fsm_interrupted_emergency_noawait(void) {
  const Event state_event = { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY };
  Event present_event = { 0 };

  Event delayed_event = { .id = CHAOS_EVENT_RELAY_OPENED };

  TEST_ASSERT_OK(event_raise(state_event.id, 0));
  StatusCode seq_status = STATUS_CODE_OK;
  while (seq_status != STATUS_CODE_RESOURCE_EXHAUSTED) {
    TEST_ASSERT_OK(seq_status);
    event_process(&present_event);
    if (present_event.id == CHAOS_EVENT_OPEN_RELAY) {
      TEST_ASSERT_OK(sequencer_fsm_publish_next_event(&present_event));
      // Emergency doesn't expect opened events.
      delayed_event.data = present_event.data;
      TEST_ASSERT_OK(soft_timer_start_millis(15, prv_raise_with_delay_cb, &delayed_event, NULL));
    } else {
      if (present_event.id <= NUM_CHAOS_EVENTS_CAN) {
        // Skip ignored events.
        continue;
      }
      // At this point |present_event| is considered to be |prev_event| as it has passed through all
      // the other FSMs in the event loop.
      seq_status = sequencer_fsm_publish_next_event(&present_event);
    }
  }
  // Ensure the transition actually happened.
  TEST_ASSERT_EQUAL(CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE, present_event.id);
}

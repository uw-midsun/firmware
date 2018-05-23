#include "sequencer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "chaos_events.h"
#include "event_queue.h"
#include "misc.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SEQUENCER_EVENT_A 0
#define TEST_SEQUENCER_EVENT_B 1
#define TEST_SEQUENCER_EVENT_C 2
static_assert(TEST_SEQUENCER_EVENT_A != CHAOS_EVENT_NO_OP, "CHAOS_EVENT_NO_OP ID reused in test.");
static_assert(TEST_SEQUENCER_EVENT_B != CHAOS_EVENT_NO_OP, "CHAOS_EVENT_NO_OP ID reused in test.");
static_assert(TEST_SEQUENCER_EVENT_C != CHAOS_EVENT_NO_OP, "CHAOS_EVENT_NO_OP ID reused in test.");

void setup_test(void) {
  event_queue_init();
}

void teardown_test(void) {}

void test_sequencer_simple(void) {
  const SequencerEventPair events[] = {
    { .raise = { .id = TEST_SEQUENCER_EVENT_A, .data = TEST_SEQUENCER_EVENT_B },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_B, .data = TEST_SEQUENCER_EVENT_C },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_C, .data = TEST_SEQUENCER_EVENT_A },
      .response = SEQUENCER_NO_RESPONSE }
  };

  SequencerStorage storage;
  TEST_ASSERT_OK(sequencer_init(&storage, events, SIZEOF_ARRAY(events)));

  Event raised_event;
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));

  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_C, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));

  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_C, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, sequencer_advance(&storage, &raised_event));
  TEST_ASSERT_TRUE(sequencer_complete(&storage));
}

void test_sequencer_complex_start(void) {
  const SequencerEventPair events[] = {
    { .raise = { .id = TEST_SEQUENCER_EVENT_A, .data = TEST_SEQUENCER_EVENT_B },
      .response = { .id = TEST_SEQUENCER_EVENT_C, .data = TEST_SEQUENCER_EVENT_C } },
    { .raise = { .id = TEST_SEQUENCER_EVENT_B, .data = TEST_SEQUENCER_EVENT_C },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_C, .data = TEST_SEQUENCER_EVENT_A },
      .response = SEQUENCER_NO_RESPONSE }
  };

  SequencerStorage storage;
  TEST_ASSERT_OK(sequencer_init(&storage, events, SIZEOF_ARRAY(events)));

  Event raised_event;
  Event null_event;
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&raised_event));
  raised_event.id = TEST_SEQUENCER_EVENT_C;
  raised_event.data = TEST_SEQUENCER_EVENT_C;
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));

  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_C, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));

  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_C, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, sequencer_advance(&storage, &raised_event));
  TEST_ASSERT_TRUE(sequencer_complete(&storage));
}

void test_sequencer_complex_end(void) {
  const SequencerEventPair events[] = {
    { .raise = { .id = TEST_SEQUENCER_EVENT_A, .data = TEST_SEQUENCER_EVENT_B },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_B, .data = TEST_SEQUENCER_EVENT_C },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_C, .data = TEST_SEQUENCER_EVENT_A },
      .response = { .id = TEST_SEQUENCER_EVENT_C, .data = TEST_SEQUENCER_EVENT_C } }
  };

  SequencerStorage storage;
  TEST_ASSERT_OK(sequencer_init(&storage, events, SIZEOF_ARRAY(events)));

  Event raised_event;
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));

  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_C, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));

  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_C, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_OK(sequencer_advance(&storage, &raised_event));
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&raised_event));
  raised_event.id = TEST_SEQUENCER_EVENT_C;
  raised_event.data = TEST_SEQUENCER_EVENT_C;
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, sequencer_advance(&storage, &raised_event));
  TEST_ASSERT_TRUE(sequencer_complete(&storage));
}

void test_sequencer_bad_args(void) {
  const SequencerEventPair events[] = {
    { .raise = { .id = TEST_SEQUENCER_EVENT_A, .data = TEST_SEQUENCER_EVENT_B },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_B, .data = TEST_SEQUENCER_EVENT_C },
      .response = SEQUENCER_NO_RESPONSE },
    { .raise = { .id = TEST_SEQUENCER_EVENT_C, .data = TEST_SEQUENCER_EVENT_A },
      .response = SEQUENCER_NO_RESPONSE }
  };
  SequencerStorage storage;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sequencer_init(&storage, events, 0));
  TEST_ASSERT_OK(sequencer_init(&storage, events, SIZEOF_ARRAY(events)));

  Event raised_event;
  TEST_ASSERT_OK(event_process(&raised_event));
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_A, raised_event.id);
  TEST_ASSERT_EQUAL(TEST_SEQUENCER_EVENT_B, raised_event.data);
  TEST_ASSERT_FALSE(sequencer_complete(&storage));
  raised_event.id = TEST_SEQUENCER_EVENT_C;
  raised_event.data = TEST_SEQUENCER_EVENT_C;
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, sequencer_advance(&storage, &raised_event));
}

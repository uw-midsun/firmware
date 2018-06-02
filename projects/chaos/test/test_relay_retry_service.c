#include "relay_retry_service.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "relay_id.h"
#include "test_helpers.h"
#include "unity.h"

static RelayRetryServiceStorage s_storage;

void setup_test(void) {
  event_queue_init();
  TEST_ASSERT_OK(relay_retry_service_init(&s_storage));
}

void teardown_test(void) {}

void test_retry_service(void) {
  // Obvious failure.
  Event e = { .id = CHAOS_EVENT_MAYBE_RETRY_RELAY, .data = NUM_RELAY_IDS };
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, relay_retry_service_update(&e));

  // Success input but relay error as no retry attempts are allowed.
  e.data = RELAY_ID_SOLAR_MASTER_FRONT;
  TEST_ASSERT_OK(relay_retry_service_update(&e));
  e.data = NUM_RELAY_IDS;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_RELAY_ERROR, e.id);
  TEST_ASSERT_EQUAL(RELAY_ID_SOLAR_MASTER_FRONT, e.data);

  // Give the default limit and check that it fails for all relays if exceeded.
  e.id = CHAOS_EVENT_SET_RELAY_RETRIES;
  e.data = RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS;
  TEST_ASSERT_OK(relay_retry_service_update(&e));
  for (uint8_t relay_id = 0; relay_id < NUM_RELAY_IDS; relay_id++) {
    for (uint8_t i = 0; i < RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS; i++) {
      e.id = CHAOS_EVENT_MAYBE_RETRY_RELAY;
      e.data = relay_id;
      TEST_ASSERT_OK(relay_retry_service_update(&e));
      e.data = NUM_RELAY_IDS;
      TEST_ASSERT_OK(event_process(&e));
      TEST_ASSERT_EQUAL(CHAOS_EVENT_RETRY_RELAY, e.id);
      TEST_ASSERT_EQUAL(relay_id, e.data);
    }
    e.id = CHAOS_EVENT_MAYBE_RETRY_RELAY;
    e.data = relay_id;
    TEST_ASSERT_OK(relay_retry_service_update(&e));
    e.data = NUM_RELAY_IDS;
    TEST_ASSERT_OK(event_process(&e));
    TEST_ASSERT_EQUAL(CHAOS_EVENT_RELAY_ERROR, e.id);
    TEST_ASSERT_EQUAL(relay_id, e.data);
  }

  // Test unlimited retries
  e.id = CHAOS_EVENT_SET_RELAY_RETRIES;
  e.data = RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS;
  TEST_ASSERT_OK(relay_retry_service_update(&e));
  for (uint16_t i = 0; i <= UINT8_MAX + 1; i++) {
    e.id = CHAOS_EVENT_MAYBE_RETRY_RELAY;
    e.data = RELAY_ID_SOLAR_MASTER_FRONT;
    TEST_ASSERT_OK(relay_retry_service_update(&e));
    e.data = NUM_RELAY_IDS;
    TEST_ASSERT_OK(event_process(&e));
    TEST_ASSERT_EQUAL(CHAOS_EVENT_RETRY_RELAY, e.id);
    TEST_ASSERT_EQUAL(RELAY_ID_SOLAR_MASTER_FRONT, e.data);
  }
}

void test_relay_fail_fast(void) {
  Event e = { .id = CHAOS_EVENT_SET_RELAY_RETRIES, .data = RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS };
  TEST_ASSERT_OK(relay_retry_service_update(&e));
  e.id = CHAOS_EVENT_MAYBE_RETRY_RELAY;
  e.data = RELAY_ID_SOLAR_MASTER_FRONT;
  TEST_ASSERT_OK(relay_retry_service_update(&e));
  e.data = NUM_RELAY_IDS;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_RETRY_RELAY, e.id);
  TEST_ASSERT_EQUAL(RELAY_ID_SOLAR_MASTER_FRONT, e.data);

  relay_retry_service_fail_fast();

  e.id = CHAOS_EVENT_MAYBE_RETRY_RELAY;
  e.data = RELAY_ID_SOLAR_MASTER_FRONT;
  TEST_ASSERT_OK(relay_retry_service_update(&e));
  e.data = NUM_RELAY_IDS;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(CHAOS_EVENT_RELAY_ERROR, e.id);
  TEST_ASSERT_EQUAL(RELAY_ID_SOLAR_MASTER_FRONT, e.data);
}

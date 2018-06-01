#include "relay_retry_service.h"

#include "relay_retry_service.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "relay_id.h"

#define RELAY_RETRY_SERVICE_BACKOFF_MS 10

static RelayRetryServiceStorage *s_storage;

StatusCode relay_retry_service_init(RelayRetryServiceStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_storage = storage;
  s_storage->max_retries = 0;
  for (RelayId i = 0; i < NUM_RELAY_IDS; i++) {
    s_storage->relays_curr_retries[i] = 0;
  }
  return STATUS_CODE_OK;
}

StatusCode relay_retry_service_update(const Event *e) {
  if (s_storage == NULL || e == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (e->id == CHAOS_EVENT_MAYBE_RETRY_RELAY) {
    if (e->data >= NUM_RELAY_IDS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
    if (s_storage->max_retries != RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS &&
        s_storage->relays_curr_retries[e->data] >= s_storage->max_retries) {
      return event_raise(CHAOS_EVENT_RELAY_ERROR, e->data);
    }
    if (s_storage->max_retries != RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS) {
      s_storage->relays_curr_retries[e->data]++;
    }
    return event_raise(CHAOS_EVENT_RETRY_RELAY, e->data);
  }

  if (e->id == CHAOS_EVENT_SET_RELAY_RETRIES) {
    if (e->data > RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
    s_storage->max_retries = e->data;
    for (RelayId i = 0; i < NUM_RELAY_IDS; i++) {
      s_storage->relays_curr_retries[i] = 0;
    }
  }

  return STATUS_CODE_OK;
}

void relay_retry_service_fail_fast(void) {
  s_storage->max_retries = 0;
}

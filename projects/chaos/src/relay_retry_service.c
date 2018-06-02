#include "relay_retry_service.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "relay_id.h"

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

  switch (e->id) {
    case CHAOS_EVENT_MAYBE_RETRY_RELAY:
      // We want to consider retrying the relay first validate the relay is valid.
      if (e->data >= NUM_RELAY_IDS) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      // In the case of limited retries handle the accounting of attempts or raise an error if
      // exceeded.
      if (s_storage->max_retries != RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS) {
        if (s_storage->relays_curr_retries[e->data] >= s_storage->max_retries) {
          return event_raise(CHAOS_EVENT_RELAY_ERROR, e->data);
        }
        s_storage->relays_curr_retries[e->data]++;
      }
      // In the unlimited case or if there are remaining retries raise a retry event.
      return event_raise(CHAOS_EVENT_RETRY_RELAY, e->data);
    case CHAOS_EVENT_SET_RELAY_RETRIES:
      // Allow setting the relay attempts to a value in range:
      // [0, RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS].
      // If the value is RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS then we will have unlimited retries.
      if (e->data > RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      // Update the storage and wipe out previous attempt
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

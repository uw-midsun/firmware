#include "relay_retry_service.h"

#include "chaos_events.h"
#include "event_queue.h"
#include "log.h"
#include "relay_id.h"
#include "soft_timer.h"

static RelayRetryServiceStorage *s_storage = NULL;

static void prv_retry_with_delay(SoftTimerId id, void *context) {
  for (RelayId i = 0; i < NUM_RELAY_IDS; ++i) {
    if (s_storage->relays_timer_id[i] == id) {
      event_raise(CHAOS_EVENT_RETRY_RELAY, i);
      s_storage->relays_timer_id[i] = SOFT_TIMER_INVALID_TIMER;
      break;
    }
  }
}

StatusCode relay_retry_service_init(RelayRetryServiceStorage *storage, uint32_t backoff_ms) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_storage = storage;
  s_storage->backoff_ms = backoff_ms;
  s_storage->max_retries = 0;
  for (RelayId i = 0; i < NUM_RELAY_IDS; i++) {
    s_storage->relays_curr_retries[i] = 0;
    s_storage->relays_timer_id[i] = SOFT_TIMER_INVALID_TIMER;
  }
  return STATUS_CODE_OK;
}

StatusCode relay_retry_service_update(const Event *e) {
  if (s_storage == NULL || e == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  switch (e->id) {
    case CHAOS_EVENT_MAYBE_RETRY_RELAY:
      // We want to consider retrying the relay. First validate the relay is valid.
      if (e->data >= NUM_RELAY_IDS) {
        return status_code(STATUS_CODE_INVALID_ARGS);
      }
      // In the case of limited retries we handle the accounting of attempts or raise an error if
      // exceeded.
      if (s_storage->max_retries != RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS) {
        if (s_storage->relays_curr_retries[e->data] >= s_storage->max_retries) {
          if (e->data == 1) {
            LOG_DEBUG("[WARNING]: supressed relay error %d\n", e->data);
            return STATUS_CODE_OK;
          }
          LOG_DEBUG("Relay error: %d\n", e->data);
          return event_raise(CHAOS_EVENT_RELAY_ERROR, e->data);
        }
        s_storage->relays_curr_retries[e->data]++;
      }
      LOG_DEBUG("Relay retry: %d\n", e->data);
      // In the unlimited case or if there are remaining retries we raise a retry event.
      return soft_timer_start_millis(s_storage->backoff_ms, prv_retry_with_delay, NULL,
                                     &s_storage->relays_timer_id[e->data]);
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
        if (s_storage->relays_timer_id[i] != SOFT_TIMER_INVALID_TIMER) {
          soft_timer_cancel(s_storage->relays_timer_id[i]);
          s_storage->relays_timer_id[i] = SOFT_TIMER_INVALID_TIMER;
        }
      }
  }
  return STATUS_CODE_OK;
}

void relay_retry_service_fail_fast(void) {
  if (s_storage != NULL) {
    s_storage->max_retries = 0;
    for (RelayId i = 0; i < NUM_RELAY_IDS; ++i) {
      if (s_storage->relays_timer_id[i] != SOFT_TIMER_INVALID_TIMER) {
        soft_timer_cancel(s_storage->relays_timer_id[i]);
        s_storage->relays_timer_id[i] = SOFT_TIMER_INVALID_TIMER;
      }
    }
  }
}

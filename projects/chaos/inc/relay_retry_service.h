#pragma once
// Module to handle relay retrying
//
// Requires event_queue and soft_timers to be initialized. Intended to be used with relays.
//
// This module subscribes to events from the main event loop that reset and update the number of
// retries and also send relay retry requests or determines if a relay has faulted.
#include <stdint.h>

#include "event_queue.h"
#include "relay_id.h"
#include "soft_timer.h"

#define RELAY_RETRY_SERVICE_BACKOFF_MS 500
#define RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS 3
#define RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS + 1

typedef struct RelayRetryServiceStorage {
  uint32_t backoff_ms;
  uint8_t relays_curr_retries[NUM_RELAY_IDS];
  SoftTimerId relays_timer_id[NUM_RELAY_IDS];
  uint8_t max_retries;
} RelayRetryServiceStorage;

// Initializes the relay retry service.
StatusCode relay_retry_service_init(RelayRetryServiceStorage *storage, uint32_t backoff_ms);

// Updates the relay retry service based on the input event.
//
// Handled events:
// - ID: CHAOS_EVENT_MAYBE_RETRY_RELAY
//   Data: A valid RelayId
// - ID: CHAOS_EVENT_SET_RELAY_RETRIES
//   Data: Oneof [0, RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS] or RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS
// Will just respond STATUS_CODE_OK for all other events.
StatusCode relay_retry_service_update(const Event *e);

// Changes the maximum allowed retries for a relay to 0 causing it to fail immediately.
// Can be recovered from by sending a CHAOS_EVENT_SET_RELAY_RETRIES event which resets all the
// relays and updates the number of retry attempts.
void relay_retry_service_fail_fast(void);

#pragma once
// Module to handle relay retrying
//
// Requires event_queue to be initialized. Intended to be used with relays.
//
// This module subscribes to events from the main event loop that reset and update the number of
// retries and also send relay retry requests or determines if a relay has faulted.
#include <stdint.h>

#include "event_queue.h"
#include "relay_id.h"

#define RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS 3
#define RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS + 1

typedef struct RelayRetryServiceStorage {
  uint8_t relays_curr_retries[NUM_RELAY_IDS];
  uint8_t max_retries;
} RelayRetryServiceStorage;

// Initializes the relay retry service.
StatusCode relay_retry_service_init(RelayRetryServiceStorage *storage);

// Updates the relay retry service based on the input event.
StatusCode relay_retry_service_update(const Event *e);

// Changes the maximum allowed retries for a relay to 0 causing it to fail immediately.
void relay_retry_service_fail_fast(void);

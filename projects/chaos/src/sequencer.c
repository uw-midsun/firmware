#include "sequencer.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "event_queue.h"
#include "log.h"
#include "status.h"

static const Event s_sequencer_no_op_event = SEQUENCER_NO_RESPONSE;

StatusCode sequencer_init(SequencerStorage *storage, const SequencerEventPair *event_array,
                          size_t size) {
  if (!size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(SequencerStorage));
  storage->events_it = event_array;
  storage->events_end = storage->events_it + size;
  storage->stop_after_response = false;
  if (memcmp(&storage->events_it->response, &s_sequencer_no_op_event, sizeof(Event)) != 0) {
    storage->awaiting_response = true;
    storage->started = true;
  }
  return event_raise(storage->events_it->raise.id, storage->events_it->raise.data);
}

StatusCode sequencer_advance(SequencerStorage *storage, const Event *last_event) {
  if (sequencer_complete(storage)) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  if (storage->awaiting_response) {
    // Check if |last_event| is the same as the previous |response| event.
    if (memcmp(last_event, &storage->events_it->response, sizeof(Event)) != 0) {
      // Return |STATUS_CODE_OK| as we are waiting for the next event.
      return STATUS_CODE_OK;
    }
    storage->awaiting_response = false;
    // Don't continue if stopped on response. Must be awaiting in order to be stopped.
    if (storage->stop_after_response) {
      return STATUS_CODE_OK;
    }
  } else {
    // Check if |last_event| is the same as the previous |raise| event.
    if (memcmp(last_event, &storage->events_it->raise, sizeof(Event)) != 0) {
      // Allow old events to flush if we haven't seen the first event from the current sequence.
      if (!storage->started) {
        return STATUS_CODE_OK;
      }
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    // Check if |response| is empty (|s_sequencer_no_op_event|).
    if (storage->events_it->response.id != s_sequencer_no_op_event.id) {
      storage->awaiting_response = true;
      return STATUS_CODE_OK;
    }
  }

  // Increment the iterator.
  const SequencerEventPair *addr = storage->events_it;
  storage->events_it++;

  storage->started = true;
  if (sequencer_complete(storage)) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }
  return event_raise(storage->events_it->raise.id, storage->events_it->raise.data);
}

bool sequencer_complete(const SequencerStorage *storage) {
  return storage->events_it == storage->events_end;
}

bool sequencer_stop_awaiting(SequencerStorage *storage) {
  if (storage->awaiting_response) {
    storage->stop_after_response = true;
    return true;
  }
  return false;
}

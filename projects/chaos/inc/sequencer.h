#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "chaos_events.h"
#include "event_queue.h"
#include "status.h"

// Sequencer module for sequencing events.
//
// Expects event_queue to be initialized.
//
// A Sequencer is initialized with an array of SequencerEventPairs containing |raise|, |response|
// pairs. |sequencer_init()| acts by raising the first |raise| event in the array and henceforth
// |sequencer_advance()| is to be called with the previously raised event until the array is
// consumed. If a |response| is event not |SEQUENCER_NO_RESPONSE| then once the previously raised
// event is received it awaits |response|.

#define SEQUENCER_EMPTY_DATA 0
#define SEQUENCER_NO_RESPONSE \
  { .id = CHAOS_EVENT_NO_OP, .data = SEQUENCER_EMPTY_DATA }
#define SEQUENCER_MAX_FILTERS 5

typedef struct SequencerEventPair {
  Event raise;
  Event response;
} SequencerEventPair;

typedef struct SequencerStorage {
  const SequencerEventPair *events_it;
  const SequencerEventPair *events_end;
  bool awaiting_response;
  bool started;
  bool stop_after_response;
} SequencerStorage;

// Initializes |storage| with the |event_array| of |size|.
StatusCode sequencer_init(SequencerStorage *storage, const SequencerEventPair *event_array,
                          size_t size);

// Advances the sequence in |storage| if |last_event| matches the previous |raise| or the awaited
// |response|.
StatusCode sequencer_advance(SequencerStorage *storage, const Event *last_event);

// Returns true if the sequence in |storage| is finished.
bool sequencer_complete(const SequencerStorage *storage);

// Stops a sequencer from raising anything else if it is awaiting an event. Returns true if it was
// awaiting and was stopped.
bool sequencer_stop_awaiting(SequencerStorage *storage);

#pragma once
// Requires that event_queue is initialized.

#include "event_queue.h"

void sequencer_init(void);

// Publishes the next event in the sequence and transitions to a new sequence if needed. Errors if
// provided event was unexpected and will attempt to restart the current sequence.
StatusCode sequencer_publish_next_event(const Event *previous_event);

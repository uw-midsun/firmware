#pragma once
// Global event queue
//
// Uses a minimum priority queue to prioritize events of lower ID. Only one
// global instance exists.
#include <stdint.h>

#include "objpool.h"
#include "status.h"

#define EVENT_QUEUE_SIZE 20
typedef uint16_t EventID;

typedef struct Event {
  EventID id;
  uint16_t data;
} Event;

// Initializes the event queue.
void event_queue_init(void);

// Raises an event in the global event queue.
StatusCode event_raise(EventID id, uint16_t data);

// Returns the next event to be processed.
// Note that events are processed by priority.
StatusCode event_process(Event *e);

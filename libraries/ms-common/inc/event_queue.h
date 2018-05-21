#pragma once
// Global event queue
//
// Uses an array of FIFOs to prioritize events of lower priority. Only one global instance exists.
#include <stdint.h>

#include "objpool.h"
#include "status.h"

#define EVENT_QUEUE_SIZE 20
typedef uint16_t EventID;

typedef enum {
  EVENT_PRIORITY_HIGHEST = 0,
  EVENT_PRIORITY_HIGH,
  EVENT_PRIORITY_NORMAL,
  EVENT_PRIORITY_LOW,
  EVENT_PRIORITY_LOWEST,
  NUM_EVENT_PRIORITIES,
} EventPriority;

typedef struct Event {
  EventID id;
  uint16_t data;
} Event;

// Initializes the event queue.
void event_queue_init(void);

// Raises an event in the global event queue at the default priority.
StatusCode event_raise_priority(EventPriority priority, EventID id, uint16_t data);

// Raises an event in the global event queue at the default priority. Provided for legacy
// compatibility.
#define event_raise(id, data) event_raise_priority(EVENT_PRIORITY_NORMAL, (id), (data))

// Returns the next event to be processed.
// Note that events are processed by priority.
StatusCode event_process(Event *e);

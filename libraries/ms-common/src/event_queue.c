// This is just a wrapper for a backed priority queue.
// Currently, there is only one global event queue.
#include <stdbool.h>
#include <string.h>

#include "event_queue.h"
#include "pqueue_backed.h"

typedef struct EventQueue {
  PQueueBacked pqueue;
  PQueueNode queue_nodes[EVENT_QUEUE_SIZE + 1];
  Event event_nodes[EVENT_QUEUE_SIZE];
} EventQueue;

static EventQueue s_queue;

void event_queue_init(void) {
  pqueue_backed_init(&s_queue.pqueue, s_queue.queue_nodes, s_queue.event_nodes);
}

StatusCode event_raise(const Event *e) {
  return pqueue_backed_push(&s_queue, e, e->id);
}

StatusCode event_process(Event *e) {
  return pqueue_backed_pop(&s_queue, e);
}

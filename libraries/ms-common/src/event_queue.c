// Event queue uses a priority heap and object pool under the hood.
// Currently, there is only one global event queue.
#include <stdbool.h>
#include <string.h>

#include "event_queue.h"
#include "objpool.h"
#include "pqueue.h"
#include "status.h"

typedef struct EventQueue {
  ObjectPool pool;
  PQueue queue;
  PQueueNode queue_nodes[EVENT_QUEUE_SIZE + 1];
  Event event_nodes[EVENT_QUEUE_SIZE];
} EventQueue;

static EventQueue queue;

static void prv_init_node(void *node) {
  Event *e = node;
  memset(e, 0xA5, sizeof(*e));
}

void event_queue_init(void) {
  pqueue_init(&queue.queue, queue.queue_nodes, SIZEOF_ARRAY(queue.queue_nodes));
  objpool_init(&queue.pool, queue.event_nodes, prv_init_node);
}

StatusCode event_raise(const Event *e) {
  Event *node = objpool_get_node(&queue.pool);
  if (node == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *node = *e;
  pqueue_push(&queue.queue, node, node->id);

  return STATUS_CODE_OK;
}

StatusCode event_process(Event *e) {
  Event *node = pqueue_pop(&queue.queue);
  if (node == NULL) {
    return status_code(STATUS_CODE_EMPTY);
  }

  *e = *node;
  objpool_free_node(&queue.pool, node);

  return STATUS_CODE_OK;
}

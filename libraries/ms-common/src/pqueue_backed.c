#include "pqueue_backed.h"

static void prv_init_node(void *node, void *context) {
  PQueueBacked *queue = context;
  memset(node, 0, queue->elem_size);
}

bool pqueue_backed_init_impl(PQueueBacked *queue, PQueueNode *nodes, void *elems,
                             size_t num_nodes, size_t num_elems, size_t elem_size) {
  if (num_nodes != num_elems + 1) {
    return false;
  }

  queue->elem_size = elem_size;

  pqueue_init(&queue->queue, nodes, num_nodes);
  objpool_init_verbose(&queue->pool, elems, num_elems, elem_size, prv_init_node, queue);

  return true;
}

bool pqueue_backed_push(PQueueBacked *queue, void *elem, uint16_t prio) {
  void *node = objpool_get_node(&queue->pool);
  if (node == NULL) {
    return false;
  }

  memcpy(node, elem, queue->elem_size);
  pqueue_push(&queue->queue, node, prio);

  return true;
}

bool pqueue_backed_pop(PQueueBacked *queue, void *elem) {
  void *node = pqueue_pop(&queue->queue);
  if (node == NULL) {
    return false;
  }

  memcpy(elem, node, queue->elem_size);
  objpool_free_node(&queue->pool, node);

  return true;
}

size_t pqueue_backed_size(PQueueBacked *queue) {
  return pqueue_size(&queue->queue);
}

// The object pool uses a flag and index to verify a given node and determine its state.
// We currently use a bitfield to maximize the index limit.
#include <stdbool.h>
#include <string.h>

#include "objpool.h"
#include "status.h"

#define OBJPOOL_GET(pool, index) \
  ((ObjectMarker *)((uintptr_t)(pool)->nodes + ((index) * (pool)->node_size)))

void objpool_init_verbose(ObjectPool *pool, void *nodes, size_t num_nodes, size_t node_size,
                          objpool_node_init_fn init_node, void *context) {
  memset(pool, 0, sizeof(*pool));

  pool->nodes = nodes;
  pool->context = context;
  pool->num_nodes = num_nodes;
  pool->node_size = node_size;
  pool->init_node = init_node;

  for (size_t i = 0; i < num_nodes; i++) {
    ObjectMarker *node = OBJPOOL_GET(pool, i);
    node->index = i;

    objpool_free_node(pool, node);
  }
}

void *objpool_get_node(ObjectPool *pool) {
  for (size_t i = 0; i < pool->num_nodes; i++) {
    ObjectMarker *node = OBJPOOL_GET(pool, i);
    if (node->free) {
      node->free = false;
      return node;
    }
  }

  return NULL;
}

StatusCode objpool_free_node(ObjectPool *pool, void *node) {
  ObjectMarker *marker = node;

  if (marker == NULL || marker->index >= pool->num_nodes ||
      marker != OBJPOOL_GET(pool, marker->index)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t index = marker->index;
  memset(node, 0, pool->node_size);
  if (pool->init_node != NULL) {
    pool->init_node(node, pool->context);
  }

  marker->index = index;
  marker->free = true;

  return STATUS_CODE_OK;
}

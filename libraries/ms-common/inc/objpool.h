#pragma once
// Object Pool Interface
//
// Manages a pre-allocated array of objects. We use this instead of a heap so we don't need to deal
// with memory fragmentation. Use designated initializers to prevent losing marker information.
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "misc.h"
#include "status.h"

// Function to initialize nodes with
typedef void (*objpool_node_init_fn)(void *node, void *context);

// All nodes compatible with object pools should begin with an object marker
typedef struct ObjectMarker {
  bool free:1;
  uint16_t index:15;
} ObjectMarker;

typedef struct ObjectPool {
  void *nodes;
  void *context;
  objpool_node_init_fn init_node;
  size_t num_nodes;
  size_t node_size;
} ObjectPool;

// Initializes an object pool given a local array (i.e. not a pointer)
#define objpool_init(pool, nodes, init_fn, context) \
  objpool_init_verbose((pool), (nodes), SIZEOF_ARRAY((nodes)), sizeof((nodes)[0]), \
                       (init_fn), (context))

// Initializes an object pool. The specified context is provided for node initialization.
void objpool_init_verbose(ObjectPool *pool, void *nodes, size_t num_nodes, size_t node_size,
                          objpool_node_init_fn init_node, void *context);

// Returns the pointer to an object from the pool.
void *objpool_get_node(ObjectPool *pool);

// Releases the specified node
StatusCode objpool_free_node(ObjectPool *pool, void *node);

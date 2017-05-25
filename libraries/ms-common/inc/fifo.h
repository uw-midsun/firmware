// Implements a FIFO using a ring buffer
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "status.h"

typedef struct {
  void *buffer;
  void *end;
  void *head;
  void *next;
  size_t num_elems;
  size_t max_elems;
  size_t elem_size;
} Fifo;

// Convenience macros - these can only be used if the element or array are preserved.
// i.e. The type must persist and arrays must keep their size information.
#define fifo_init(fifo, buffer) \
  fifo_init_impl((fifo), (buffer), sizeof((buffer)[0]), SIZEOF_ARRAY((buffer)))
#define fifo_push(fifo, source) \
  fifo_push_impl((fifo), (source), sizeof(*(source)))
#define fifo_pop(fifo, dest) \
  fifo_pop_impl((fifo), (dest), sizeof(*(dest)))
#define fifo_push_arr(fifo, source_arr, len) \
  fifo_push_arr_impl((fifo), (source_arr), sizeof((source_arr)[0]), (len))
#define fifo_pop_arr(fifo, dest_arr, len) \
  fifo_pop_arr_impl((fifo), (dest_arr), sizeof((dest_arr)[0]), (len))

StatusCode fifo_init_impl(Fifo *fifo, void *buffer, size_t elem_size, size_t num_elems);

size_t fifo_size(Fifo *fifo);

StatusCode fifo_push_impl(Fifo *fifo, void *source_elem, size_t elem_size);

StatusCode fifo_pop_impl(Fifo *fifo, void *dest_elem, size_t elem_size);

// Note that the array functions will only push or pop data on success.
StatusCode fifo_push_arr_impl(Fifo *fifo, void *source_arr, size_t elem_size, size_t num_elems);

StatusCode fifo_pop_arr_impl(Fifo *fifo, void *dest_arr, size_t elem_size, size_t num_elems);

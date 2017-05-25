// Implements a FIFO using a ring buffer
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "status.h"

typedef struct {
  void *buffer;
  void *head;
  void *tail;
  size_t num_elems;
  size_t max_elems;
  size_t elem_size;
} Fifo;

StatusCode fifo_init(Fifo *fifo, void *buffer, size_t num_elems, size_t elem_size);

size_t fifo_size(Fifo *fifo);

StatusCode fifo_push(Fifo *fifo, void *source_elem, size_t elem_size);

StatusCode fifo_pop(Fifo *fifo, void *dest_elem, size_t elem_size);

StatusCode fifo_push_in(Fifo *fifo, void *source, size_t elem_size, size_t num_elems);

StatusCode fifo_pop_out(Fifo *fifo, void *dest, size_t elem_size, size_t num_elems);

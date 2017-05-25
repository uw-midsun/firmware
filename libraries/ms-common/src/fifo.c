#include "fifo.h"
#include <stdio.h>

StatusCode fifo_init(Fifo *fifo, void *buffer, size_t num_elems, size_t elem_size) {
  memset(fifo, 0, sizeof(*fifo));
  memset(buffer, 0, num_elems * elem_size);

  fifo->buffer = buffer;
  fifo->head = buffer;
  fifo->tail = buffer;
  fifo->max_elems = num_elems;
  fifo->elem_size = elem_size;
}

size_t fifo_size(Fifo *fifo) {
  return fifo->num_elems;
}

StatusCode fifo_push(Fifo *fifo, void *source_elem, size_t elem_size) {
  // check if there's space
  if (fifo->num_elems == fifo->max_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memcpy(fifo->tail, source_elem, fifo->elem_size);

  fifo->tail += elem_size;
  if (fifo->tail >= fifo->buffer + fifo->elem_size * fifo->max_elems) {
    fifo->tail = fifo->buffer;
  }

  fifo->num_elems++;

  return STATUS_CODE_OK;
}

StatusCode fifo_pop(Fifo *fifo, void *dest_elem, size_t elem_size) {
  if (fifo->num_elems == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memcpy(dest_elem, fifo->head, fifo->elem_size);

  fifo->head += elem_size;
  if (fifo->head >= fifo->buffer + fifo->elem_size * fifo->max_elems) {
    fifo->head = fifo->buffer;
  }

  fifo->num_elems--;

  return STATUS_CODE_OK;
}

StatusCode fifo_push_in(Fifo *fifo, void *source, size_t elem_size, size_t num_elems);

StatusCode fifo_pop_out(Fifo *fifo, void *dest, size_t elem_size, size_t num_elems);

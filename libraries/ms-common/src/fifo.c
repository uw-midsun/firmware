#include "fifo.h"
#include <stdio.h>

StatusCode fifo_init(Fifo *fifo, void *buffer, size_t elem_size, size_t num_elems) {
  memset(fifo, 0, sizeof(*fifo));
  memset(buffer, 0, num_elems * elem_size);

  fifo->buffer = buffer;
  fifo->end = buffer + num_elems * elem_size;
  fifo->head = buffer;
  fifo->next = buffer;
  fifo->max_elems = num_elems;
  fifo->elem_size = elem_size;
}

size_t fifo_size(Fifo *fifo) {
  return fifo->num_elems;
}

StatusCode fifo_push(Fifo *fifo, void *source_elem, size_t elem_size) {
  if (fifo->num_elems == fifo->max_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memcpy(fifo->next, source_elem, fifo->elem_size);

  fifo->next += elem_size;
  if (fifo->next >= fifo->end) {
    fifo->next = fifo->buffer;
  }

  fifo->num_elems++;

  return STATUS_CODE_OK;
}

StatusCode fifo_pop(Fifo *fifo, void *dest_elem, size_t elem_size) {
  if (fifo->num_elems == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size && dest_elem != NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (dest_elem != NULL) {
    memcpy(dest_elem, fifo->head, fifo->elem_size);
  }

  memset(fifo->head, 0, fifo->elem_size);

  fifo->head += fifo->elem_size;
  if (fifo->head >= fifo->end) {
    fifo->head = fifo->buffer;
  }

  fifo->num_elems--;

  return STATUS_CODE_OK;
}

StatusCode fifo_push_arr(Fifo *fifo, void *source, size_t elem_size, size_t num_elems) {
  if (fifo->num_elems + num_elems > fifo->max_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  void *new_next = fifo->next + fifo->elem_size * num_elems;

  size_t wrap_bytes = 0;
  if (fifo->end < new_next) {
    wrap_bytes = new_next - fifo->end;
  }

  size_t nonwrap_bytes = fifo->elem_size * num_elems - wrap_bytes;
  memcpy(fifo->next, source, nonwrap_bytes);
  fifo->next += nonwrap_bytes;
  if (fifo->next >= fifo->end) {
    fifo->next = fifo->buffer;
  }

  if (wrap_bytes > 0) {
    memcpy(fifo->next, source + nonwrap_bytes, wrap_bytes);
    fifo->next += wrap_bytes;
  }

  fifo->num_elems += num_elems;

  return STATUS_CODE_OK;
}

StatusCode fifo_pop_arr(Fifo *fifo, void *dest, size_t elem_size, size_t num_elems) {
  if (fifo->num_elems < num_elems) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  } else if (fifo->elem_size != elem_size && dest != NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  void *new_head = fifo->head + fifo->elem_size * num_elems;

  size_t wrap_bytes = 0;
  if (fifo->end < new_head) {
    wrap_bytes = new_head - fifo->end;
  }

  size_t nonwrap_bytes = fifo->elem_size * num_elems - wrap_bytes;

  if (dest != NULL) {
    memcpy(dest, fifo->head, nonwrap_bytes);
  }
  memset(fifo->head, 0, nonwrap_bytes);

  fifo->head += nonwrap_bytes;
  if (fifo->head >= fifo->end) {
    fifo->head = fifo->buffer;
  }

  if (wrap_bytes > 0) {
    if (dest != NULL) {
      memcpy(dest + nonwrap_bytes, fifo->head, wrap_bytes);
    }
    memset(fifo->head, 0, wrap_bytes);
    fifo->head += wrap_bytes;
  }

  fifo->num_elems -= num_elems;

  return STATUS_CODE_OK;
}

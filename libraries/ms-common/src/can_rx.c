#include "can_rx.h"

static int prv_handler_comp(const void *a, const void *b) {
  const CANRxHandler *x = a;
  const CANRxHandler *y = b;

  return x->msg_id - y->msg_id;
}

StatusCode can_rx_init(CANRxHandlers *rx_handlers,
                       CANRxHandler *handler_storage, size_t num_handlers) {
  // TODO: error checking

  memset(rx_handlers, 0, sizeof(*rx_handlers));
  memset(handler_storage, 0, sizeof(*handler_storage) * num_handlers);

  rx_handlers->storage = handler_storage;
  rx_handlers->num_handlers = 0;
  rx_handlers->max_handlers = num_handlers;

  return STATUS_CODE_OK;
}

StatusCode can_rx_register_handler(CANRxHandlers *rx_handlers, CANMessageID msg_id,
                                   CANRxHandlerCb handler, void *context) {
  // TODO: error checking

  rx_handlers->storage[rx_handlers->num_handlers++] = (CANRxHandler) {
    .msg_id = msg_id,
    .callback = handler,
    .context = context
  };

  qsort(rx_handlers->storage, rx_handlers->num_handlers,
        sizeof(*rx_handlers->storage), prv_handler_comp);

  return STATUS_CODE_OK;
}

CANRxHandler *can_rx_get_handler(CANRxHandlers *rx_handlers, CANMessageID msg_id) {
  // TODO: error checking
  const CANRxHandler key = { .msg_id = msg_id };
  return bsearch(&key, rx_handlers->storage, rx_handlers->num_handlers,
                 sizeof(*rx_handlers->storage), prv_handler_comp);
}

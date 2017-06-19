#include "can_queue.h"

StatusCode can_queue_init(CANQueue *can_queue) {
  return pqueue_backed_init(&can_queue->pqueue, can_queue->queue_nodes, can_queue->msg_nodes);
}

StatusCode can_queue_push(CANQueue *can_queue, const CANMessage *msg) {
  return pqueue_backed_push(&can_queue->pqueue, msg, msg->msg_id);
}

StatusCode can_queue_pop(CANQueue *can_queue, CANMessage *msg) {
  return pqueue_backed_pop(&can_queue->pqueue, msg);
}

StatusCode can_queue_peek(CANQueue *can_queue, CANMessage *msg) {
  return pqueue_backed_peek(&can_queue->pqueue, msg);
}

size_t can_queue_size(CANQueue *can_queue) {
  return pqueue_backed_size(&can_queue->pqueue);
}

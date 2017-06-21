#pragma once
#include "pqueue_backed.h"
#include "can_msg.h"

#define CAN_QUEUE_SIZE 10

typedef struct CANQueue {
  PQueueBacked pqueue;
  PQueueNode queue_nodes[CAN_QUEUE_SIZE + 1];
  CANMessage msg_nodes[CAN_QUEUE_SIZE];
} CANQueue;

StatusCode can_queue_init(CANQueue *can_queue);

StatusCode can_queue_push(CANQueue *can_queue, const CANMessage *msg);

StatusCode can_queue_pop(CANQueue *can_queue, CANMessage *msg);

StatusCode can_queue_peek(CANQueue *can_queue, CANMessage *msg);

size_t can_queue_size(CANQueue *can_queue);

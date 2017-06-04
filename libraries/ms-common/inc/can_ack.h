#pragma once
// CAN ACK handling
//
// ACK request objects are provided with critical messages.
// They define a callback that should be run when the ACK status is updated.
// The ACK status is updated when
// a) We receive an ACK over CAN
// b) The timer expires and we timeout the ACK
//
// If the ACK has timed out or we've received the expected number of ACKs, we remove the ACK request
// from the list. We use an object pool and an array of pointers to do so efficiently.
#include "can_msg.h"
#include "objpool.h"
#include "status.h"
#include "soft_timer.h"

// Arbitrary timeout - should calculate reasonable value
#define CAN_ACK_TIMEOUT_US 1000

#define CAN_ACK_MAX_REQUESTS 10

typedef enum {
  CAN_ACK_STATUS_OK = 0,
  CAN_ACK_STATUS_TIMEOUT,
  CAN_ACK_STATUS_INVALID, // Currently unused
  CAN_ACK_STATUS_UNKNOWN,
  NUM_ACK_STATUSES
} CANAckStatus;

// If the callback was fired due to timer expiry, the device ID is invalid.
// If the return code is non-OK, it is assumed that the received ACK is invalid and should be
// ignored. If this occurs on a timer expiry, we still remove the ACK request.
typedef StatusCode (*CANAckRequestCb)(CANMessageID msg_id, uint16_t device, CANAckStatus status,
                                      uint16_t num_remaining, void *context);

// TODO: we may want to keep track of which devices we've received from to prevent duplicates
typedef struct CANAckRequest {
  CANMessageID msg_id;
  uint16_t num_remaining;
  CANAckRequestCb callback;
  void *context;
  SoftTimerID timer;
  uint32_t response_bitset;
} CANAckRequest;

typedef struct CANAckRequests {
  ObjectPool pool;
  CANAckRequest request_nodes[CAN_ACK_MAX_REQUESTS];
  CANAckRequest *active_requests[CAN_ACK_MAX_REQUESTS];
  size_t num_requests;
} CANAckRequests;

StatusCode can_ack_init(CANAckRequests *requests);

StatusCode can_ack_add_request(CANAckRequests *requests, CANMessageID msg_id, uint16_t num_expected,
                               CANAckRequestCb callback, void *context);

// Handle a received ACK, firing the callback associated with the received message
StatusCode can_ack_handle_msg(CANAckRequests *requests, const CANId *can_id);

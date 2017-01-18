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
// from the list. We use an object pool and an array of pointers to do so efficiently. If necessary,
// we could use a BST, but for the purposes of keeping it simple, we'll just use qsort and bsearch.
//
// TODO: Will timers have context? How should we sort the list?
// If we're trying to process a received message, we search for the first request that matches the
// message ID. (Note that we order requests by time)
// If we're trying to process an expired ACK request, assume we have a void *context. This context
// should be the index (or pointer) of the element to be removed.
#include "can_msg.h"
#include "objpool.h"
#include "status.h"

// TODO: Replace
typedef uint16_t TimerID;

#define CAN_ACK_MAX_REQUESTS 10

typedef enum {
  CAN_ACK_STATUS_OK = 0,
  CAN_ACK_STATUS_TIMEOUT,
  CAN_ACK_STATUS_INVALID,
  CAN_ACK_STATUS_UNKNOWN,
  NUM_ACK_STATUSES
} CANAckStatus;

// If the callback was fired due to timer expiry, the device ID is invalid.
// If the return code is non-OK, it is assumed that the received ACK is invalid and should be
// ignored. If this occurs on a timer expiry, we still remove the ACK request.
typedef StatusCode (*CANAckRequestCb)(CANMessageID msg_id, uint16_t device, CANAckStatus response,
                                      uint16_t num_remaining, void *context);

// TODO: we may want to keep track of which devices we've received from to prevent duplicates
typedef struct CANAckRequest {
  CANMessageID msg_id;
  uint16_t num_remaining;
  CANAckRequestCb callback;
  void *context;
  TimerID timer;
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

// Removes the request with the same message ID, matching timer ID if non-zero
StatusCode can_ack_remove(CANAckRequests *requests, const CANAckRequest *ack_request);

// Handle a received ACK, firing the callback associated with the received message
StatusCode can_ack_handle_msg(CANAckRequests *requests, CANMessageID msg_id);

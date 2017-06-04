// Uses an object pool to track the storage for ack requests, but the actual requests are handled
// through an array of request pointers to minimize copying
// ACK requests currently ordered as they were created
#include "can_ack.h"
#include "log.h"

static StatusCode prv_update_req(CANAckRequests *requests, CANMessageID msg_id,
                                 SoftTimerID timer_id, CANAckStatus status, uint16_t device);

static void prv_timeout_cb(SoftTimerID timer_id, void *context);

StatusCode can_ack_init(CANAckRequests *requests) {
  // TODO: error checking
  memset(requests, 0, sizeof(*requests));

  objpool_init(&requests->pool, requests->request_nodes, NULL, NULL);
  requests->num_requests = 0;

  for (int i = 0; i < CAN_ACK_MAX_REQUESTS; i++) {
    requests->active_requests[i] = i;
  }
}

StatusCode can_ack_add_request(CANAckRequests *requests, CANMessageID msg_id, uint16_t num_expected,
                               CANAckRequestCb callback, void *context) {
  CANAckRequest *ack_request = objpool_get_node(&requests->pool);

  if (ack_request == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  memset(ack_request, 0, sizeof(*ack_request));
  ack_request->msg_id = msg_id;
  ack_request->num_remaining = num_expected;
  ack_request->callback = callback;
  ack_request->context = context;
  soft_timer_start(CAN_ACK_TIMEOUT_US, prv_timeout_cb, requests, &ack_request->timer);

  requests->active_requests[requests->num_requests++] = ack_request;

  return STATUS_CODE_OK;
}

StatusCode can_ack_handle_msg(CANAckRequests *requests, const CANId *can_id) {
  return prv_update_req(requests, can_id->msg_id, SOFT_TIMER_MAX_TIMERS,
                        CAN_ACK_STATUS_OK, can_id->source_id);
}

static StatusCode prv_update_req(CANAckRequests *requests, CANMessageID msg_id,
                                 SoftTimerID timer_id, CANAckStatus status, uint16_t device) {
  // TODO: error checking

  CANAckRequest *found_request = NULL;
  size_t index = 0;

  // Requests should be in the order that they were made, and there's a higher
  // chance that requests made first will be serviced first. In the case where we're
  // searching for a message ID, we'd like to pick the ACK request closest to expiry,
  // which should be the first one we encounter because we keep them in the order they were made.

  // Essentially checks if an ACK has been received from the device already and
  // * The message ID matches given an invalid timer
  // * The timer ID matches given an invalid message ID
  // * Both message and timer match given valid values for both
  for (index = 0; index < requests->num_requests; index++) {
    const CANAckRequest *req = requests->active_requests[index];
    if (((req->msg_id == msg_id && timer_id == SOFT_TIMER_MAX_TIMERS) ||
         (req->timer == timer_id && msg_id == CAN_MSG_INVALID_ID) ||
         (req->msg_id == msg_id && req->timer == timer_id)) &&
        (device == CAN_MSG_INVALID_DEVICE || (req->response_bitset & (1 << device)) == 0)) {
      found_request = req;
      break;
    }
  }

  if (found_request == NULL) {
    return status_code(STATUS_CODE_UNKNOWN);
  }

  // We use a bitset to keep track of which devices we've received an ACK for this message from
  found_request->num_remaining--;
  found_request->response_bitset |= (1 << device);

  if (found_request->callback != NULL) {
    StatusCode ret = STATUS_CODE_OK;
    ret = found_request->callback(found_request->msg_id, device, status,
                                  found_request->num_remaining, found_request->context);

    if (ret != STATUS_CODE_OK) {
      found_request->num_remaining++;
    }
  }

  if (found_request->num_remaining == 0 || status == CAN_ACK_STATUS_TIMEOUT) {
    StatusCode ret = objpool_free_node(&requests->pool, found_request);
    status_ok_or_return(ret);

    requests->num_requests--;
    if (index != requests->num_requests) {
      // Shift all requests to the left by 1
      memmove(&requests->active_requests[index], &requests->active_requests[index + 1],
              sizeof(requests->active_requests[0]) * (requests->num_requests - index));
    }

    requests->active_requests[requests->num_requests] = NULL;
  }

  return STATUS_CODE_OK;
}

static void prv_timeout_cb(SoftTimerID timer_id, void *context) {
  CANAckRequests *requests = context;

  prv_update_req(requests, CAN_MSG_INVALID_ID, timer_id,
                 CAN_ACK_STATUS_TIMEOUT, CAN_MSG_INVALID_DEVICE);
}

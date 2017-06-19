// Uses an object pool to track the storage for ack requests, but the actual requests are handled
// through an array of request pointers to minimize copying
// ACK requests currently ordered as they were created
#include "can_ack.h"

static StatusCode prv_update_req(CANAckRequests *requests, CANMessageID msg_id,
                                 SoftTimerID timer_id, CANAckStatus status, uint16_t device);

static void prv_timeout_cb(SoftTimerID timer_id, void *context);

StatusCode can_ack_init(CANAckRequests *requests) {
  memset(requests, 0, sizeof(*requests));

  objpool_init(&requests->pool, requests->request_nodes, NULL, NULL);
  requests->num_requests = 0;

  return STATUS_CODE_OK;
}

StatusCode can_ack_add_request(CANAckRequests *requests, CANMessageID msg_id,
                               const CANAckRequest *ack_request) {
  CANAckPendingReq *pending_ack = objpool_get_node(&requests->pool);

  if (pending_ack == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  memset(pending_ack, 0, sizeof(*pending_ack));
  pending_ack->msg_id = msg_id;
  pending_ack->num_remaining = ack_request->num_expected;
  pending_ack->callback = ack_request->callback;
  pending_ack->context = ack_request->context;
  StatusCode ret = soft_timer_start_millis(CAN_ACK_TIMEOUT_MS, prv_timeout_cb,
                                           requests, &pending_ack->timer);

  if (ret != STATUS_CODE_OK) {
    objpool_free_node(&requests->pool, pending_ack);
    return ret;
  }

  requests->active_requests[requests->num_requests++] = pending_ack;

  return STATUS_CODE_OK;
}

StatusCode can_ack_handle_msg(CANAckRequests *requests, const CANMessage *msg) {
  return prv_update_req(requests, msg->msg_id, SOFT_TIMER_INVALID_TIMER,
                        msg->data, msg->source_id);
}

static StatusCode prv_update_req(CANAckRequests *requests, CANMessageID msg_id,
                                 SoftTimerID timer_id, CANAckStatus status, uint16_t device) {
  CANAckPendingReq *found_request = NULL;
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
    const CANAckPendingReq *req = requests->active_requests[index];
    if (((req->msg_id == msg_id && timer_id == SOFT_TIMER_INVALID_TIMER) ||
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
    StatusCode ret = found_request->callback(found_request->msg_id, device, status,
                                             found_request->num_remaining, found_request->context);
    if (ret != STATUS_CODE_OK) {
      found_request->num_remaining++;
    }
  }

  if (found_request->num_remaining == 0 || status == CAN_ACK_STATUS_TIMEOUT) {
    soft_timer_cancel(found_request->timer);
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

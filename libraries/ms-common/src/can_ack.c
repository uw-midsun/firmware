// Uses an object pool to track the storage for ack requests, but the actual requests are handled
// through an array of request pointers to minimize copying
#include "can_ack.h"

static int prv_request_comp(const void *a, const void *b) {
  const CANAckRequest **req_a = a;
  const CANAckRequest **req_b = b;

  // If timer ID is 0, we're searching by ID
  if ((*req_a)->msg_id == (*req_b)->msg_id && (*req_a)->timer != 0) {
    // TODO: replace to sort by time until expiry
    return (*req_a)->timer - (*req_b)->timer;
  }

  return (*req_a)->msg_id - (*req_b)->msg_id;
}

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

  // TODO: REPLACE
  static uint8_t s_timer_id = 1;

  memset(ack_request, 0, sizeof(*ack_request));
  ack_request->msg_id = msg_id;
  ack_request->num_remaining = num_expected;
  ack_request->callback = callback;
  ack_request->context = context;
  ack_request->timer = s_timer_id++;

  requests->active_requests[requests->num_requests++] = ack_request;

  return STATUS_CODE_OK;
}

StatusCode can_ack_expire(CANAckRequests *requests, const CANAckRequest *ack_request) {
  // TODO: error checking

  CANAckRequest *found_request = NULL;
  size_t index = 0;
  for (index = 0; index < requests->num_requests; index++) {
    const CANAckRequest *req = requests->active_requests[index];
    if (req->msg_id == ack_request->msg_id &&
        (ack_request->timer == 0 || req->timer == ack_request->timer)) {
      found_request = req;
      break;
    }
  }

  if (found_request == NULL) {
    return status_code(STATUS_CODE_UNKNOWN);
  }

  if (found_request->callback != NULL) {
    found_request->callback(found_request->msg_id, 0, CAN_ACK_STATUS_TIMEOUT,
                            found_request->num_remaining, found_request->context);
  }

  StatusCode ret = objpool_free_node(&requests->pool, found_request);
  status_ok_or_return(ret);

  requests->num_requests--;
  if (index != requests->num_requests) {
    memmove(&requests->active_requests[index], &requests->active_requests[index + 1],
            sizeof(requests->active_requests[0]) * (requests->num_requests - index));
  }

  requests->active_requests[requests->num_requests] = NULL;

  return STATUS_CODE_OK;
}

StatusCode can_ack_handle_msg(CANAckRequests *requests, CANMessageID msg_id) {
  CANAckRequest ack_request = {
    .msg_id = msg_id
  };
  CANAckRequest *req_ptr = &ack_request;

  CANAckRequest **elem = bsearch(&req_ptr, requests->active_requests, requests->num_requests,
                                 sizeof(requests->active_requests[0]), prv_request_comp);
  if (elem == NULL) {
    return status_code(STATUS_CODE_UNKNOWN);
  }

  return STATUS_CODE_OK;
}

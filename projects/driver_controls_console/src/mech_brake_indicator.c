#include "mech_brake_indicator.h"
#include "can_msg_defs.h"
#include "can_unpack.h"

static StatusCode prv_handle_mech_brake(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  uint64_t data = 0;
  CAN_UNPACK_MECH_BRAKE(msg, &data);  // implement this
}

StatusCode mech_brake_indicator_init(void) {
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_MECH_BRAKE, prv_handle_mech_brake, NULL);
  return STATUC_CODE_OK;
}

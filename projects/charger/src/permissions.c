#include "permissions.h"

#include <stddef.h>

#include "can_interval.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "can_unpack.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "status.h"

#define PERMISSIONS_PERIOD 30000000  // 30s

static CanInterval *s_interval;
static GenericCanMsg s_msg;

// GenericCanRx
static void prv_permissions_rx(const GenericCanMsg *msg, const GenericCanMsg *msg) {
  CANMessage can_msg = { 0 };
  generic_can_msg_to_can_message(msg, &can_msg);
  uint8_t allowed = 0;
  CAN_UNPACK_CHARGING_PERMISSION(can_msg, &allowed);
  if (allowed) {
    // TODO(ELEC-355):
    // Raise permitted.
  } else {
    // TODO(ELEC-355):
    // Raise not-permitted.
  }
}

StatusCode permissions_init(GenericCan *can) {
  CANMessage msg = { 0 };
  CAN_PACK_CHARGING_REQ(&msg);
  can_message_to_generic_can_message(&msg, &s_msg);

  status_ok_or_return(can_interval_factory(can, &s_msg, PERMISSIONS_PERIOD, s_interval));
  return generic_can_register_rx(can, prv_permissions_rx, SYSTEM_CAN_MESSAGE_CHARGING_PERMISSION,
                                 NULL);
}

void permissions_request(void) {
  can_interval_enable(s_interval);
  generic_can_enable_rx(s_interval->can, SYSTEM_CAN_MESSAGE_CHARGING_PERMISSION);
}

void permissions_cease_request(void) {
  can_interval_disable(s_interval);
  generic_can_disable_rx(s_interval->can, SYSTEM_CAN_MESSAGE_CHARGING_PERMISSION);
}

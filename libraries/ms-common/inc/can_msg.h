#pragma once
// Defines the CAN message type
// This is kept in a separate file to prevent cyclic dependencies
#include <stdint.h>

#define CAN_MSG_INVALID_ID (UINT16_MAX)
#define CAN_MSG_INVALID_DEVICE (UINT16_MAX)

#define CAN_MSG_MAX_DEVICES (1 << 4)

#define CAN_MSG_SET_RAW_ID(can_msg, can_id) \
do { \
  CANId id = { .raw = (can_id) }; \
  (can_msg)->source_id = id.source_id; \
  (can_msg)->msg_id = id.msg_id; \
  (can_msg)->type = id.type; \
} while(0)

typedef enum {
  CAN_MSG_TYPE_DATA = 0,
  CAN_MSG_TYPE_ACK,
  CAN_MSG_NUM_TYPES
} CANMsgType;

typedef uint16_t CANMessageID;

typedef struct CANMessage {
  uint16_t source_id;
  CANMessageID msg_id;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
  CANMsgType type;
  uint8_t dlc;
} CANMessage;

typedef union CANId {
  uint16_t raw;
  struct {
    uint16_t source_id:4;
    uint16_t type:1;
    uint16_t msg_id:6;
  };
} CANId;

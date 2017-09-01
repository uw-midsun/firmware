#pragma once

#include <stdint.h>

#include "can_msg.h"

#define CAN_PACK_IMPL_EMPTY 0

// Packs eight u8s into a CAN Msg
void can_pack_impl_u8(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                      uint8_t f1, uint8_t f2, uint8_t f3, uint8_t f4, uint8_t f5, uint8_t f6,
                      uint8_t f7, uint8_t f8);

// Packs four u16s into a CAN Msg
void can_pack_impl_u16(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                       uint16_t f1, uint16_t f2, uint16_t f3, uint16_t f4);

// Packs a pair of u32 into a CAN Msg
void can_pack_impl_u32(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                       uint32_t f1, uint32_t f2);

// Packs a u64 into a CAN Msg
void can_pack_impl_u64(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                       uint64_t f1);

// Packs an empty CAN Msg
#define can_pack_impl_empty(msg_ptr, source_id, id, type) \
  can_pack_impl_u64((msg_ptr), (source_id), (id), (type), CAN_PACK_IMPL_EMPTY)

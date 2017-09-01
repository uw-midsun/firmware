#include "can_pack_impl.h"

#include <stdint.h>

#include "can_msg.h"

// Calculate the number of used bytes if the trailing bytes are 0 or unused then there is no point
// wasting bus time transmitting them.
#define CAN_PACK_CALC_DLC(data) 8 - (size_t)__builtin_ffsll((int64_t)(data)) / 8

void can_pack_impl_u8(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                      uint8_t f1, uint8_t f2, uint8_t f3, uint8_t f4, uint8_t f5, uint8_t f6,
                      uint8_t f7, uint8_t f8) {
  *msg = (CANMessage){
    .type = type,                                   //
    .source_id = source_id,                         //
    .msg_id = id,                                   //
    .data_u8 = { f1, f2, f3, f4, f5, f6, f7, f8 },  //
  };
  msg->dlc = CAN_PACK_CALC_DLC(msg->data);
}

void can_pack_impl_u16(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                       uint16_t f1, uint16_t f2, uint16_t f3, uint16_t f4) {
  *msg = (CANMessage){
    .type = type,                    //
    .source_id = source_id,          //
    .msg_id = id,                    //
    .data_u16 = { f1, f2, f3, f4 },  //
  };
  msg->dlc = CAN_PACK_CALC_DLC(msg->data);
}

void can_pack_impl_u32(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                       uint32_t f1, uint32_t f2) {
  *msg = (CANMessage){
    .type = type,            //
    .source_id = source_id,  //
    .msg_id = id,            //
    .data_u32 = { f1, f2 },  //
  };
  msg->dlc = CAN_PACK_CALC_DLC(msg->data);
}

void can_pack_impl_u64(CANMessage *msg, uint16_t source_id, CANMessageID id, CANMsgType type,
                       uint64_t f1) {
  *msg = (CANMessage){
    .type = type,            //
    .source_id = source_id,  //
    .msg_id = id,            //
    .data = f1,              //
  };
  msg->dlc = CAN_PACK_CALC_DLC(msg->data);
}

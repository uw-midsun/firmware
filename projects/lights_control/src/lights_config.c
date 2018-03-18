#include "lights_config.h"
#include "can_msg_defs.h"
#include "lights_can.h"

const CANSettings can_settings_front = { .bitrate = CAN_HW_BITRATE_125KBPS,
                                         .rx_event = LIGHTS_EVENT_CAN_RX,
                                         .tx_event = LIGHTS_EVENT_CAN_TX,
                                         .fault_event = LIGHTS_EVENT_CAN_FAULT,
                                         .tx = LIGHTS_CAN_TX_ADDR,
                                         .rx = LIGHTS_CAN_RX_ADDR,
                                         .device_id = SYSTEM_CAN_DEVICE_LIGHTS_FRONT,
                                         .loopback = false };

const CANSettings can_settings_rear = { .bitrate = CAN_HW_BITRATE_125KBPS,
                                        .rx_event = LIGHTS_EVENT_CAN_RX,
                                        .tx_event = LIGHTS_EVENT_CAN_TX,
                                        .fault_event = LIGHTS_EVENT_CAN_FAULT,
                                        .tx = LIGHTS_CAN_TX_ADDR,
                                        .rx = LIGHTS_CAN_RX_ADDR,
                                        .device_id = SYSTEM_CAN_DEVICE_LIGHTS_REAR,
                                        .loopback = false };

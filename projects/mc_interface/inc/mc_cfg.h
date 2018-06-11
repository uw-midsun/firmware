#pragma once

#define MC_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS
#define MC_CFG_NUM_CAN_RX_HANDLERS 5

#define MC_CFG_CAN_UART_PORT UART_PORT_3
#define MC_CFG_CAN_UART_TX \
  { .port = GPIO_PORT_B, .pin = 10 }
#define MC_CFG_CAN_UART_RX \
  { .port = GPIO_PORT_B, .pin = 11 }
#define MC_CFG_CAN_UART_ALTFN GPIO_ALTFN_4
#define MC_CFG_CAN_UART_BAUDRATE 115200

#define MC_CFG_RELAY_LEFT \
  { .port = GPIO_PORT_B, .pin = 3 }
#define MC_CFG_RELAY_RIGHT \
  { .port = GPIO_PORT_B, .pin = 9 }
#define MC_CFG_RELAY_DELAY_MS 100

#define MC_CFG_MOTOR_CAN_ID_DC_LEFT 0x01
#define MC_CFG_MOTOR_CAN_ID_DC_RIGHT 0x02
#define MC_CFG_MOTOR_CAN_ID_MC_LEFT 0x03
#define MC_CFG_MOTOR_CAN_ID_MC_RIGHT 0x04
#define MC_CFG_MOTOR_MAX_BUS_CURRENT 65.0f

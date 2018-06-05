#pragma once
#define PLUTUS_CFG_DEBUG

#include "adc_cfg.h"
#include "afe_cfg.h"
#include "can_msg_defs.h"

// Input fault conditions
// in 100uV units
#define PLUTUS_CFG_CELL_UNDERVOLTAGE 25000
#define PLUTUS_CFG_CELL_OVERVOLTAGE 42000
// in 0.1C units
#define PLUTUS_CFG_OVERTEMP_DISCHARGE 600
#define PLUTUS_CFG_OVERTEMP_CHARGE 450
// in mA
#define PLUTUS_CFG_OVERCURRENT_DISCHARGE 122400
#define PLUTUS_CFG_OVERCURRENT_CHARGE -130000

// Heartbeat settings
#define PLUTUS_CFG_HEARTBEAT_PERIOD_MS 500
// TODO: Add slave board
#define PLUTUS_CFG_HEARTBEAT_EXPECTED_DEVICES \
  CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CHAOS, SYSTEM_CAN_DEVICE_DRIVER_CONTROLS)

// Killswitch
#define PLUTUS_CFG_KILLSWITCH \
  { .port = GPIO_PORT_B, .pin = 1 }

// HV Relay - power/ground
#define PLUTUS_CFG_RELAY_DELAY_MS 1000
#define PLUTUS_CFG_RELAY_PWR \
  { .port = GPIO_PORT_B, .pin = 5 }
#define PLUTUS_CFG_RELAY_GND \
  { .port = GPIO_PORT_B, .pin = 4 }

// Fans
#define PLUTUS_CFG_FAN_1 \
  { .port = GPIO_PORT_A, .pin = 10 }
#define PLUTUS_CFG_FAN_2 \
  { .port = GPIO_PORT_A, .pin = 9 }
#define PLUTUS_CFG_FAN_PWM_TIM PWM_TIMER_1
// ~26kHz
#define PLUTUS_CFG_FAN_PWM_PERIOD_US 38

// CAN
#define PLUTUS_CFG_CAN_BITRATE CAN_HW_BITRATE_500KBPS
#define PLUTUS_CFG_NUM_CAN_RX_HANDLERS 5

// Master/Slave selector
#define PLUTUS_CFG_BOARD_TYPE_SEL \
  { .port = GPIO_PORT_B, .pin = 2 }
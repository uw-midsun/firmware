#pragma once
// Module for loading static settings regarding the charger.

#include "can.h"
#include "charger_controller.h"
#include "gpio.h"
#include "status.h"
#include "uart.h"

#define CHARGER_CFG_NUM_CAN_RX_HANDLERS 2

CANSettings *charger_cfg_load_can_settings(void);

UARTSettings *charger_cfg_load_uart_settings(void);

UARTPort charger_cfg_load_uart_port(void);

GPIOAddress charger_cfg_load_charger_pin(void);

// Required before |charger_cfg_load_settings|, requires UART and CAN to be running.
StatusCode charger_cfg_init_settings(void);

ChargerSettings *charger_cfg_load_settings(void);

#pragma once
// Module for loading static settings regarding the charger.
//
// Requires
// - Interrupts
// - Soft Timers
// - GPIO
// - GPIO Interrupts
// - Event Queue
// - CAN
// - UART
// to be initialized before calling charger_cfg_init_settings().

#include "can.h"
#include "charger_controller.h"
#include "gpio.h"
#include "status.h"
#include "uart.h"

// TODO(ELEC-355): Configure these.
#define CHARGER_CFG_SEND_PERIOD_S 30
#define CHARGER_CFG_WATCHDOG_PERIOD_S 60
#define CHARGER_CFG_NUM_CAN_RX_HANDLERS 3

// Returns a pointer to a static CANSettings object for Network Layer CAN.
CANSettings *charger_cfg_load_can_settings(void);

// Returns a pointer to a static UARTSettings object for UART CAN.
UARTSettings *charger_cfg_load_uart_settings(void);

// Returns a UART Port to configure UART CAN.
UARTPort charger_cfg_load_uart_port(void);

// TODO(ELEC-355): Correct this pin's usage to ADC or PWM based on what happens with the hardware.
// Returns the GpioAddress of the charger_pin.
GpioAddress charger_cfg_load_charger_pin(void);

// Required before |charger_cfg_load_settings|, requires UART and CAN to be running.
StatusCode charger_cfg_init_settings(void);

// Returns the settings for the charger controller.
ChargerSettings *charger_cfg_load_settings(void);

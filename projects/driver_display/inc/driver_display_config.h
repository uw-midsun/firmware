#pragma once
// Configuration module for the can_slave
// Defines settings for uart, can_hw and can_uart and loads them

#include "can.h"
#include "can_hw.h"
#include "can_uart.h"
#include "uart.h"

#define DRIVER_DISPLAY_CONFIG_UART_BAUDRATE 115200

// Load uart settings for uart initialization
UartSettings *driver_display_config_load_uart(void);

// Load can_hw settings for can_hw initialization
CanHwSettings *driver_display_config_load_can(void);

// Load can_uart settings for can_uart initialization
CanUart *driver_display_config_load_can_uart(void);

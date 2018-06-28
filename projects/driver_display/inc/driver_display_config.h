#pragma once
// Configuration module for the can_slave
// Defines settings for uart, can_hw and can_uart and loads them

#include "can.h"
#include "can_hw.h"
#include "can_uart.h"
#include "uart.h"

#define DRIVER_DISPLAY_CONFIG_UART_BAUDRATE 115200
#define DRIVER_DISPLAY_CONFIG_UART_PORT UART_PORT_3
#define DRIVER_DISPLAY_CONFIG_UART_TX \
  { .port = GPIO_PORT_B, .pin = 10 }
#define DRIVER_DISPLAY_CONFIG_UART_RX \
  { .port = GPIO_PORT_B, .pin = 11 }
#define DRIVER_DISPLAY_CONFIG_UART_ALTFN GPIO_ALTFN_4
#define DRIVER_DISPLAY_CONFIG_CAN_BITRATE CAN_HW_BITRATE_500KBPS

// Load uart settings for uart initialization
UARTSettings *driver_display_config_load_uart(void);

// Load can_hw settings for can_hw initialization
CANHwSettings *driver_display_config_load_can(void);

// Load can_uart settings for can_uart initialization
CanUart *driver_display_config_load_can_uart(void);

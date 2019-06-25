#pragma once

// UART 
#define TELEMETRY_GPS_UART_BAUD_RATE 9600

#define TELEMETRY_GPS_UART_PORT UART_PORT_3

#define TELEMETRY_GPS_UART_TX \
{ .port = GPIO_PORT_B, .pin = 10 }

#define TELEMETRY_GPS_UART_RX \
{ .port = GPIO_PORT_B, .pin = 11 }

#define TELEMETRY_GPS_UART_ALTFN GPIO_ALTFN_4

// CAN
#define GPS_CAN_BITRATE CAN_HW_BITRATE_500KBPS

// The pin numbers to use for providing power and turning the GPS on and off
#define TELEMETRY_GPS_ON_OFF_PIN \
{ .port = GPIO_PORT_B, .pin = 9}

// Uncomment to print raw NEMA messages 
#define GPS_DEBUG_PRINT_MESSAGES
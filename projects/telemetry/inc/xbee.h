#pragma once
// This will be the xbee driver. It can be initialized, and then used to send
// messages to the xbee. Basically it's a UART wrapper
#include <stdbool.h>
#include "status.h"
#include "uart.h"

// Initializes the xbee. Assumes it is already powered and good to transmit
StatusCode xbee_init();

// Sends a message to the xbee. Must be terminated with a newline or else the message will
// not send
StatusCode xbee_transmit(const uint8_t *message, size_t len);

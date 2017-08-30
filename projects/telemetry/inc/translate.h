#pragma once
#include "can.h"
#include "status.h"
#include "uart.h"

// Expects initialized CAN and UART
StatusCode translate_init(UARTPort uart);

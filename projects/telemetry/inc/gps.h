#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "status.h"
#include "uart.h"
#include "nmea.h"

// This enum contains the list of supported NMEA sentences that are supported by our GPS chip

StatusCode evm_gps_init(UARTSettings* settings);

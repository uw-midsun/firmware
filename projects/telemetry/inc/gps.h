#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "status.h"
#include "uart.h"
#include "nmea.h"

// This enum contains the list of supported NMEA sentences that are supported by our GPS chip

typedef void (*GPSHandler)(const NMEAResult);
typedef void (*GGAHandler)(const GGASentence);

StatusCode evm_gps_init(UARTSettings* settings);

// These methods will add the handler to the handler array, and returns the index so that it can
// be removed. If the array is full, then it returns -1.

// The reason for different handlers is so that it is easier to use the driver.
int32_t add_gps_handler(GPSHandler handler);
int32_t add_gga_handler(GGAHandler handler);
void remove_gps_handler(uint32_t index);
void remove_gga_handler(uint32_t index);

#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "nmea.h"
#include "status.h"
#include "uart.h"

// This enum contains the list of supported NMEA sentences that are supported by
// our GPS chip

typedef void (*GPSHandler)(const NMEAResult);
typedef void (*GGAHandler)(const GGASentence);

typedef struct {
  UARTPort *port;
  UARTSettings *uart_settings;
  GPIOSettings *settings_tx;
  GPIOSettings *settings_rx;
  GPIOSettings *settings_power;
  GPIOAddress *pin_rx;
  GPIOAddress *pin_tx;
  GPIOAddress *pin_power;
} EvmSettings;

StatusCode evm_gps_init(EvmSettings *settings);

// These methods will add the handler to the handler array, and returns the
// index so that it can
// be removed. If the array is full, then it returns -1.

// The reason for different handlers is so that it is easier to use the driver.
StatusCode add_gps_handler(GPSHandler handler, size_t *index);
StatusCode add_gga_handler(GGAHandler handler, size_t *index);
StatusCode remove_gps_handler(size_t index);
StatusCode remove_gga_handler(size_t index);

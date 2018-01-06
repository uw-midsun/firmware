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

// This struct basically contains all the info about pins etc.
// Check this document on page 4:
// https://www.linxtechnologies.com/wp/wp-content/uploads/evm-gps-f4.pdf
typedef struct {
  UARTPort *port;
  UARTSettings *uart_settings;    // The uart settings to be used
  GPIOSettings *settings_tx;      // Pin configurations
  GPIOSettings *settings_rx;      // Transmitting and receiving pins
  GPIOSettings *settings_power;   // Pin which provides power
  GPIOSettings *settings_on_off;  // Pin to power up the device
  GPIOAddress *pin_rx;            // Pin addresses
  GPIOAddress *pin_tx;
  GPIOAddress *pin_power;
  GPIOAddress *pin_on_off;
} EvmSettings;

StatusCode evm_gps_init(EvmSettings *settings);

// These methods will add the handler to the handler array, and returns the
// index so that it can
// be removed. If the array is full, then it returns -1.

// The reason for different handlers is so that it is easier to use the driver.
StatusCode add_gga_handler(GGAHandler handler, size_t *index);
StatusCode remove_gga_handler(size_t index);

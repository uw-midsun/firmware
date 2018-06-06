#pragma once
// This will be the GPS driver. It will be responsive for initializing, and cleaning up the GPS
// as well as providing data from the GPS.
#include <stdbool.h>
#include "status.h"
#include "uart.h"

// This struct basically contains all the info about pins etc.
// Check this document on page 4:
// https://www.linxtechnologies.com/wp/wp-content/uploads/evm-gps-f4.pdf
typedef struct {
  UARTPort port;
  UARTSettings *uart_settings;    // The uart settings to be used
  GPIOSettings *settings_power;   // Pin which provides power
  GPIOSettings *settings_on_off;  // Pin to trigger power up
  GPIOAddress *pin_rx;            // Pin addresses
  GPIOAddress *pin_tx;
  GPIOAddress *pin_power;
  GPIOAddress *pin_on_off;
} gps_settings;

// Initialized the GPS module
StatusCode gps_init(gps_settings *settings);

// Shuts the GPS module down
StatusCode gps_clean_up(gps_settings *settings);

// Sets gga_message to the last received GGA message
// Returns whether the GPS is active or not
bool gps_get_gga(char *gga_message);

// Sets vtg_message to the last received VTG message
// Returns whether the GPS is active or not
bool gps_get_vtg(char *vtg_message);

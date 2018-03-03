#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "nmea.h"
#include "status.h"
#include "uart.h"

typedef void (*evm_gps_gga_handler)(const evm_gps_gga_sentence);

// This struct basically contains all the info about pins etc.
// Check this document on page 4:
// https://www.linxtechnologies.com/wp/wp-content/uploads/evm-gps-f4.pdf
typedef struct {
  UARTPort *port;
  UARTSettings *uart_settings;    // The uart settings to be used
  GPIOSettings *settings_power;   // Pin which provides power
  GPIOSettings *settings_on_off;  // Pin to power up the device
  GPIOAddress *pin_rx;            // Pin addresses
  GPIOAddress *pin_tx;
  GPIOAddress *pin_power;
  GPIOAddress *pin_on_off;
} evm_gps_settings;

StatusCode evm_gps_init(evm_gps_settings *settings);
StatusCode evm_gps_clean_up(evm_gps_settings *settings);

// These methods will add the handler to the handler array, and returns the
// index so that it can
// be removed. If the array is full, then it returns -1.

// The reason for different handlers is so that it is easier to use the driver.
StatusCode evm_gps_add_gga_handler(evm_gps_gga_handler handler, size_t *index);
StatusCode evm_gps_remove_gga_handler(size_t index);
void disable_message_type(EVM_GPS_NMEA_MESSAGE_ID type, char *out);

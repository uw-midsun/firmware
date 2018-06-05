#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "nmea.h"
#include "soft_timer.h"
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

StatusCode gps_init(gps_settings *settings);
StatusCode gps_clean_up(gps_settings *settings);
void gps_dump_internal(SoftTimerID timer_id, void *context);

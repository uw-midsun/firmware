#pragma once
// This will be the GPS driver. It will be responsive for initializing, and cleaning up the GPS
// as well as providing data from the GPS.

// On GPS data: the GPS sends NMEA (National Marine Electronics Association) messages which
// contain data about current time, position, speed, etc. The message we are most interested
// in is the GGA message (Global Positioning System Fix Data). It contains latitude and
// longitude.
#include <stdbool.h>
#include "status.h"
#include "uart.h"

// Just some constants so that the max length of raw data can be set.
// A GGA message will be around a hundred characters.
#define GPS_MAX_NMEA_LENGTH 128

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
  volatile bool gps_active;         // Keeps track of whether the GPS is sending data or not
  volatile bool gps_desired_state;  // Keeps track of whether we want the GPS to be active or not
  UARTStorage uart_storage;
  volatile char gga_data[GPS_MAX_NMEA_LENGTH];  // Stores raw NMEA messages sent by the chip
} GpsSettings;

// Initialized the GPS module
StatusCode gps_init();

// Shuts the GPS module down
StatusCode gps_clean_up();

// Sets: gga_message to the last received GGA message
// Returns: a bool with the GPS' current state (active/inactive)
bool gps_get_gga(char *gga_message);

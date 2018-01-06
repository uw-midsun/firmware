#include "nmea.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "status.h"
#include "util.h"

// Just a function to loosely validate if a sentence is valid
StatusCode is_valid_nmea(const uint8_t *to_check, size_t len) {
  if (len < 3) {
    LOG_DEBUG("The length is too short to contain useful information!\n");
    return STATUS_CODE_UNKNOWN;
  }
  if (to_check[0] != '$') {
    LOG_DEBUG("First character of a NMEA sentence should be a $, it is a %c\n", to_check[0]);
    return STATUS_CODE_UNKNOWN;
  }
  if (to_check[1] != 'G' || to_check[2] != 'P') {
    LOG_DEBUG("NMEA sentence should begin with GP. They are %c%c\n", to_check[1], to_check[2]);
    return STATUS_CODE_UNKNOWN;
  }
  return STATUS_CODE_OK;
}

StatusCode get_nmea_sentence_type(const uint8_t *rx_arr, NMEAMessageID *result) {
  // Array index 3 should be 0, so that it is a null terminated string
  uint8_t message_id[4] = { 0 };
  for (uint16_t i = 3; i < 6; i++) {
    if (rx_arr[i] == ',') {
      break;
    }
    message_id[i - 3] = rx_arr[i];
  }

  // Making sure array index 3 is \n
  message_id[3] = '\0';

  // Parsing which type of NMEA message this is
  if (strcmp((char *)message_id, "GGA") == 0) {
    *result = GGA;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "GLL") == 0) {
    *result = GLL;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "GSA") == 0) {
    *result = GSA;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "GSV") == 0) {
    *result = GSV;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "RMC") == 0) {
    *result = RMC;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "VTG") == 0) {
    *result = VTG;
    return STATUS_CODE_OK;
  } else {
    LOG_WARN("Unknown message type: %c%c%c", message_id[0], message_id[1], message_id[2]);
    result = NULL;
    return STATUS_CODE_INVALID_ARGS;
  }
}

GGASentence parse_nmea_gga_sentence(const uint8_t *rx_arr, size_t len) {
  LOG_DEBUG("NMEA Parser received data with len %zu\n", len);
  GGASentence r = { 0 };
  LOG_DEBUG("%s\n", (char *)rx_arr);

  // Should use context somewhere here to make sure the message is for us

  if (!status_ok(is_valid_nmea(rx_arr, len))) {
    return r;  // Not sure how to deal with an invalid NMEA message being passed in.
  }

  // m_id will keep track of which sentence type we are currently operating on
  NMEAMessageID m_id = 0;

  if (!status_ok(get_nmea_sentence_type(rx_arr, &m_id))) {
    return r;
  }

  r.message_id = m_id;

  // Do checksum right here
  if (compare_checksum((char *)rx_arr)) {
    LOG_WARN("Checksums do not match\n");
  }
  // Parse message_id below

  char temp_buf[NMEAMessageNumFields[GGA]][10];
  int16_t b1 = 0;  // This is on purpose, so it does not underflow.
  uint16_t b = 0;  // It will still works if b1 underflows (because it will
                   // immediately overflow)?

  // Splits individual message components into a 2D array
  for (uint16_t i = 7; i < len; b1++, i++) {
    if (rx_arr[i] == ',') {
      b++;
      b1 = -1;
      continue;
    }
    temp_buf[b][b1] = (char)rx_arr[i];
  }
  if (m_id == GGA) {
    // Example message:
    // $GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64

    // Parses NMEA message
    sscanf(temp_buf[0], "%2d%2d%2d%3d", (int *)&r.time.hh, (int *)&r.time.mm, (int *)&r.time.ss,
           (int *)&r.time.sss);

    sscanf(temp_buf[1], "%2d%2d%4d", (int *)&r.latitude.degrees, (int *)&r.latitude.minutes,
           (int *)&r.latitude.fraction);

    r.north_south = (uint8_t)temp_buf[2][0];

    sscanf(temp_buf[3], "%2d%2d%4d", (int *)&r.longtitude.degrees, (int *)&r.longtitude.minutes,
           (int *)&r.longtitude.fraction);

    r.east_west = (uint8_t)temp_buf[4][0];
    sscanf(temp_buf[5], "%d", (int *)&r.position_fix);
    sscanf(temp_buf[6], "%d", (int *)&r.satellites_used);
    sscanf(temp_buf[7], "%f", &r.hdop);
    sscanf(temp_buf[8], "%f", &r.msl_altitude);
    r.units_1 = (uint8_t)temp_buf[9][0];
    sscanf(temp_buf[10], "%f", &r.geoid_seperation);
    r.units_2 = (uint8_t)temp_buf[11][0];
    sscanf(temp_buf[12], "%d", (int *)&r.adc);
    sscanf(temp_buf[13], "%d", (int *)&r.drs);
    sscanf(temp_buf[13], "%*[^*]*%2x", (int *)&r.checksum);
  }
  return r;
}

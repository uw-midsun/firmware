#pragma once

// This module is used to parse and represent NMEA data. Used in the GPS module

#include <stdint.h>
#include <stdio.h>
#include "status.h"

// This enum contains the list of NMEA sentences (not all of then are supported yet)
// because they do not contain useful information
typedef enum {
  NMEA_UNKNOWN = 0,
  NMEA_GGA,
  NMEA_GLL,
  NMEA_GSA,
  NMEA_GSV,
  NMEA_RMC,
  NMEA_VTG,
  NMEA_NUM_MESSAGE_IDS
} NMEA_MESSAGE_ID;

// This is to get the number of fields in a sentence for array allocation
static const size_t s_nmea_message_num_fields[] = { 0, 16, 9, 11, 13, 14, 14, 10 };

typedef struct {
  uint32_t hh;   // Hours
  uint32_t mm;   // Minutes
  uint32_t ss;   // Seconds
  uint32_t sss;  // Milliseconds
} nmea_utc_time;

// Representation of longitude or latitude
// https://en.wikipedia.org/wiki/Longitude
typedef struct {
  uint32_t degrees;
  uint32_t minutes;
  uint32_t fraction;
} nmea_coord;
// Info passed from the GPS chip should be dropped into this struct (more fields
// coming soon)

typedef struct {
  uint32_t position_fix;  // 0 = invalid, (1,2,6) = valid, all else is undefined
  uint32_t satellites_used;
  uint32_t adc;                 // Age of diff. corr. in seconds
  uint32_t drs;                 // Diff. Ref. Station. Not sure what it is yet
  uint32_t hdop_1;              // Horizontal dilution of precision, characteristic
  uint32_t hdop_2;              // Horizontal dilution of precision, mantissa
  uint32_t msl_altitude_1;      // In meters, characteristic
  uint32_t msl_altitude_2;      // In meters, mantissa
  uint32_t geoid_seperation_1;  // In meters, characteristic
  uint32_t geoid_seperation_2;  // In meters, mantissa
  nmea_coord latitude;
  nmea_coord longitude;
  nmea_utc_time time;
  uint8_t east_west;               // East or West. E for East, W for West, treat as char
  uint8_t north_south;             // North or South. N for North, S for South, treat as char
  uint8_t units_msl_altitude;      // Indicated units of above, should be M for meters.
  uint8_t units_geoid_seperation;  // Indicates units of above, should be M for meters.
  uint8_t checksum[3];             // Should be * followed by two integers
  NMEA_MESSAGE_ID message_id;
} nmea_gga_sentence;

typedef struct {
  uint32_t degrees_1;    // Whole number of degrees
  uint32_t degrees_2;    // Decimal part of degrees
  uint32_t speed_kmh_1;  // Speed in km/h
  uint32_t speed_kmh_2;  // Speed in km/h, fractional part
  uint8_t checksum[3];
} nmea_vtg_sentence;

// Parsing function for gga sentence
StatusCode nmea_get_gga_sentence(const uint8_t *nmea_input, size_t len, nmea_gga_sentence *result);
StatusCode nmea_get_vtg_sentence(const uint8_t *nmea_input, size_t len, nmea_vtg_sentence *result);
StatusCode nmea_valid(const uint8_t *to_check, size_t len);
StatusCode nmea_sentence_type(const uint8_t *rx_arr, size_t len, NMEA_MESSAGE_ID *result);

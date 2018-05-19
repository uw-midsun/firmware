#pragma once
#include <stdint.h>
#include <stdio.h>
#include "status.h"

// This enum contains the list of NMEA sentences (not all of then are supported yet)

// because they do not contain useful information
typedef enum {
  EVM_GPS_UNKNOWN = 0,
  EVM_GPS_GGA,
  EVM_GPS_GLL,
  EVM_GPS_GSA,
  EVM_GPS_GSV,
  EVM_GPS_RMC,
  EVM_GPS_VTG,
  EVM_GPS_NUM_MESSAGE_IDS
} EVM_GPS_NMEA_MESSAGE_ID;

// This is to get the number of fields in a sentence for array allocation
static const int s_nmea_message_num_fields[] = { 0, 16, 9, 11, 13, 14, 14, 10 };

typedef struct {
  uint32_t hh;   // Hours
  uint32_t mm;   // Minutes
  uint32_t ss;   // Seconds
  uint32_t sss;  // Milliseconds
} evm_gps_utc_time;

// Representation of longtitude or latitude
// https://en.wikipedia.org/wiki/Longitude
typedef struct {
  uint32_t degrees;
  uint32_t minutes;
  uint32_t fraction;
} evm_gps_coord;
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
  evm_gps_coord latitude;
  evm_gps_coord longtitude;
  evm_gps_utc_time time;
  uint8_t east_west;               // East or West. E for East, W for West, treat as char
  uint8_t north_south;             // North or South. N for North, S for South, treat as char
  uint8_t units_msl_altitude;      // Indicated units of above, should be M for meters.
  uint8_t units_geoid_seperation;  // Indicates units of above, should be M for meters.
  uint8_t checksum[3];             // Should be * followed by two integers
  EVM_GPS_NMEA_MESSAGE_ID message_id;
} evm_gps_gga_sentence;

typedef struct {
  uint32_t degrees_1;    // Whole number of degrees
  uint32_t degrees_2;    // Decimal part of degrees
  uint32_t speed_kmh_1;  // Speed in km/h
  uint32_t speed_kmh_2;  // Speed in km/h, fractional part
  uint8_t checksum[3];
} evm_gps_vtg_sentence;

// Parsing function for gga sentence
evm_gps_gga_sentence evm_gps_parse_nmea_gga_sentence(const uint8_t *nmea_input, size_t len);
evm_gps_vtg_sentence evm_gps_parse_nmea_vtg_sentence(const uint8_t *nmea_input, size_t len);
StatusCode evm_gps_is_valid_nmea(const uint8_t *to_check, size_t len);
StatusCode evm_gps_get_nmea_sentence_type(const uint8_t *rx_arr, size_t len,
                                          EVM_GPS_NMEA_MESSAGE_ID *result);

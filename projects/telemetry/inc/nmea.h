#pragma once
#include <stdint.h>
#include <stdio.h>
#include "status.h"
#include "uart_mcu.h"

// This enum contains the list of NMEA sentences (not all of then are supported yet)

typedef enum {
  GGA = 0,  // These are the number of data fields in each message type,
            // excluding the message id
  GLL,
  GSA,
  GSV,
  RMC,
  VTG,
  NUM_MESSAGE_IDS
} NMEAMessageID;

static const int NMEAMessageNumFields[] = { 15, 8, 10, 12, 13, 10 };

typedef struct {
  uint32_t hh;   // Hours
  uint32_t mm;   // Minutes
  uint32_t ss;   // Seconds
  uint32_t sss;  // Milliseconds
} UTCTime;

typedef struct {
  uint32_t degrees;
  uint32_t minutes;
  uint32_t fraction;
} Coord;  // Valid representation of longtitude or latitude
// Info passed from the GPS chip should be dropped into this struct (more fields
// coming soon)

typedef struct {
  uint32_t position_fix;  // 0 = invalid, (1,2,6) = valid, all else is undefined
  uint32_t satellites_used;
  uint32_t adc;  // Age of diff. corr. in seconds
  uint32_t drs;  // Diff. Ref. Station. Not sure what it is yet
  Coord latitude;
  Coord longtitude;
  UTCTime time;
  float hdop;              // Horizontal dilution of percision
  float msl_altitude;      // In meters
  float geoid_seperation;  // In meters
  uint8_t east_west;       // East or West. E for East, W for West, treat as char
  uint8_t north_south;     // North or South. N for North, S for South, treat as char
  uint8_t units_1;         // Indicated units of above, should be M for meters.
  uint8_t units_2;         // Indicates units of above, should be M for meters.
  uint8_t checksum[3];     // Should be * followed by two integers
  NMEAMessageID message_id;
} GGASentence;

// Parsing function for gga sentence
GGASentence parse_nmea_gga_sentence(const uint8_t *nmea_input, size_t len);
StatusCode is_valid_nmea(const uint8_t *to_check, size_t len);
StatusCode get_nmea_sentence_type(const uint8_t *rx_arr, NMEAMessageID *result);

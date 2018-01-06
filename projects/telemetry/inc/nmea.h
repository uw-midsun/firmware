#pragma once
#include <stdint.h>
#include <stdio.h>
#include "status.h"
#include "uart_mcu.h"
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
  NMEAMessageID message_id;
  UTCTime time;
  Coord latitude;
  uint8_t north_south;  // North or South. N for North, S for South, treat as char
  Coord longtitude;
  uint8_t east_west;      // East or West. E for East, W for West, treat as char
  uint32_t position_fix;  // 0 = invalid, (1,2,6) = valid, all else is undefined
  uint32_t satellites_used;
  float hdop;              // Horizontal dilution of percision
  float msl_altitude;      // In meters
  uint8_t units_1;         // Indicated units of above, should be M for meters.
  float geoid_seperation;  // In meters
  uint8_t units_2;         // Indicates units of above, should be M for meters.
  uint32_t adc;            // Age of diff. corr. in seconds
  uint32_t drs;            // Diff. Ref. Station. Not sure what it is yet
  uint8_t checksum[3];     // Should be * followed by two integers
} GGASentence;

// This struct will have the parsed sentence in it. It will have multiple
// sentences as they are
// implemented. Others will just be null on return.
// The reason for this "strange" return type is because the function doesn't
// know which sentence
// will be initially passed to it,
// so there isn't any way to overload the function to provide multiple return
// types. Furthermore,
// the code that is calling the parser
// also doesn't know which sentence it is passing, because it is not parsing it
// yet.
typedef struct {
  union {
    GGASentence gga;
    // GLLSentence gll; For instance, if the parse function was returning a gga
    // sentence, then the
    // gll field would be null
  };
  NMEAMessageID message_type;
} NMEAResult;

// Parsing function for gga sentence
GGASentence parse_nmea_gga_sentence(const uint8_t *nmea_input, size_t len);
StatusCode is_valid_nmea(const uint8_t *to_check, size_t len);
StatusCode get_nmea_sentence_type(const uint8_t *rx_arr, NMEAMessageID *result);

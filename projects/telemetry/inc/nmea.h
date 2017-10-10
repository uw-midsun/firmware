#pragma once
#include <stdio.h>
typedef enum {
  GGA = 15, // These are the number of data fields in each message type, excluding the message id
  GLL = 8,
  GSA = 10,
  GSV = 12,
  RMC = 13,
  VTG = 10,
  NUM_MESSAGE_IDS = 6,
} NMEAMessageID;

typedef struct {
  uint32_t hh; // Hours
  uint32_t mm; // Minutes
  uint32_t ss; //Seconds
  uint32_t sss; //Milliseconds
} UTCTime;

typedef struct {
  uint32_t degrees;
  uint32_t minutes;
  uint32_t fraction;
} Coord; // Valid representation of longtitude or latitude
// Info passed from the GPS chip should be dropped into this struct (more fields coming soon)

typedef struct {
  NMEAMessageID message_id;
  UTCTime time;
  Coord latitude;
  uint8_t north_south; // North or South. N for North, S for South, treat as char
  Coord longtitude;
  uint8_t east_west; // East or West. E for East, W for West, treat as char
  uint32_t position_fix; //0 = invalid, (1,2,6) = valid, all else is undefined
  uint32_t satellites_used;
  float hdop; // Horizontal dilution of percision
  float msl_altitude; // In meters
  uint8_t units_1; // Indicated units of above, should be M for meters.
  float geoid_seperation; // In meters
  uint8_t units_2; // Indicates units of above, should be M for meters.
  uint32_t adc; // Age of diff. corr. in seconds
  uint32_t drs; // Diff. Ref. Station. Not sure what it is yet
  uint8_t checksum[3]; // Should be * followed by two integers
  
} GGASentence;

// This struct will have the parsed sentence in it. It will have multiple sentences as they are implemented. Others will just be null on return.
// The reason for this "strange" return type is because the function doesn't know which sentence will be initially passed to it,
// so there isn't any way to overload the function to provide multiple return types. Furthermore, the code that is calling the parser
// also doesn't know which sentence it is passing, because it is not parsing it yet.
typedef struct {
  GGASentence gga;
  // GLLSentence gll; For instance, if the parse function was returning a gga sentence, then the gll field would be null
}NMEAResult;

// Main parsing function
NMEAResult parse_nmea_sentence(const uint8_t *nmea_input, size_t len);
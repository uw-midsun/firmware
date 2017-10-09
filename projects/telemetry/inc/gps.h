#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "status.h"
#include "uart.h"

// This enum contains the list of supported NMEA sentences that are supported by our GPS chip
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


// TODO, make this private and add methods to read from this
GGASentence result;
StatusCode evm_gps_init(void);

// For testing purposes
void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context);
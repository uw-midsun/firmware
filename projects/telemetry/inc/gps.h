#pragma once
// This will be the GPS driver
#include <stdio.h>
#include "status.h"
#include "uart.h"

// This enum contains the list of supported NMEA sentences that are supported by our GPS chip
typedef enum {
  GGA = 0,
  GLL = 1,
  GSA = 2,
  GSV = 3,
  RMC = 4,
  VTG = 5,
  NUM_MESSAGE_IDS = 6,
} NMEAMessageID;

// Info passed from the GPS chip should be dropped into this struct (more fields coming soon)
typedef struct {
  NMEAMessageID message_id;
} NMEASentence;

// TODO, make this private and add methods to read from this
NMEASentence result;
StatusCode evm_gps_init(void);

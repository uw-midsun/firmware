#pragma once

#include <stdint.h>

typedef union ChargerCanJ1939Id {
  uint32_t raw_id;
  struct {
    uint32_t source_address : 8;  // Source
    uint32_t pdu_specifics : 8;   // Destination
    uint32_t pdu_format : 8;      // Packet Format
    uint32_t dp : 1;              // Always 0
    uint32_t r : 1;               // Always 0
    uint32_t priority : 3;        // Anything
  };
} ChargerCanJ1939Id;

// Defined in datasheet.
typedef enum ChargerCanState {
  CHARGER_STATE_START = 0,
  CHARGER_STATE_STOP = 1,
  CHARGER_STATE_OFF = 2,
  NUM_CHARGER_STATES,
} ChargerCanState;

// Also defined in datasheet.
typedef union ChargerCanStatus {
  uint8_t raw;
  struct {
    uint8_t hw_fault : 1;
    uint8_t over_temp : 1;
    uint8_t input_voltage : 1;
    uint8_t starting_state : 1;
    uint8_t comms_state : 1;
  };
} ChargerCanStatus;

typedef struct ChargerCanTxDataImpl {
  uint16_t max_voltage;
  uint16_t max_current;
  uint8_t charging;
} ChargerCanTxDataImpl;

typedef union ChargerCanTxData {
  uint64_t raw_data;
  ChargerCanTxDataImpl data_impl;
} ChargerCanTxData;

typedef struct ChargerCanRxDataImpl {
  uint16_t voltage;
  uint16_t current;
  ChargerCanStatus status_flags;
} ChargerCanRxDataImpl;

typedef union ChargerControllerRxData {
  uint64_t raw_data;
  ChargerCanRxDataImpl data_impl;
} ChargerCanRxData;

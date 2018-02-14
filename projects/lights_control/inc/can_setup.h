#pragma once

#include "can.h"
#include "lights_events.h"
#include "structs.h"

#define CAN_NUM_RX_HANDLERS 1

StatusCode can_setup_init(BoardType type);

typedef enum {
  PERIPH_SIGNAL_RIGHT,
  PERIPH_SIGNAL_LEFT,
  PERIPH_SIGNAL_HAZARD,
  PERIPH_HORN,
  PERIPH_HEADLIGHTS,
  PERIPH_BRAKE,
  PERIPH_STROBE,
  NUM_PERIPHERALS
} CanSetupPeripheral;

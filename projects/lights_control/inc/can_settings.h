#pragma once

#include "can.h"
#include "structs.h"
#include "lights_events.h"

#define CAN_NUM_RX_HANDLERS 5

StatusCode initialize_can_settings(BoardType type);

typedef enum {
  PERIPH_SIGNAL_RIGHT,
  PERIPH_SIGNAL_LEFT,
  PERIPH_SIGNAL_HAZARD,
  PERIPH_HORN,
  PERIPH_HEADLIGHTS,
  PERIPH_BRAKE,
  PERIPH_STROBE,
  NUM_PERIPHERALS
} Peripheral;


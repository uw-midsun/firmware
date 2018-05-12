#pragma once

#include "status.h"

typedef enum {
  LIGHTS_EVENT_CAN_RX = 0,
  LIGHTS_EVENT_CAN_TX,
  LIGHTS_EVENT_CAN_FAULT,
  // Events processed by the lights_gpio module.
  LIGHTS_EVENT_GPIO_OFF,
  LIGHTS_EVENT_GPIO_ON,
  // Events processed by the lights_signal module.
  LIGHTS_EVENT_SIGNAL_OFF,
  LIGHTS_EVENT_SIGNAL_ON,
  NUM_LIGHTS_EVENTS
} LightsEvent;

// Possible data fields to be used with a lights gpio event.
typedef enum {
  LIGHTS_EVENT_GPIO_DATA_HORN = 0,
  LIGHTS_EVENT_GPIO_DATA_HIGH_BEAMS,
  LIGHTS_EVENT_GPIO_DATA_LOW_BEAMS,
  LIGHTS_EVENT_GPIO_DATA_DRL,
  LIGHTS_EVENT_GPIO_DATA_SIGNAL_HAZARD,
  LIGHTS_EVENT_GPIO_DATA_SIGNAL_LEFT,
  LIGHTS_EVENT_GPIO_DATA_SIGNAL_RIGHT,
  LIGHTS_EVENT_GPIO_DATA_STROBE,
  LIGHTS_EVENT_GPIO_DATA_BRAKES,
  LIGHTS_EVENT_GPIO_DATA_SYNC,
  NUM_LIGHTS_EVENT_DATAS
} LightsEventGPIOData;

// Possible data fields to be used with a lights signal event.
typedef enum {
  LIGHTS_EVENT_SIGNAL_DATA_LEFT = 0,
  LIGHTS_EVENT_SIGNAL_DATA_RIGHT,
  LIGHTS_EVENT_SIGNAL_DATA_HAZARD
} LightsEventSignalData;

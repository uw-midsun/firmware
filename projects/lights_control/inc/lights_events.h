#pragma once

#include "status.h"

#define LIGHTS_EVENTS_BLINK_ON_OFFSET 4
#define LIGHTS_EVENTS_BLINK_OFF_OFFSET 8

typedef enum {
  LIGHTS_EVENT_CAN_RX = 0,
  LIGHTS_EVENT_CAN_TX,
  LIGHTS_EVENT_CAN_FAULT,
  LIGHTS_EVENT_HORN,
  LIGHTS_EVENT_HIGH_BEAMS,
  LIGHTS_EVENT_LOW_BEAMS,
  LIGHTS_EVENT_DRL,
  // Do NOT change the below events' orders.
  LIGHTS_EVENT_SIGNAL_HAZARD_BLINK_OFF,
  LIGHTS_EVENT_SIGNAL_LEFT_BLINK_OFF,
  LIGHTS_EVENT_SIGNAL_RIGHT_BLINK_OFF,
  LIGHTS_EVENT_STROBE_BLINK_OFF,
  LIGHTS_EVENT_SIGNAL_HAZARD_BLINK_ON,
  LIGHTS_EVENT_SIGNAL_LEFT_BLINK_ON,
  LIGHTS_EVENT_SIGNAL_RIGHT_BLINK_ON,
  LIGHTS_EVENT_STROBE_BLINK_ON,
  LIGHTS_EVENT_SIGNAL_HAZARD,
  LIGHTS_EVENT_SIGNAL_LEFT,
  LIGHTS_EVENT_SIGNAL_RIGHT,
  LIGHTS_EVENT_STROBE,
  // Do NOT change above events' orders.
  LIGHTS_EVENT_BRAKES,
  LIGHTS_EVENT_SYNC,
  NUM_LIGHTS_EVENTS
} LightsEvent;

StatusCode lights_events_get_blink_on_event(LightsEvent event, LightsEvent *blink_event);

StatusCode lights_events_get_blink_off_event(LightsEvent event, LightsEvent *blink_event);

#pragma once

typedef enum {
  LIGHTS_SIMPLE_STATE_ON = 0,
  LIGHTS_SIMPLE_STATE_OFF,
  NUM_LIGHTS_SIMPLE_STATES,
} LightsSimpleState;

typedef StatusCode (*LightsSimplePeripheralCallback)(const Event *e);

// Registers the callback for all the events that correspond to "simple" peripherals, that is: horn,
// headlights, brakes
StatusCode lights_simple_peripherals_init(LightsSimplePeripheralCallback cb);

// processes the event if it relates to this module, that is: horn, high beams, low beams, daytime
// running lights(DRL) and brakes.
StatusCode lights_simple_peripherals_process_event(const Event *e);

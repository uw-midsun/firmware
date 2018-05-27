#pragma once

// Every event is post-fixed by the module that processes it.
// i.e. LIGHTS_EVENT_GPIO* is used by the lights_gpio module.

// TODO(ELEC-372): Rename this file to lights_event.h
typedef enum {
  // Used internally by the can module.
  LIGHTS_EVENT_CAN_RX = 0,
  LIGHTS_EVENT_CAN_TX,
  LIGHTS_EVENT_CAN_FAULT,
  // Events processed by lights_gpio module.
  LIGHTS_EVENT_GPIO_OFF,
  LIGHTS_EVENT_GPIO_ON,
  // Events processed by lights_signal module.
  LIGHTS_EVENT_SIGNAL_OFF,
  LIGHTS_EVENT_SIGNAL_ON,
  // Events processed by lights_strobe module.
  LIGHTS_EVENT_STROBE_OFF,
  LIGHTS_EVENT_STROBE_ON,
  // Event processed by lights_sync module.
  LIGHTS_EVENT_SYNC,
  LIGHTS_EVENT_BPS_HEARTBEAT,
  NUM_LIGHTS_EVENTS
} LightsEvent;

// Possible data fields to be used with a LIGHTS_EVENT_GPIO_* event.
typedef enum {
  LIGHTS_EVENT_GPIO_PERIPHERAL_HIGH_BEAMS = 0,
  LIGHTS_EVENT_GPIO_PERIPHERAL_LOW_BEAMS,
  LIGHTS_EVENT_GPIO_PERIPHERAL_DRL,
  LIGHTS_EVENT_GPIO_PERIPHERAL_BRAKES,
  LIGHTS_EVENT_GPIO_PERIPHERAL_STROBE,
  LIGHTS_EVENT_GPIO_PERIPHERAL_SYNC,
  LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_HAZARD,
  LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_LEFT,
  LIGHTS_EVENT_GPIO_PERIPHERAL_SIGNAL_RIGHT,
  LIGHTS_EVENT_GPIO_PERIPHERAL_HORN,
  NUM_LIGHTS_EVENT_GPIO_PERIPHERALS
} LightsEventGpioPeripheral;

// Possible data fields to be used with a LIGHTS_EVENT_SIGNAL_* event.
typedef enum {
  LIGHTS_EVENT_SIGNAL_MODE_LEFT = 0,
  LIGHTS_EVENT_SIGNAL_MODE_RIGHT,
  LIGHTS_EVENT_SIGNAL_MODE_HAZARD,
  NUM_LIGHTS_EVENT_SIGNAL_MODES
} LightsEventSignalMode;

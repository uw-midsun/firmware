#pragma once

#include "event_queue.h"

// High priority messages in the event queue
typedef enum {
  CENTER_CONSOLE_EVENT_DUMMY = 0,
  NUM_CENTER_CONSOLE_EVENTS_CRITICAL,
} CenterConsoleEventsCritical;

// CAN related messages in the event queue
typedef enum {
  CENTER_CONSOLE_EVENT_CAN_FAULT = NUM_CENTER_CONSOLE_EVENTS_CRITICAL + 1,
  CENTER_CONSOLE_EVENT_CAN_RX,
  CENTER_CONSOLE_EVENT_CAN_TX,
  NUM_CENTER_CONSOLE_EVENTS_CAN,
} CenterConsoleEventsCAN;

// State transition messages in the event queue
typedef enum {
  CENTER_CONSOLE_EVENT_BUTTON_TOGGLE_STATE = NUM_CENTER_CONSOLE_EVENTS_CAN + 1,
  CENTER_CONSOLE_EVENT_BUTTON_NEUTRAL_PRESSED,
  CENTER_CONSOLE_EVENT_BUTTON_REVERSE_PRESSED,
  CENTER_CONSOLE_EVENT_BUTTON_DRIVE_PRESSED,
  NUM_CENTER_CONSOLE_EVENTS_BUTTON,
} CenterConsoleEventsButton;

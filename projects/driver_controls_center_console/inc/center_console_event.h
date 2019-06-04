#pragma once

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

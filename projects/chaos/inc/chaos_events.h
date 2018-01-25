#pragma once

// High priority messages in the event queue
typedef enum {
  NUM_CHAOS_EVENTS_CRITICAL = 0,
} ChaosEventCritical;

// CAN messages in the event queue
typedef enum {
  CHAOS_EVENT_CAN_UV_OV = NUM_CHAOS_EVENTS_CRITICAL + 1,
  NUM_CHAOS_EVENTS_CAN,
} ChaosEventCAN;

// State transition messages in the event queue
typedef enum {
  NUM_CHAOS_EVENTS_FSM = NUM_CHAOS_EVENTS_CAN + 1,
} ChaosEventFSM;

#define NUM_CHAOS_EVENTS NUM_CHAOS_EVENTS_FSM

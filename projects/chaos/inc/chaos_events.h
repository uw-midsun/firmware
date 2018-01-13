#pragma once

// High priority messages in the event queue
typedef enum {
  NUM_CHAOS_EVENTS_CRITICAL = 0,
} ChaosEventCritical;

// CAN related messages in the event queue
typedef enum {
  CHAOS_EVENT_CAN_TRANSMIT_ERROR = NUM_CHAOS_EVENTS_CRITICAL + 1,
  CHAOS_EVENT_CAN_FAULT,
  CHAOS_EVENT_CAN_RX,
  CHAOS_EVENT_CAN_TX,
  NUM_CHAOS_EVENTS_CAN,
} ChaosEventCAN;

// State transition messages in the event queue
typedef enum {
  NUM_CHAOS_EVENTS_FSM = NUM_CHAOS_EVENTS_CAN + 1,
} ChaosEventFSM;

#define NUM_CHAOS_EVENTS NUM_CHAOS_EVENTS_FSM

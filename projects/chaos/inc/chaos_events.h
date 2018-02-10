#pragma once

// High priority messages in the event queue
typedef enum {
  CHAOS_EVENT_RELAY_ERROR = 0,
  NUM_CHAOS_EVENTS_CRITICAL,  // 1
} ChaosEventsCritical;

// CAN related messages in the event queue
typedef enum {
  CHAOS_EVENT_CAN_TRANSMIT_ERROR = NUM_CHAOS_EVENTS_CRITICAL + 1,  // 2
  CHAOS_EVENT_CAN_FAULT,
  CHAOS_EVENT_CAN_RX,
  CHAOS_EVENT_CAN_TX,
  CHAOS_EVENT_RETRY_RELAY,
  NUM_CHAOS_EVENTS_CAN,  // 7
} ChaosEventsCAN;

// State transition messages in the event queue (only 1 should be in the queue at a time so order is
// irrelevant).
typedef enum {
  CHAOS_EVENT_NO_OP = NUM_CHAOS_EVENTS_CAN + 1,  // 8
  CHAOS_EVENT_MONITOR_ENABLE,
  CHAOS_EVENT_MONITOR_DISABLE,
  CHAOS_EVENT_OPEN_RELAY,
  CHAOS_EVENT_CLOSE_RELAY,
  CHAOS_EVENT_RELAY_OPENED,
  CHAOS_EVENT_RELAY_CLOSED,
  CHAOS_EVENT_GPIO_IDLE,
  CHAOS_EVENT_GPIO_CHARGE,
  CHAOS_EVENT_GPIO_DRIVE,
  NUM_CHAOS_EVENTS_FSM  // 18
} ChaosEventsFSM;

// Used for the sequence generator that schedules and order events for the other FSMs.
typedef enum {
  CHAOS_EVENT_SEQUENCE_IDLE = NUM_CHAOS_EVENTS_FSM + 1,  // 19
  CHAOS_EVENT_SEQUENCE_CHARGE,
  CHAOS_EVENT_SEQUENCE_DRIVE,
  CHAOS_EVENT_SEQUENCE_RESET,
  NUM_CHAOS_EVENT_SEQUENCES,  // 23
} ChaosEventSequence;

#define NUM_CHAOS_EVENTS NUM_CHAOS_EVENT_SEQUENCES

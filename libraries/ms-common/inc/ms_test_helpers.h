#pragma once
// Test helpers for common test paradigms.
//
// All require event_queue to be initialized.

#include "can.h"
#include "delay.h"
#include "event_queue.h"
#include "fsm.h"
#include "status.h"
#include "unity.h"

// Awaits an event and populates |e| with that event.
#define MS_TEST_HELPER_AWAIT_EVENT(e)     \
  ({                                      \
    StatusCode status = NUM_STATUS_CODES; \
    do {                                  \
      status = event_process(&(e));       \
    } while (status != STATUS_CODE_OK);   \
  })

// The following require CAN and soft_timer to be initialized.

// Send a TX message over CAN and RX it.
#define MS_TEST_HELPER_CAN_TX_RX(tx_event, rx_event)           \
  ({                                                           \
    Event e = { 0, 0 };                                        \
    MS_TEST_HELPER_AWAIT_EVENT(e);                             \
    TEST_ASSERT_EQUAL((tx_event).id, e.id);                    \
    TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &(tx_event))); \
    delay_us(1000);                                            \
    MS_TEST_HELPER_AWAIT_EVENT(e);                             \
    TEST_ASSERT_EQUAL((rx_event).id, e.id);                    \
    TEST_ASSERT_TRUE(fsm_process_event(CAN_FSM, &(rx_event))); \
  })

// Send a TX message over CAN and RX it, then respond with an ACK.
#define MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(tx_event, rx_event) \
  ({                                                          \
    MS_TEST_HELPER_CAN_TX_RX((tx_event), (rx_event));         \
    delay_us(1000);                                           \
    MS_TEST_HELPER_CAN_TX_RX((tx_event), (rx_event));         \
    delay_us(1000);                                           \
  })

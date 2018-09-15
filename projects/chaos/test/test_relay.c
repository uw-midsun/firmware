#include "relay.h"

#include <inttypes.h>
#include <stddef.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "relay_fsm.h"
#include "relay_id.h"
#include "relay_retry_service.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_RELAY_DELAY_MS 50

static RelayRetryServiceStorage s_relay_retry_storage;
static CANStorage s_storage;

static StatusCode prv_rx_handler(const CANMessage *msg, void *context, CANAckStatus *ack_reply) {
  CANAckStatus *status = context;
  *ack_reply = *status;
  LOG_DEBUG("Handled; responded with %u\n", *ack_reply);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  relay_retry_service_init(&s_relay_retry_storage, TEST_RELAY_DELAY_MS);
  Event e = { .id = CHAOS_EVENT_SET_RELAY_RETRIES, .data = RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS };
  relay_retry_service_update(&e);

  CANSettings settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_storage, &settings);

  RelaySettings relay_settings = {
    .battery_main_power_pin = { GPIO_PORT_A, 0 },
    .battery_slave_power_pin = { GPIO_PORT_A, 0 },
    .motor_power_pin = { GPIO_PORT_A, 0 },
    .solar_front_power_pin = { GPIO_PORT_A, 0 },
    .solar_rear_power_pin = { GPIO_PORT_A, 0 },
    .loopback = true,
  };
  GpioAddress addr = { GPIO_PORT_A, 0 };
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };
  gpio_init_pin(&addr, &gpio_settings);

  relay_init(&relay_settings);
}

void teardown_test(void) {}

typedef struct TestRelayEvents {
  Event invoke_event;
  Event response_event;
} TestRelayEvents;

typedef struct TestRelayParams {
  RelayId id;
  bool retry[2];
} TestRelayParams;

// Iterate through all the states of each relay to validate their behavior with retry logic based on
// the 4 different success/failure combinations for opening and closing. Essentially checks all
// success cases, handling of failures, etc.
void test_relay_cycle(void) {
  CANAckStatus ack_status = CAN_ACK_STATUS_OK;

  can_register_rx_default_handler(prv_rx_handler, &ack_status);

  // RelayID being tested and in which conditions (close, open) to fail.
  const TestRelayParams params[] = {
    { .id = RELAY_ID_MOTORS, .retry = { true, true } },
    { .id = RELAY_ID_BATTERY_MAIN, .retry = { true, false } },
    { .id = RELAY_ID_BATTERY_SLAVE, .retry = { true, false } },
    { .id = RELAY_ID_SOLAR_MASTER_REAR, .retry = { false, false } },
    { .id = RELAY_ID_SOLAR_MASTER_FRONT, .retry = { false, true } }
  };

  // EventsIDs for closing and opening.
  TestRelayEvents events[] = {
    { .invoke_event = { .id = CHAOS_EVENT_CLOSE_RELAY },
      .response_event = { .id = CHAOS_EVENT_RELAY_CLOSED } },
    { .invoke_event = { .id = CHAOS_EVENT_OPEN_RELAY },
      .response_event = { .id = CHAOS_EVENT_RELAY_OPENED } },
  };

  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;

  // For each RelayID.
  for (uint16_t i = 0; i < SIZEOF_ARRAY(params); i++) {
    LOG_DEBUG("Id %u\n", params[i].id);

    // For each set of test parameters.
    for (uint16_t j = 0; j < SIZEOF_ARRAY(events); j++) {
      LOG_DEBUG("Event %u\n", j);
      events[j].invoke_event.data = params[i].id;
      events[j].response_event.data = params[i].id;
      ack_status = params[i].retry[j] ? CAN_ACK_STATUS_INVALID : CAN_ACK_STATUS_OK;

      uint16_t can_cnt = 0;
      // Each ACK event causes 4 event (RX and TX on sender and receiver).
      uint16_t expected_can_cnt = 4 + 4 * params[i].retry[j];
      uint16_t event_cnt = 0;
      // Each FSM event causes 1 event (RX and TX on sender and receiver).
      uint16_t expected_event_cnt = 1 + params[i].retry[j];
      uint16_t retry_cnt = 0;
      uint16_t expected_retry_cnt = params[i].retry[j];

      // Start the test
      TEST_ASSERT_TRUE(relay_process_event(&events[j].invoke_event));

      // While we still expect events keep trying. The exact order of RX/TX vs FSM processing are
      // non-deterministic due to threading.
      for (uint16_t k = 0; k < expected_can_cnt + expected_event_cnt + expected_retry_cnt; k++) {
        // Get an event to handle.
        do {
          status = event_process(&e);
        } while (status != STATUS_CODE_OK);

        // If it is a CAN event let the CAN_FSM handle it.
        if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX ||
            e.id == CHAOS_EVENT_CAN_FAULT) {
          TEST_ASSERT_TRUE(can_process_event(&e));
          can_cnt++;
        } else if (e.id == CHAOS_EVENT_MAYBE_RETRY_RELAY) {
          TEST_ASSERT_OK(relay_retry_service_update(&e));
          retry_cnt++;
        } else {
          // If the ACK status is currently OK make sure the event is the expected one.
          if (ack_status == CAN_ACK_STATUS_OK) {
            TEST_ASSERT_EQUAL(events[j].response_event.id, e.id);
            TEST_ASSERT_EQUAL(events[j].response_event.data, e.data);
          }
          // Process the event.
          TEST_ASSERT_TRUE(relay_process_event(&e));
          ack_status = CAN_ACK_STATUS_OK;
          event_cnt++;
        }
      }

      // Assert the expected number of each type of event occurred.
      TEST_ASSERT_EQUAL(expected_can_cnt, can_cnt);
      TEST_ASSERT_EQUAL(expected_event_cnt, event_cnt);
      TEST_ASSERT_EQUAL(expected_retry_cnt, retry_cnt);
    }
  }
}

// Validates that the retry limit is a cap on the number of attempts at triggering a Relay.
void test_relay_retry_limit(void) {
  // Handler config.
  CANAckStatus ack_status = CAN_ACK_STATUS_INVALID;
  can_register_rx_default_handler(prv_rx_handler, &ack_status);

  // Start the FSM.
  Event e = { 0 };
  TEST_ASSERT_OK(relay_fsm_close_event(RELAY_ID_BATTERY_MAIN, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  // Track the number of retries.
  uint16_t retries = 0;
  StatusCode status = NUM_STATUS_CODES;

  // While the retry cap event isn't reached keep retrying.
  while (e.id != CHAOS_EVENT_RELAY_ERROR) {
    // Grab a valid event.
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);

    // Handle CAN message
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else if (e.id == CHAOS_EVENT_MAYBE_RETRY_RELAY) {
      TEST_ASSERT_OK(relay_retry_service_update(&e));
    } else if (e.id != CHAOS_EVENT_RELAY_ERROR) {
      // Handle any other message other than reaching the retry cap.
      TEST_ASSERT_TRUE(relay_process_event(&e));
      retries++;
    }
  }

  // Assert the correct event and number of retries occurred.
  TEST_ASSERT_EQUAL(CHAOS_EVENT_RELAY_ERROR, e.id);
  TEST_ASSERT_EQUAL(RELAY_ID_BATTERY_MAIN, e.data);
  TEST_ASSERT_EQUAL(RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS, retries);
}

// Validate that the events are in fact separate FSMs and don't interfere with each other.
void test_relay_concurrent(void) {
  CANAckStatus ack_status = CAN_ACK_STATUS_OK;

  can_register_rx_default_handler(prv_rx_handler, &ack_status);

  // Close the battery relays using the same harness as the previous test.
  Event e = { 0 };
  TEST_ASSERT_OK(relay_fsm_close_event(RELAY_ID_BATTERY_SLAVE, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  StatusCode status = NUM_STATUS_CODES;

  while (!(e.id == CHAOS_EVENT_RELAY_CLOSED && e.data == RELAY_ID_BATTERY_SLAVE)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else {
      TEST_ASSERT_TRUE(relay_process_event(&e));
    }
  }
  TEST_ASSERT_OK(relay_fsm_close_event(RELAY_ID_BATTERY_MAIN, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  while (!(e.id == CHAOS_EVENT_RELAY_CLOSED && e.data == RELAY_ID_BATTERY_MAIN)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else {
      TEST_ASSERT_TRUE(relay_process_event(&e));
    }
  }

  // Close the main power relay.
  TEST_ASSERT_OK(relay_fsm_close_event(RELAY_ID_MOTORS, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  while (!(e.id == CHAOS_EVENT_RELAY_CLOSED && e.data == RELAY_ID_MOTORS)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else {
      TEST_ASSERT_TRUE(relay_process_event(&e));
    }
  }

  // Open the main power relay.
  TEST_ASSERT_OK(relay_fsm_open_event(RELAY_ID_MOTORS, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  while (!(e.id == CHAOS_EVENT_RELAY_OPENED && e.data == RELAY_ID_MOTORS)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else {
      TEST_ASSERT_TRUE(relay_process_event(&e));
    }
  }

  // Open the battery relays.
  TEST_ASSERT_OK(relay_fsm_open_event(RELAY_ID_BATTERY_MAIN, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  while (!(e.id == CHAOS_EVENT_RELAY_OPENED && e.data == RELAY_ID_BATTERY_MAIN)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else {
      TEST_ASSERT_TRUE(relay_process_event(&e));
    }
  }
  TEST_ASSERT_OK(relay_fsm_open_event(RELAY_ID_BATTERY_SLAVE, &e));
  TEST_ASSERT_TRUE(relay_process_event(&e));

  while (!(e.id == CHAOS_EVENT_RELAY_OPENED && e.data == RELAY_ID_BATTERY_SLAVE)) {
    do {
      status = event_process(&e);
    } while (status != STATUS_CODE_OK);
    if (e.id == CHAOS_EVENT_CAN_RX || e.id == CHAOS_EVENT_CAN_TX || e.id == CHAOS_EVENT_CAN_FAULT) {
      TEST_ASSERT_TRUE(can_process_event(&e));
    } else {
      TEST_ASSERT_TRUE(relay_process_event(&e));
    }
  }
}

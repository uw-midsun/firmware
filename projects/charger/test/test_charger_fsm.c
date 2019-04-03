#include "charger_fsm.h"

#include <stdbool.h>

#include "can_interval.h"
#include "charger_controller.h"
#include "charger_events.h"
#include "charger_pin.h"
#include "delay.h"
#include "event_queue.h"
#include "generic_can_hw.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "notify.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CHARGER_MAX_VOLTAGE 100
#define TEST_CHARGER_MAX_CURRENT 200

static GenericCanHw s_can;

void setup_test(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  can_interval_init();

  const CanHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  TEST_ASSERT_OK(generic_can_hw_init(&s_can, &can_hw_settings, CHARGER_EVENT_CAN_FAULT));
}

void teardown_test(void) {}

// No need to validate anything other than transitions as the underlying modules are tested and
// fully testing the behavior would require a massive test.
void test_charger_fsm_transitions(void) {
  Fsm fsm;
  GenericCan *can = (GenericCan *)&s_can;
  ChargerCanStatus status = { 0 };
  charger_fsm_init(&fsm);

  ChargerSettings settings = {
    .max_voltage = TEST_CHARGER_MAX_VOLTAGE,
    .max_current = TEST_CHARGER_MAX_CURRENT,
    .can = can,  // Use pure HW can for both CAN and CAN UART since Extended support is needed.
    .can_mcp2515 = can,
  };
  ChargerStorage storage;
  // Init these because they need to be not because they will be used.
  TEST_ASSERT_OK(charger_controller_init(&storage, &settings, &status));
  TEST_ASSERT_OK(notify_init(can, UINT32_MAX, UINT32_MAX));

  // Check ban transitions from Disconnected
  Event e = { CHARGER_EVENT_DISCONNECTED, 0 };
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_START_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_STOP_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_CONNECTED_CHARGING_ALLOWED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));

  // Transition to connected and blocked transitions
  e.id = CHARGER_EVENT_CONNECTED;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_STOP_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_START_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));

  // Transition to connected, charging allowed
  e.id = CHARGER_EVENT_CONNECTED_CHARGING_ALLOWED;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));

  // Prevent transition to charging
  status.hw_fault = true;
  e.id = CHARGER_EVENT_START_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));

  // Allow transition to charging
  status.hw_fault = false;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));

  // Invalid transitions
  e.id = CHARGER_EVENT_CONNECTED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_START_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_CONNECTED_CHARGING_ALLOWED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_CONNECTED_NO_CHARGING_ALLOWED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));

  // Transition back to disconnected
  e.id = CHARGER_EVENT_STOP_CHARGING;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_DISCONNECTED;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));

  // Go back to charging
  e.id = CHARGER_EVENT_CONNECTED;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_CONNECTED_CHARGING_ALLOWED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_START_CHARGING;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));

  // Finally go to not charging directly
  e.id = CHARGER_EVENT_DISCONNECTED;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));

  // Go to error state
  e.id = CHARGER_EVENT_ERROR;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));

  // Test error state only escapes through disconnected
  e.id = CHARGER_EVENT_CONNECTED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_CONNECTED_CHARGING_ALLOWED;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_CONNECTED_NO_CHARGING_ALLOWED TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_START_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_STOP_CHARGING;
  TEST_ASSERT_FALSE(fsm_process_event(&fsm, &e));
  e.id = CHARGER_EVENT_DISCONNECTED;
  TEST_ASSERT_TRUE(fsm_process_event(&fsm, &e));
}

// TODO(ELEC-355): Add full integration test.

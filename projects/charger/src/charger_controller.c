#include "charger_controller.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "can_interval.h"
#include "can_transmit.h"
#include "charger_events.h"
#include "event_queue.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "status.h"

#define CHARGER_PERIOD_US 1000000  // 1 Second as defined in datasheet.

#define CHARGER_EXPECTED_RX_DLC 5
#define CHARGER_EXPECTED_RX_ID 0x18FF50E5

#define CHARGER_EXPECTED_TX_DLC 5
#define CHARGER_EXPECTED_TX_ID 0x1806E5F4

static CanInterval *s_interval;
static ChargerStatus *s_charger_status;
static GenericCanMsg s_tx_msg;

typedef union J1939CanId {
  uint32_t raw_id;
  struct {
    uint32_t source_address : 8;  // Source
    uint32_t pdu_specifics : 8;   // Destination
    uint32_t pdu_format : 8;      // Packet Format
    uint32_t dp : 1;              // Always 0
    uint32_t r : 1;               // Always 0
    uint32_t priority : 3;        // Anything
  };
} J1939CanId;

// Explicit for readability.
static const J1939CanId s_rx_id = {
  .source_address = 0xE5,  // Indicates BMS in the J1939 standard.
  .pdu_specifics = 0x50,   // Broadcast address (BCA) id.
  .pdu_format = 0xFF,      // From datasheet.
  .dp = 0,
  .r = 0,
  .priority = 0x06,  // From datasheet.
};
static const J1939CanId s_tx_id = {
  .source_address = 0xF4,  // Indicates BMS in the J1939 standard.
  .pdu_specifics = 0xE5,   // Charger control system (CCS) id.
  .pdu_format = 0x06,      // From datasheet.
  .dp = 0,
  .r = 0,
  .priority = 0x06,  // From datasheet.
};

// GenericCanRx
static void prv_rx_handler(const GenericCanMsg *msg, void *context) {
  (void)context;
  // TODO(ELEC-355): Rebroadcast this message after transform to the car.

  const ChargerControllerRxData data = {
    .raw_data = msg->data,
  };
  *s_charger_status = data.data_impl.status_flags;

  // Check for statuses
  if (!charger_controller_is_safe(data.data_impl.status_flags)) {
    // If unsafe immediately stop charging and force the FSM to transition to the unsafe state.
    charger_controller_set_state(CHARGER_STATE_STOP);
    event_raise(CHARGER_EVENT_STOP_CHARGING, 0);
  }
}

StatusCode charger_controller_init(ChargerSettings *settings, ChargerStatus *status) {
  assert(s_tx_id.raw_id == CHARGER_EXPECTED_TX_ID);
  assert(s_rx_id.raw_id == CHARGER_EXPECTED_RX_ID);
  s_charger_status = status;

  const ChargerControllerTxData tx_data = { .data_impl = {
                                                .max_voltage = settings->max_voltage,
                                                .max_current = settings->max_current,
                                                .charging = CHARGER_STATE_STOP,
                                            } };

  s_tx_msg.id = s_tx_id.raw_id;
  s_tx_msg.dlc = CHARGER_EXPECTED_TX_DLC;
  s_tx_msg.data = tx_data.raw_data;
  s_tx_msg.extended = true;

  status_ok_or_return(
      can_interval_factory(settings->can_uart, &s_tx_msg, CHARGER_PERIOD_US, &s_interval));

  status_ok_or_return(generic_can_register_rx(settings->can_uart, prv_rx_handler,
                                              GENERIC_CAN_EMPTY_MASK, s_rx_id.raw_id, true,
                                              settings->can));
  status_ok_or_return(can_interval_enable(s_interval));
  return STATUS_CODE_OK;
}

StatusCode charger_controller_set_state(ChargerState state) {
  if (state >= NUM_CHARGER_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (!charger_controller_is_safe(*s_charger_status)) {
    // Unsafe to start
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }
  // TODO(ELEC-355): Consider disabling rx rebroadcasts if off (1/s).

  ChargerControllerTxData tx_data = {
    .raw_data = s_tx_msg.data,
  };

  tx_data.data_impl.charging = state;
  s_tx_msg.data = tx_data.raw_data;
  return can_interval_send_now(s_interval);
}

bool charger_controller_is_safe(ChargerStatus status) {
  // - Input voltage is an indication the charger is lower voltage the battery so don't charge.
  // - Over Temp is an indication of a heat issue with the charger, it is possible to resume
  //   charging after a break.
  // - HW fault is an indeterminate hardware failure which should be addressed.
  //
  // Implicitly forgives communication issues.
  return !(status.input_voltage || status.over_temp || status.hw_fault);
}

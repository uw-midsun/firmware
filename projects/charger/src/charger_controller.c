#include "charger_controller.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "can_interval.h"
#include "can_transmit.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "status.h"

#define CHARGER_PERIOD_US 1000000  // 1 Second as defined in datasheet.

#define CHARGER_EXPECTED_RX_DLC 5
#define CHARGER_EXPECTED_RX_ID 0x18FF50E5

#define CHARGER_EXPECTED_TX_DLC 5
#define CHARGER_EXPECTED_TX_ID 0x1806E5F4

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

static CanInterval *s_interval;
static ChargerStatus *s_charger_status;
static GenericCanMsg s_tx_msg;
static const J1939CanId s_rx_id = {
  .source_address = 0xE5,  // Indicates BMS in the J1939 standard.
  .pdu_specifics = 0xF4,   // Charger control system (CCS) id.
  .pdu_format = 0xFF,      // From datasheet.
  .dp = 0,
  .r = 0,
  .priority = 0x06,  // From datasheet.
};
static_assert(CHARGER_EXPECTED_RX_ID == rx_id.raw_id);

typedef struct ChargerControllerTxDataImpl {
  uint16_t max_voltage;
  uint16_t max_current;
  uint8_t charging;
} ChargerControllerTxDataImpl;
static_assert(CHARGER_EXPECTED_TX_DLC == sizeof(ChargerControllerTxDataImpl));

typedef union ChargerControllerTxData {
  uint64_t raw_data;
  ChargerControllerTxDataImpl data_impl;
} ChargerControllerTxData;

typedef struct ChargerControllerRxDataImpl {
  uint16_t voltage;
  uint16_t current;
  ChargerStatus status_flags;
} ChargerControllerRxDataImpl;
static_assert(CHARGER_EXPECTED_RX_DLC == sizeof(ChargerControllerRxDataImpl));

typedef union ChargerControllerRxData {
  uint64_t raw_data;
  ChargerControllerRxDataImpl data_impl;
} ChargerControllerRxData;

// GenericCanRx
static void prv_rx_handler(const GenericCanMsg *msg, void *context) {
  (void)context;
  // TODO(ELEC-355): Rebroadcast this message after transform to the car.

  const ChargerControllerRxData data = {
    .raw_data = msg->data,
  };
  *s_charger_status = data.data_impl.status_flags;

  // Check for statuses
  if (data.data_impl.status_flags.raw) {
    if (data.data_impl.status_flags.starting_state || data.data_impl.status_flags.input_voltage) {
      // TODO(ELEC-355): Raise event to stop charging since this is a safety issue.
      charger_set_state(CHARGER_STATE_STOP)
    }
    // The other error types indicate issues with the communicating or interacting with the charger.
    // Continue to try.
  }
}

StatusCode charger_init(ChargerSettings *settings, ChargerStatus *status) {
  // Explicit for readability.
  static const J1939CanId tx_id = {
    .source_address = 0xF4,  // Indicates BMS in the J1939 standard.
    .pdu_specifics = 0xE5,   // Charger control system (CCS) id.
    .pdu_format = 0x06,      // From datasheet.
    .dp = 0,
    .r = 0,
    .priority = 0x06,  // From datasheet.
  };
  static_assert(CHARGER_EXPECTED_TX_ID == tx_id.raw_id);

  const ChargerControllerTxData tx_data = { .data_impl = {
                                                .max_voltage = settings->max_voltage,
                                                .max_current = settings->max_current,
                                                .charging = CHARGER_STATE_STOP,
                                            } };

  s_tx_msg.id = tx_id.raw_id;
  s_tx_msg.dlc = CHARGER_EXPECTED_TX_DLC;
  s_tx_msg.data = tx_data.raw_data;
  s_tx_msg.extended = true;

  status_ok_or_return(
      can_interval_factory(settings->can_uart, &s_tx_msg, CHARGER_PERIOD_US, s_interval));

  status_ok_or_return(
      generic_can_register_rx(settings->can_uart, prv_rx_handler, s_rx_id.raw_id, settings->can));
  status_ok_or_return(can_interval_enable(s_interval));
  return STATUS_CODE_OK;
}

StatusCode charger_set_state(ChargerState state) {
  if (state >= NUM_CHARGER_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  // TODO(ELEC-355): Consider disabling rx rebroadcasts (1/s).

  ChargerControllerTxData tx_data = {
    .raw_data = s_tx_msg.data,
  };

  tx_data.data_impl.charging = state;
  s_tx_msg.data = tx_data.raw_data;
  return can_interval_send_now(s_interval);
}

#include "charger_controller.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "can_interval.h"
#include "can_transmit.h"
#include "charger_can.h"
#include "charger_events.h"
#include "event_queue.h"
#include "generic_can.h"
#include "generic_can_msg.h"
#include "log.h"
#include "misc.h"
#include "status.h"

#define CHARGER_PERIOD_US 1000000  // 1 Second as defined in datasheet.

#define CHARGER_EXPECTED_RX_DLC 8
#define CHARGER_EXPECTED_TX_DLC 8

static ChargerStorage *s_storage;
static CanInterval *s_interval;
static ChargerCanStatus *s_charger_status;

// Explicit for readability.
static const ChargerCanJ1939Id s_rx_id = {
  .source_address = 0xE5,  // Indicates BMS in the J1939 standard.
  .pdu_specifics = 0x50,   // Broadcast address (BCA) id.
  .pdu_format = 0xFF,      // From datasheet.
  .dp = 0,
  .r = 0,
  .priority = 0x06,  // From datasheet.
};
static const ChargerCanJ1939Id s_tx_id = {
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

  const ChargerCanRxData data = {
    .raw_data = msg->data,
  };
  *s_charger_status = data.data_impl.status_flags;
  LOG_DEBUG("Current: %u\n", SWAP_UINT16(data.data_impl.current));
  LOG_DEBUG("Voltage: %u\n", SWAP_UINT16(data.data_impl.voltage));
  // LOG_DEBUG("Comms: %u\n", data.data_impl.status_flags.comms_state);
  // LOG_DEBUG("HW Fault: %u\n", data.data_impl.status_flags.hw_fault);
  // LOG_DEBUG("Input voltage: %u\n", data.data_impl.status_flags.input_voltage);
  // LOG_DEBUG("Over temp: %u\n", data.data_impl.status_flags.over_temp);
  // LOG_DEBUG("Starting state: %u\n", data.data_impl.status_flags.starting_state);

  // Check for statuses
  if (!charger_controller_is_safe()) {
    // If unsafe immediately stop charging and force the FSM to transition to the unsafe state.
    charger_controller_set_state(CHARGER_STATE_STOP);
    event_raise(CHARGER_EVENT_STOP_CHARGING, 0);
  }
}

StatusCode charger_controller_init(ChargerStorage *storage, const ChargerSettings *settings,
                                   ChargerCanStatus *status) {
  if (status == NULL || settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_charger_status = status;
  s_storage = storage;
  memcpy(&s_storage->relay_control_pin, &settings->relay_control_pin, sizeof(GPIOAddress));

  const GPIOSettings gpio_settings = {
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .direction = GPIO_DIR_OUT,
    .resistor = GPIO_RES_NONE,
  };
  status_ok_or_return(gpio_init_pin(&settings->relay_control_pin, &gpio_settings));

  const ChargerCanTxData tx_data = { .data_impl = {
                                         .max_voltage = settings->max_voltage,
                                         .max_current = settings->max_current,
                                         .charging = CHARGER_STATE_STOP,
                                     } };
  GenericCanMsg tx_msg = {
    .id = s_tx_id.raw_id,
    .dlc = CHARGER_EXPECTED_TX_DLC,
    .data = tx_data.raw_data,
    .extended = true,
  };

  status_ok_or_return(
      can_interval_factory(settings->can_uart, &tx_msg, CHARGER_PERIOD_US, &s_interval));

  status_ok_or_return(generic_can_register_rx(settings->can_uart, prv_rx_handler,
                                              GENERIC_CAN_EMPTY_MASK, s_rx_id.raw_id, true,
                                              settings->can));
  return STATUS_CODE_OK;
}

StatusCode charger_controller_set_state(ChargerCanState state) {
  if (state >= NUM_CHARGER_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_charger_status == NULL || !charger_controller_is_safe()) {
    // Unsafe to start
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  ChargerCanTxData tx_data = {
    .raw_data = s_interval->msg.data,
  };

  tx_data.data_impl.charging = state ? CHARGER_STATE_STOP : CHARGER_STATE_START;
  s_interval->msg.data = tx_data.raw_data;

  if (state == CHARGER_STATE_START) {
    gpio_set_state(&s_storage->relay_control_pin, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&s_storage->relay_control_pin, GPIO_STATE_LOW);
  }

  if (state == CHARGER_STATE_OFF) {
    status_ok_or_return(can_interval_send_now(s_interval));
    return can_interval_disable(s_interval);
  }
  return can_interval_send_now(s_interval);
}

bool charger_controller_is_safe(void) {
  // - Input voltage is an indication the charger is lower voltage the battery so don't charge.
  // - Over Temp is an indication of a heat issue with the charger, it is possible to resume
  //   charging after a break.
  // - HW fault is an indeterminate hardware failure which should be addressed.
  //
  // Implicitly forgives communication issues.
  return !(s_charger_status->input_voltage || s_charger_status->over_temp ||
           s_charger_status->hw_fault);
}

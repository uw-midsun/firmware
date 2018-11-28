#include "charger.h"
#include "misc.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "delay.h"
#include "log.h"
#include "soft_timer.h"
#include <string.h>

typedef struct ChargerStorage {
  uint16_t max_voltage;
  uint16_t max_current;
  bool charging;
  ChargerSettings settings;
} ChargerStorage;

static ChargerStorage s_charger;

// Explicit for readability.
static const ChargerCanJ1939Id s_rx_id = {
  .source_address = 0xE5,  // Indicates CCS in the J1939 standard.
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

static void prv_handle_info_cb(const GenericCanMsg *msg, void *context) {
  const ChargerCanRxData data = {
    .raw_data = msg->data,
  };

  if (data.data_impl.status_flags.hw_fault || data.data_impl.status_flags.over_temp || data.data_impl.status_flags.input_voltage) {
    LOG_DEBUG("Charger HW Fault 0x%x! Stopping charger.\n", data.data_impl.status_flags.raw);
    charger_stop();
  }

  if (s_charger.settings.info_cb != NULL) {
    s_charger.settings.info_cb(SWAP_UINT16(data.data_impl.voltage), SWAP_UINT16(data.data_impl.current), data.data_impl.status_flags, s_charger.settings.context);
  }
}

static void prv_periodic_cmd(SoftTimerId timer_id, void *context) {
  const ChargerCanTxData tx_data = {
    .data_impl = {
      .max_voltage = SWAP_UINT16(s_charger.max_voltage),
      .max_current = SWAP_UINT16(s_charger.max_current),
      .charging = s_charger.charging,
    },
  };

  GenericCanMsg tx_msg = {
    .id = s_tx_id.raw_id,
    .dlc = sizeof(s_tx_id.raw_id),
    .data = tx_data.raw_data,
    .extended = true,
  };
  generic_can_tx(s_charger.settings.charger_can, &tx_msg);

  soft_timer_start_millis(CHARGER_BROADCAST_PERIOD_MS, prv_periodic_cmd, NULL, NULL);
}

static void prv_delay_start(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("Beginning charge\n");
  s_charger.charging = true;
}

static void prv_delay_relay_slave(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("Closing slave relay\n");
  CAN_TRANSMIT_BATTERY_RELAY_SLAVE(NULL, EE_RELAY_STATE_CLOSE);
  soft_timer_start_millis(2000, prv_delay_start, NULL, NULL);
}

StatusCode charger_init(ChargerSettings *settings) {
  memset(&s_charger, 0, sizeof(s_charger));
  s_charger.settings = *settings;

  generic_can_register_rx(settings->charger_can, prv_handle_info_cb, GENERIC_CAN_EMPTY_MASK, s_rx_id.raw_id, true, NULL);
  soft_timer_start_millis(CHARGER_BROADCAST_PERIOD_MS, prv_periodic_cmd, NULL, NULL);

  return STATUS_CODE_OK;
}

StatusCode charger_start(uint16_t voltage, uint16_t current) {
  // Ask relays to close and wait
  LOG_DEBUG("Closing main relay\n");
  CAN_TRANSMIT_BATTERY_RELAY_MAIN(NULL, EE_RELAY_STATE_CLOSE);
  soft_timer_start_millis(2000, prv_delay_relay_slave, NULL, NULL);

  s_charger.max_voltage = voltage;
  s_charger.max_current = current;

  return STATUS_CODE_OK;
}

StatusCode charger_stop(void) {
  LOG_DEBUG("Opening both relays\n");
  CAN_TRANSMIT_BATTERY_RELAY_MAIN(NULL, EE_RELAY_STATE_OPEN);
  CAN_TRANSMIT_BATTERY_RELAY_SLAVE(NULL, EE_RELAY_STATE_OPEN);

  s_charger.charging = false;

  return STATUS_CODE_OK;
}

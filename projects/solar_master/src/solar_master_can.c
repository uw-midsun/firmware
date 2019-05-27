#include "solar_master_can.h"

static StatusCode prv_rx_handler(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // Storage for extracting message data.
  uint8_t relay_state = 0;
  switch (msg->msg_id) {
    case SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR:
      status_ok_or_return(CAN_UNPACK_SOLAR_RELAY_REAR(msg, &relay_state));
      break;
    case SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT:
      status_ok_or_return(CAN_UNPACK_SOLAR_RELAY_FRONT(msg, &relay_state));
      break;
  }
  return event_raise(SOLAR_MASTER_EVENT_RELAY_STATE, relay_state);
}

static void prv_periodic_tx_telemetry(SoftTimerId timer_id, void *context) {
  SolarMasterCanStorage *storage = context;
  uint16_t current = (uint16_t)(
      ((uint32_t)storage->current_storage->sliding_sum -
       (SOLAR_MASTER_CURRENT_SAMPLE_SIZE * (uint32_t)storage->current_storage->zero_point)) *
      1000 / (SOLAR_MASTER_CURRENT_GRADIENT * SOLAR_MASTER_CURRENT_SAMPLE_SIZE));
  uint16_t module_id = 0;
  uint16_t voltage = 0;
  uint16_t temp = 0;

  uint32_t temp_resistance_ohms = 0;
  for (int i = 0; i < SOLAR_MASTER_NUM_SOLAR_SLAVES; i++) {
    if (storage->slave_storage[i].is_stale) {
      continue;
    }
    voltage = (uint16_t)((uint32_t)storage->slave_storage[i].sliding_sum_voltage_mv *
                         SOLAR_MASTER_VOLTAGE_SCALING_FACTOR / SOLAR_MASTER_MCP3427_SAMPLE_SIZE);

    // Voltage dividor:
    // 5V --> thermistor --> Measurement (mv) --> 10k Ohms --> GND
    // R = (5 V) * (10000 Ohms) * 1000 / (Avg Measurement mV)
    // temp_resistance_ohms = ((SOLAR_MASTER_TEMP_VOLTAGE * SOLAR_MASTER_TEMP_RESISTOR) /
    //                         ((uint32_t)storage->slave_storage[i].sliding_sum_temp_mv /
    //                          SOLAR_MASTER_MCP3427_SAMPLE_SIZE)) -
    //                        SOLAR_MASTER_TEMP_RESISTOR;

    // StatusCode sc = thermistor_calculate_temp(temp_resistance_ohms, &temp);

    temp = (uint16_t)((uint32_t)storage->slave_storage[i].sliding_sum_temp_mv / SOLAR_MASTER_MCP3427_SAMPLE_SIZE);


    module_id = i;
    LOG_DEBUG("module: %i, voltage: %i, current: %i, temp: %i\n", module_id, voltage, current,
              temp);
    if (storage->board == SOLAR_MASTER_CONFIG_BOARD_FRONT) {
      CAN_TRANSMIT_SOLAR_DATA_FRONT(module_id, voltage, current, temp);
    } else {
      CAN_TRANSMIT_SOLAR_DATA_REAR(module_id, voltage, current, temp);
    }
    storage->slave_storage[i].is_stale = true;
  }
  soft_timer_start_millis(SOLAR_MASTER_TELEMETRY_PERIOD_MS, prv_periodic_tx_telemetry, storage,
                          NULL);
}

StatusCode solar_master_can_init(SolarMasterCanStorage *storage, const CanSettings *can_settings,
                                 SolarMasterConfigBoard board) {
  status_ok_or_return(can_init(&storage->can_storage, can_settings));
  storage->board = board;
  // Specify filters. Since we add filters, the handlers don't need to care about board type.
  status_ok_or_return(can_add_filter((storage->board == SOLAR_MASTER_CONFIG_BOARD_FRONT)
                                         ? SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT
                                         : SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR));
  // Set up RX handlers.
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_REAR, prv_rx_handler, NULL));
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_RELAY_FRONT, prv_rx_handler, NULL));

  soft_timer_start_millis(SOLAR_MASTER_TELEMETRY_PERIOD_MS, prv_periodic_tx_telemetry, storage,
                          NULL);
  return STATUS_CODE_OK;
}

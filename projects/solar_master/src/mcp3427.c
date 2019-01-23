#include "mcp3427.h"
#include "fsm.h"
#include "log.h"
#include "mcp3427defs.h"
#include "soft_timer.h"

#define MCP3427_FSM_NAME "MCP3427 FSM"
#define MCP3427_MAX_CONV_TIME_MS 200

FSM_DECLARE_STATE(channel_1_trigger);
FSM_DECLARE_STATE(channel_1_readback);
FSM_DECLARE_STATE(channel_2_trigger);
FSM_DECLARE_STATE(channel_2_readback);

FSM_STATE_TRANSITION(channel_1_trigger) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_ready_event, channel_1_readback);
}

FSM_STATE_TRANSITION(channel_1_readback) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_trigger_event, channel_2_trigger);
}

FSM_STATE_TRANSITION(channel_2_trigger) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_ready_event, channel_2_readback);
}

FSM_STATE_TRANSITION(channel_2_readback) {
  Mcp3427Storage *storage = fsm->context;
  FSM_ADD_TRANSITION(storage->data_trigger_event, channel_1_trigger);
}

static void prv_raise_ready(SoftTimerID timer_id, void *context) {
  Mcp3427Storage *storage = (Mcp3427Storage *)context;
  event_raise(storage->data_ready_event, 0);
}

static uint16_t s_data_mask_lookup[] = {
  [MCP3427_SAMPLE_RATE_12_BIT] = MCP3427_DATA_MASK_12_BIT,  //
  [MCP3427_SAMPLE_RATE_14_BIT] = MCP3427_DATA_MASK_14_BIT,  //
  [MCP3427_SAMPLE_RATE_16_BIT] = MCP3427_DATA_MASK_16_BIT,  //
};

static void prv_channel_ready(struct FSM *fsm, const Event *e, void *context) {
  Mcp3427Storage *storage = (Mcp3427Storage *)context;
  uint8_t read_data[MCP3427_NUM_DATA_BYTES] = { 0 };
  StatusCode status = i2c_read(storage->port, storage->addr, read_data, MCP3427_NUM_DATA_BYTES);
  // The first byte is the config/status byte. It contains the ready bit.
  uint8_t config = read_data[0];
  // If the latest data is not ready, we log it.
  if (!(config & MCP3427_RDY_MASK)) {
    LOG_WARN("Ready bit not cleared. Data may not be the latest data available.");
    if (storage->fault_callback != NULL) {
      storage->fault_callback(storage->fault_context);
    }
    event_raise(storage->data_trigger_event, 0);
    return;
  }
  // The second and third byte contain latest ADC value.
  uint16_t sensor_data = read_data[2] | (read_data[1] << 8);
  sensor_data |= s_data_mask_lookup[storage->sample_rate];

  uint8_t current_channel =
      (storage->config & (1 << MCP3427_CH_SEL_OFFSET)) >> MCP3427_CH_SEL_OFFSET;

  storage->sensor_data[current_channel] = sensor_data;

  if (current_channel == MCP3427_CHANNEL_2 && storage->callback != NULL) {
    // We have all the data ready.
    uint32_t data = (uint32_t)storage->sensor_data[0] << 16 | storage->sensor_data[1];
    storage->callback(data, storage->context);
  }

  event_raise(storage->data_trigger_event, 0);
}

// Trigger data read. Schedule a data ready event to be raised.
static void prv_channel_trigger(struct FSM *fsm, const Event *e, void *context) {
  Mcp3427Storage *storage = (Mcp3427Storage *)context;
  // We want to trigger a read. So we set the ready bit.
  uint8_t config = storage->config;
  config |= MCP3427_RDY_MASK;
  // Setting the current channel we want to read from. We just flip it from the
  // previous read.
  config ^= (1 << MCP3427_CH_SEL_OFFSET);
  storage->config = config;

  i2c_write(storage->port, storage->addr, &config, MCP3427_NUM_CONFIG_BYTES);
  soft_timer_start_millis(MCP3427_MAX_CONV_TIME_MS, prv_raise_ready, storage, NULL);
}

// Lookup table for selected address. (TODO: manual tbl)
static uint8_t s_addr_lookup[NUM_MCP3427_PIN_STATES][NUM_MCP3427_PIN_STATES] = {
  { 0x0, 0x1, 0x2 },
  { 0x3, 0x0, 0x7 },
  { 0x4, 0x5, 0x6 },
};

StatusCode mcp3427_init(Mcp3427Storage *storage, Mcp3427Setting *setting) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  fsm_state_init(channel_1_trigger, prv_channel_trigger);
  fsm_state_init(channel_1_readback, prv_channel_ready);
  fsm_state_init(channel_2_trigger, prv_channel_trigger);
  fsm_state_init(channel_2_readback, prv_channel_ready);
  storage->port = setting->port;
  storage->addr = s_addr_lookup[setting->Adr0][setting->Adr1] | (MCP3427_DEVICE_CODE << 3);
  // Writing configuration to the chip (see section 5.3.3 of manual).
  uint8_t config = 0;
  // Note: Here, channel gets defaulted to 0.
  config |= (setting->conversion_mode << MCP3427_CONVERSION_MODE_OFFSET);
  config |= (setting->sample_rate << MCP3427_SAMPLE_RATE_OFFSET);
  config |= (setting->amplifier_gain << MCP3427_GAIN_SEL_OFFSET);
  storage->config = config;

  return i2c_write(storage->port, storage->addr, &config, MCP3427_NUM_CONFIG_BYTES);
}

StatusCode mcp3427_register_callback(Mcp3427Storage *storage, Mcp3427Callback callback,
                                     void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->callback = callback;
  storage->context = context;
  // Starting the state machine.
  fsm_init(&storage->fsm, MCP3427_FSM_NAME, &channel_1_trigger, storage);
  return STATUS_CODE_OK;
}

StatusCode mcp3427_register_fault_callback(Mcp3427Storage *storage, Mcp3427FaultCallback callback,
                                           void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->fault_callback = callback;
  storage->fault_context = context;
  return STATUS_CODE_OK;
}

#include "mcp3427.h"
#include "mcp3427defs.h"

#define MCP_3427_MAX_CONV_TIME 


// Lookup table for selected address. (TODO: manual tbl)
static s_addr_lookup[NUM_MCP3427_PIN_STATES][NUM_MCP3427_PIN_STATES] = {
  { 0x0, 0x1, 0x2 },
  { 0x3, 0x0, 0x7 },
  { 0x4, 0x5, 0x6 },
}



StatusCode mcp3427_init(Mcp3427Storage *storage, Mcp3427Setting *setting) {
  storage->port = setting->port;
  storage->addr = s_addr_lookup[setting->Adr0][setting->Adr1] | (MCP3427_DEVICE_CODE << 3);
  // Writing configuration to the chip (see section 5.3.3 of manual).
  uint8_t config = 0;
  config |= (setting->conversion_mode << MCP3427_CONVERSION_MODE_OFFSET);
  config |= (setting->sample_rate << MCP3427_SAMPLE_RATE_OFFSET);
  config |= (setting->amplifier_gain << MCP3427_GAIN_SEL_OFFSET);
  storage->config = config;

  soft_timer_start_millis(MCP3427_MAX_CONVERSION_TIME_MS, )

  return i2c_write(storage->port, storage->addr, &config, MCP3427_NUM_CONFIG_BYTES);
}

// Register a callback to be run whenever there is new data.
StatusCode mcp3427_register_callback(Mcp3427Storage *storage, Mcp3427Channel channel, Mcp3427Callback callback, void *context) {
  storage->callbacks[channel] = callback;
  storage->contexts[channel] = context;
}

StatusCode mcp3427_register_fault_callback(Mcp3427Storage *storage, Mcp3427FaultCallback callback, void *context) {
  storage->fault_callback = callback;
}

StatusCode prv_wait_till_ready(Mcp3427Storage *storage) {
  uint8_t data[MCP3427_NUM_DATA_BYTES] = 0;
  StatusCode status;
  bool ready = 0;
  do {
    status = i2c_read(storage->port, storage->addr, &data, MCP3427_NUM_DATA_BYTES);
    ready = ((data[2] & MCP3427_RDY_MASK) == 0);
  } while(status_ok(status) && (!ready));
  if (ready) {
    return STATUS_CODE_OK;
  } else {
    
  }
}

StatusCode mcp3427_read(Mcp3427Storage *storage, Mcp3427Channel channel, uint16_t *data) {
  uint8_t config = storage->config;
  config |= (channel << MCP3427_CH_SEL_OFFSET);
  i2c_write(storage->port, storage->addr, &config, MCP3427_NUM_CONFIG_BYTES);


}



#include "mcp3427.h"
#include "mcp3427defs.h"

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
  return i2c_write(storage->port, storage->addr, &config, MCP3427_NUM_CONFIG_BYTES);
}

static prv_wait_till_ready(Mcp3427Storage *storage) {
  uint8_t data[MCP3427_NUM_DATA_BYTES];

}


StatusCode mcp3427_read(Mcp3427Storage *storage, Mcp3427Channel channel, uint16_t *data) {
  uint8_t config = storage->config;
  config |= (channel << MCP3427_CH_SEL_OFFSET);
  i2c_write(storage->port, storage->addr, &config, MCP3427_NUM_CONFIG_BYTES);


}



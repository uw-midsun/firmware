#include "solar_master_slave.h"

static void prv_voltage_reading_cb(uint32_t value, void *context) {
  SolarMasterSlave *slave = context;

  slave->sliding_sum_voltage_mv -= slave->averaging_voltage[slave->counter];
  slave->sliding_sum_temp_mv -= slave->averaging_temp[slave->counter];
  slave->averaging_voltage[slave->counter] = (int32_t)value >> 16;
  slave->averaging_temp[slave->counter] = (int32_t)(value & ((1 << 16) - 1));
  slave->sliding_sum_voltage_mv += slave->averaging_voltage[slave->counter];
  slave->sliding_sum_temp_mv += slave->averaging_temp[slave->counter++];
  if (slave->counter == SOLAR_MASTER_MCP3427_SAMPLE_SIZE) {
    slave->counter = 0;
  }
}

StatusCode solar_master_slave_init(SolarMasterSlave *slave, Mcp3427Storage *mcp3427) {
  slave->mcp3427 = mcp3427;
  mcp3427_register_callback(mcp3427, (Mcp3427Callback)prv_voltage_reading_cb, slave);
  event_raise(mcp3427->data_trigger_event, mcp3427->addr ^ (MCP3427_DEVICE_CODE << 3));
  return STATUS_CODE_OK;
}

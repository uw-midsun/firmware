#include "solar_master_slave.h"

static void prv_voltage_reading_cb(uint32_t value, void *context) {
  SolarMasterSlave *slave = context;

  slave->sliding_sum_voltage -= slave->averaging_voltage[slave->counter];
  slave->sliding_sum_temp -= slave->averaging_temp[slave->counter];
  slave->averaging_voltage[slave->counter] = (int32_t)value >> 16;
  slave->averaging_temp[slave->counter] = (int32_t)(value && ((1 << 16) - 1));
  slave->sliding_sum_voltage += slave->averaging_voltage[slave->counter];
  slave->sliding_sum_temp += slave->averaging_temp[slave->counter];
  printf("WEE\n");
  if (slave->counter == SOLAR_MASTER_MCP3427_SAMPLE_SIZE) {
    slave->counter = 0;

    // DEBUGGGING
    int32_t averaged_voltage = 0;
    int32_t averaged_temp = 0;
    for (int i = 0; i < SOLAR_MASTER_MCP3427_SAMPLE_SIZE; i++) {
      averaged_voltage += slave->averaging_voltage[i];
      averaged_temp += slave->averaging_temp[i];
    }
    // In the case of 12 bit sample rate, the LSB in the measurements corresponds to 1 mV
    averaged_voltage =
        (averaged_voltage / SOLAR_MASTER_MCP3427_SAMPLE_SIZE) * SOLAR_MASTER_VOLTAGE_SCALING_FACTOR;
    averaged_temp =
        (averaged_temp / SOLAR_MASTER_MCP3427_SAMPLE_SIZE) * SOLAR_MASTER_TEMP_SCALING_FACTOR;
    printf("%p: Voltage: %i mV, Temperature: %i mV\n", (void *)slave, (int)averaged_voltage,
           (int)averaged_temp);
  }
}

StatusCode solar_master_slave_init(SolarMasterSlave *slave, Mcp3427Storage *mcp3427) {
  slave->mcp3427 = mcp3427;
  mcp3427_register_callback(mcp3427, (Mcp3427Callback)prv_voltage_reading_cb, slave);
  event_raise(mcp3427->data_ready_event, 0);
  return STATUS_CODE_OK;
}

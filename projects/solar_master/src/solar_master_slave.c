#include "solar_master_slave.h"


static void prv_voltage_reading_cb(uint32_t value, void *context) {
  SolarMasterSlave *slave = context;  

  slave->averaging_voltage[slave->counter_voltage++] = value;

  if(slave->counter_voltage == SOLAR_MASTER_VOLTAGE_SAMPLE_SIZE) {
    uint32_t averaged = 0;
    for (int i = 0; i < SOLAR_MASTER_VOLTAGE_SAMPLE_SIZE; i++) {
      averaged += slave->averaging_voltage[i];
    }
    averaged /= SOLAR_MASTER_VOLTAGE_SAMPLE_SIZE;

    LOG_DEBUG("Reading: %i\n Voltage: %i", averaged, averaged);
    // event_raise(SOLAR_MASTER_EVENT_CURRENT, reading);
    // maybe handle everything in this func?
  }
  
}

StatusCode solar_master_slave_init(SolarMasterSlave *slave, Mcp3427Storage *mcp3427) {
  slave->voltage_mcp3427 = mcp3427;

  mcp3427_register_callback(mcp3427, (Mcp3427Callback) prv_voltage_reading_cb, slave);
  return STATUS_CODE_OK;
}


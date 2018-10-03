#include "solar_master_slave.h"

/*
static void prv_analog_cb(Ads1015Channel channel, void *context) {
  SolarMasterCurrent *current = context;
  int16_t reading = 0;
  ads1015_read_converted(current->ads1015, channel, &reading);
  
  float current_measurement = (float) (reading - SOLAR_MASTER_CURRENT_INTERCEPT) / SOLAR_MASTER_CURRENT_GRADIENT;

  printf("Reading: %i\n Current: %f\n", reading, current_measurement);
  // LOG_DEBUG("Reading: %i\n Current: %f", reading, current_measurement);
  // event_raise(SOLAR_MASTER_EVENT_CURRENT, reading);
  // maybe handle everything in this func?
}

StatusCode solar_master_slave_init(SolarMasterSlave *slave, Mcp3427Storage *mcp3427) {
  slave->voltage_mcp3427 = mcp3427;

  //mcp3427_register_callback(slave, prv_analog_cb, void *context);
  return STATUS_CODE_OK;
}*/


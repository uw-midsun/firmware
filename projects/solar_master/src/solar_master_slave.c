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
  if (value != 0) {
    slave->is_stale = false;
  }
}

static void prv_periodic_slave_init(SoftTimerId timer_id, void *context) {
  SolarMasterSlave *slave_storage = context;
  bool failed_init = false;

  for (int i = 0; i < SOLAR_MASTER_NUM_SOLAR_SLAVES; i++) {
    if (slave_storage[i].is_init) {
      continue;
    }

    Mcp3427Setting temp_slave_settings = *slave_storage[i].mcp3427_settings;
    temp_slave_settings.Adr0 = ADC_ADDRESS_MAP[i][0];
    temp_slave_settings.Adr1 = ADC_ADDRESS_MAP[i][1];

    StatusCode status = mcp3427_init(slave_storage[i].mcp3427, &temp_slave_settings);
    if (!status_ok(status)) {
      LOG_DEBUG("Error initializing Solar Slave ADC with addressing pins %i, %i.\n",
                temp_slave_settings.Adr0, temp_slave_settings.Adr1);
      failed_init = true;
      continue;
    }

    slave_storage[i].is_init = true;
    mcp3427_register_callback(slave_storage[i].mcp3427, (Mcp3427Callback)prv_voltage_reading_cb,
                              &(slave_storage[i]));
    event_raise(slave_storage[i].mcp3427->data_trigger_event,
                slave_storage[i].mcp3427->addr ^ (MCP3427_DEVICE_CODE << 3));
  }
  if (failed_init) {
    soft_timer_start_millis(SOLAR_MASTER_SLAVE_REINIT_PERIOD_MS, prv_periodic_slave_init,
                            slave_storage, NULL);
  }
}

StatusCode solar_master_slave_init(SolarMasterSlave *slave_storage, Mcp3427Storage *mcp3427_storage,
                                   Mcp3427Setting *mcp3427_settings_base) {
  for (int i = 0; i < SOLAR_MASTER_NUM_SOLAR_SLAVES; i++) {
    slave_storage[i].mcp3427_settings = mcp3427_settings_base;
    slave_storage[i].mcp3427 = &(mcp3427_storage[i]);
    slave_storage[i].is_stale = true;
  }

  prv_periodic_slave_init(0, slave_storage);
  return STATUS_CODE_OK;
}

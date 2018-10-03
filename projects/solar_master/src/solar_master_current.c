#include "solar_master_current.h"


static void prv_current_read_cb(Ads1015Channel channel, void *context) {
  SolarMasterCurrent *current = context;
  int16_t reading = 0;
  ads1015_read_converted(current->ads1015, channel, &reading);

  current->averaging[current->counter++] = reading;
  if (current->counter == SOLAR_MASTER_CURRENT_SAMPLE_SIZE) {
    float current_measurement = 0;
    for (int i = 0; i < SOLAR_MASTER_CURRENT_SAMPLE_SIZE; i++) {
      current_measurement += current->averaging[i];
    }

    current_measurement =  ((current_measurement/SOLAR_MASTER_CURRENT_SAMPLE_SIZE) - SOLAR_MASTER_CURRENT_INTERCEPT) / SOLAR_MASTER_CURRENT_GRADIENT;

    LOG_DEBUG("Reading: %i Current: %i mA\n", reading, (int16_t)(current_measurement*1000));
    // event_raise(SOLAR_MASTER_EVENT_CURRENT, reading);
    // maybe handle everything in this func?
    current->counter = 0;
  }
}

StatusCode solar_master_current_init(SolarMasterCurrent *current, Ads1015Storage *ads1015) {
  current->ads1015 = ads1015;
  ads1015_configure_channel(current->ads1015, SOLAR_MASTER_CURRENT_CHANNEL, true, prv_current_read_cb, current);
  return STATUS_CODE_OK;
}


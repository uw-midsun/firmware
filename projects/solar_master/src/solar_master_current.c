#include "solar_master_current.h"

static void prv_current_read_cb(Ads1015Channel channel, void *context) {
  SolarMasterCurrent *current = context;
  int16_t reading = 0;
  ads1015_read_converted(current->ads1015, channel, &reading);

  current->sliding_sum -= current->averaging[current->counter];
  current->sliding_sum += reading;
  current->averaging[current->counter++] = reading;
  if (current->counter == SOLAR_MASTER_CURRENT_SAMPLE_SIZE) {
    current->counter = 0;

    if (current->zero_point == 0) {
      LOG_DEBUG("setting zero point...\n");
      uint16_t current_measurement = 0;
      for (int i = 0; i < SOLAR_MASTER_CURRENT_SAMPLE_SIZE; i++) {
        current_measurement += current->averaging[i];
      }
      current->zero_point = current_measurement / SOLAR_MASTER_CURRENT_SAMPLE_SIZE;
    }
    // DEBUGGING
    // current_measurement =  ((current_measurement/SOLAR_MASTER_CURRENT_SAMPLE_SIZE) -
    // current->zero_point) / SOLAR_MASTER_CURRENT_GRADIENT; printf("Reading: %i Current: %i mA\n",
    // reading, (int16_t)(current_measurement*1000));
  }
}

StatusCode solar_master_current_init(SolarMasterCurrent *current, Ads1015Storage *ads1015) {
  current->ads1015 = ads1015;
  ads1015_configure_channel(current->ads1015, SOLAR_MASTER_CURRENT_CHANNEL, true,
                            prv_current_read_cb, current);
  return STATUS_CODE_OK;
}

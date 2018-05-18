#include "ltc_adc_calibration.h"

#include <string.h>

#define LTC2484_OFFSET_MICROVOLTS   1000000
#define LTC2484_SHUNT_RESISTOR      100

// Use a running average to correct the value for resistance. Because resistance is directly
// proportional to voltage, we can observe errors in the measured value and apply the corrections
// to the known resisitance.
static void prv_running_average(LTCCalibrationStorage *storage) {
    // Add the latest measurement to the average buffer and update the average
    storage->buffer[storage->index] += (int32_t)((float)storage->value.voltage / LTC2484_AVERAGE_CAP);
    storage->average += storage->buffer[storage->index];

    // Remove the earliest measurement from the buffer
    storage->average -= storage->buffer[(storage->index + 1) % LTC2484_AVERAGE_WINDOW];
}

static void prv_callback(int32_t *value, void *context) {
  LTCCalibrationStorage *storage = (LTCCalibrationStorage *)context;

  // Correct for voltage offset
  storage->value.voltage = *value - LTC2484_OFFSET_MICROVOLTS;

  // Update average
  prv_running_average(storage, storage->value.voltage);

  // Calculate the percent error of the voltage reading and apply it to LTC2484_SHUNT_RESISTOR
  float error = (float)(voltage - storage->average) / storage->average;
  int32_t resistance = (int32_t)(LTC2484_SHUNT_RESISTOR * (1 + error));

  // Use calibrated voltage value to obtain current calibration_value
  storage->value.current = (int32_t)((float)storage->value.voltage /
                                      prv_resistance_corrected(storage->value.voltage));
}

StatusCode ltc_adc_calibration_init(LTCCalibrationStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // Initialize ADC and start periodic polling
  status_ok_or_return(ltc_adc_init(&(storage->storage)));
  status_ok_or_return(ltc_adc_register_callback(&(storage->storage), prv_callback, storage));

  memset (storage->buffer, 0, LTC2484_AVERAGE_WINDOW);
  storage->index = 0;
  storage->average = 0;

  return STATUS_CODE_OK;
}

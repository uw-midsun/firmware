#pragma once
// driver for LTC6804-1 AFE chip
// requires GPIO, Interrupts and Soft Timers to be initialized

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"

// select the ADC mode (trade-off between speed or minimizing noise)
// see p.50 for conversion times and p.23 for noise
typedef enum {
  LTC_AFE_ADC_MODE_27KHZ = 0,
  LTC_AFE_ADC_MODE_7KHZ,
  LTC_AFE_ADC_MODE_26HZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  NUM_LTC_AFE_ADC_MODE
} LTCAFEADCMode;

typedef struct LTCAFESettings {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  const SPIPort spi_port;
  uint32_t spi_baudrate;

  LTCAFEADCMode adc_mode;
} LTCAFESettings;

// initialize the LTC6804
StatusCode ltc_afe_init(const LTCAFESettings *afe);

// read all voltages
// result should be an array of size LTC6804_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN
StatusCode ltc_afe_read_all_voltage(const LTCAFESettings *afe, uint16_t *result, size_t len);

// read all auxiliary registers
// result should be an array of size LTC6804_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN
StatusCode ltc_afe_read_all_aux(const LTCAFESettings *afe, uint16_t *result);

// mark cells for discharging (takes effect after config is re-written)
StatusCode ltc_afe_toggle_discharge_cells(const LTCAFESettings *afe, uint16_t cell, bool discharge);

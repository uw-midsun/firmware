#pragma once

#include <stdint.h>
#include "cn_proc.h"
#include "status.h"
#include "spi.h"

typedef enum {
  LTC_AFE_ADC_MODE_27KHZ = 0,
  LTC_AFE_ADC_MODE_7KHZ,
  LTC_AFE_ADC_MODE_26HZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  LTC_AFE_ADC_MODE_NUM
} LtcAfeAdcMode;

typedef struct {
  const SPISettings *spi_settings;
  const SPIPort spi_port;
  LtcAfeAdcMode afe_mode;
} LtcAfeSettings;

// initialize the LTC6804
StatusCode LtcAfe_init(const LtcAfeSettings *afe);

// read all voltages
StatusCode LtcAfe_read_all_voltage(const LtcAfeSettings *afe);

// read all temperatures
StatusCode LtcAfe_read_all_temperature(const LtcAfeSettings *afe);

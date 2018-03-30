#pragma once
// LTC2484 ADC application interface
//
// Application code should only use functions in this header.
#include <stdint.h>

#include "gpio.h"
#include "spi.h"
#include "status.h"

typedef enum {
  LTC_ADC_FILTER_50HZ_60HZ = 0,
  LTC_ADC_FILTER_50HZ,
  LTC_ADC_FILTER_60HZ,
  NUM_LTC_ADC_FILTER_MODES
} LtcAdcFilterMode;

typedef struct {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  const SPIPort spi_port;
  uint32_t spi_baudrate;

  LtcAdcFilterMode filter_mode;
} LtcAdcSettings;

// Initializes the ADC by setting up the GPIO pins and configuring the ADC with
// the selected settings
StatusCode ltc_adc_init(const LtcAdcSettings *config);

// Get the last voltage reading (in uV) reported by the ADC
// The buffered value will be updated every 200ms
StatusCode ltc_adc_get_value(const LtcAdcSettings *config, int32_t *value);

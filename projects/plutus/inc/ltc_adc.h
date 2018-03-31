#pragma once
// LTC2484 ADC application interface
//
// Application code should only use functions in this header.
#include <stdint.h>

#include "gpio.h"
#include "soft_timer.h"
#include "spi.h"
#include "status.h"

typedef enum {
  LTC_ADC_FILTER_50HZ_60HZ = 0,
  LTC_ADC_FILTER_50HZ,
  LTC_ADC_FILTER_60HZ,
  NUM_LTC_ADC_FILTER_MODES
} LtcAdcFilterMode;

typedef struct {
  StatusCode status;
  int32_t value;

  SoftTimerID timer_id;
} LtcAdcStorageBuffer;

typedef struct {
  // Storage buffer managed by the driver
  LtcAdcStorageBuffer buffer;

  const GPIOAddress cs;
  const GPIOAddress mosi;
  const GPIOAddress miso;
  const GPIOAddress sclk;

  const SPIPort spi_port;
  const uint32_t spi_baudrate;

  const LtcAdcFilterMode filter_mode;
} LtcAdcStorage;

// Initializes the ADC by setting up the GPIO pins and configuring the ADC with
// the selected settings
StatusCode ltc_adc_init(LtcAdcStorage *storage);

// Get the last voltage reading (in uV) reported by the ADC
// The buffered value will be updated every 200ms
StatusCode ltc_adc_get_value(LtcAdcStorage *storage, int32_t *value);

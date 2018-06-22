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

typedef void (*LtcAdcCallback)(int32_t *value, void *context);

typedef struct {
  // Storage buffer managed by the driver
  LtcAdcStorageBuffer buffer;
  // Callback that is run whenever new data is available
  LtcAdcCallback callback;
  void *context;

  GPIOAddress cs;
  GPIOAddress miso;
  SPIPort spi_port;
} LtcAdcStorage;

typedef struct LtcAdcSettings {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  SPIPort spi_port;
  uint32_t spi_baudrate;

  LtcAdcFilterMode filter_mode;
} LtcAdcSettings;

// Initializes the ADC by setting up the GPIO pins and configuring the ADC with
// the selected settings
StatusCode ltc_adc_init(LtcAdcStorage *storage, const LtcAdcSettings *settings);

// Register a callback to be run whenever there is new data
StatusCode ltc_adc_register_callback(LtcAdcStorage *storage, LtcAdcCallback callback,
                                     void *context);

// Exposed for testing
StatusCode test_ltc_adc_set_input_voltage(int32_t input_voltage);

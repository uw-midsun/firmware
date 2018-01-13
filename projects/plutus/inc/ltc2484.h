#pragma once

#include <stdint.h>

#include "gpio.h"
#include "spi.h"
#include "status.h"

// commands
// see datasheet Table 1 (p. 14) for these command values
#define LTC2484_ENABLE (1 << 7)
#define LTC2484_DISABLE (0 << 7)

#define LTC2484_EXTERNAL_INPUT (0 << 3)
#define LTC2484_TEMPERATURE_INPUT (1 << 3)

#define LTC2484_REJECTION_50HZ_60HZ 0x00
#define LTC2484_REJECTION_50HZ 0x02
#define LTC2484_REJECTION_60HZ 0x04

#define LTC2484_AUTO_CALIBRATION (0 << 0)
#define LTC2484_SPEED_2X (1 << 0)

// constants
#define LTC2484_CONVERSION_CYCLES 5520
#define LTC2484_TIMEOUT_CYCLES (LTC2484_CONVERSION_CYCLES * 3)

#define LTC2484_OVERRANGE_CODE ((1 << 5) | (1 << 4))
#define LTC2484_UNDERRANGE_CODE ((1 << 1) | (1 << 2) | (1 << 3))
// We mask to check bits EOC, DMY, SIG, MSB, B27, B26, B25
// (see Table 3 on p.16)
#define LTC2484_ERROR_CODE_MASK 0xFE
#define LTC2484_V_REF_MILLIVOLTS 4092

typedef union {
  uint8_t u8data[4];
  int32_t i32data;
} Ltc2484Response;

typedef enum {
  LTC_2484_FILTER_50HZ_60HZ = 0,
  LTC_2484_FILTER_50HZ,
  LTC_2484_FILTER_60HZ,
  NUM_LTC_2484_FILTER_MODES
} Ltc2484FilterMode;

typedef struct {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  const SPIPort spi_port;
  uint32_t spi_baudrate;

  Ltc2484FilterMode filter_mode;
} Ltc2484Settings;

// Initializes the ADC by setting up the GPIO pins and configuring the ADC with
// the selected settings
StatusCode ltc2484_init(const Ltc2484Settings *config);

// Read the voltage (in uV) reported by the ADC
StatusCode ltc2484_read(const Ltc2484Settings *config, int32_t *value);

// Parse the raw 24-bit ADC reading to a voltage measurement in uV. This is
// called by ltc2484_read, and is exposed for testing
StatusCode ltc2484_raw_adc_to_uv(uint8_t *spi_data, int32_t *voltage);

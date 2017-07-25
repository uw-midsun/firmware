#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"

// number of devices in daisy chain (including master)
#define LTC_AFE_DEVICES_IN_CHAIN                1

#define LTC_AFE_CELLS_IN_REG                    3
#define LTC_AFE_GPIOS_IN_REG                    3
#define LTC_CELLS_PER_DEVICE                    12

typedef enum {
  LTC_AFE_DISCHARGE_TIMEOUT_DISABLED = 0,
  LTC_AFE_DISCHARGE_TIMEOUT_30_S,
  LTC_AFE_DISCHARGE_TIMEOUT_1_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_2_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_3_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_4_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_5_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_10_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_15_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_20_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_30_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_40_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_60_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_75_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_90_MIN,
  LTC_AFE_DISCHARGE_TIMEOUT_120_MIN
} LTCAFEDischargeTimeout;

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

  LTCAFEADCMode adc_mode;
} LTCAFESettings;

// initialize the LTC6804
StatusCode ltc_afe_init(const LTCAFESettings *afe);

// read all voltages
// result should be an array of size LTC_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN
StatusCode ltc_afe_read_all_voltage(const LTCAFESettings *afe, uint16_t *result);

// read all auxilary registers
// result should be an array of size LTC_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN
StatusCode ltc_afe_read_all_aux(const LTCAFESettings *afe, uint16_t *result);

// mark cells for discharging (takes effect after config is re-written)
StatusCode ltc_afe_toggle_discharge_cells(const LTCAFESettings *afe, uint16_t cell, bool discharge);

// read config registers
// config should be an array of size 6 * LTC_AFE_DEVICES_IN_CHAIN
StatusCode ltc_afe_read_config(const LTCAFESettings *afe, uint8_t *config);

// write config registers
StatusCode ltc_afe_write_config(const LTCAFESettings *afe, uint8_t gpio_enable_pins);

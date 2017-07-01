#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "status.h"
#include "spi.h"

#define LTC_AFE_CELL_IN_REG 6

// used to allocate static memory
#define LTC_DEVICES_IN_CHAIN 3

#define LTC6804_ADCOPT        (1 << 0)
#define LTC6804_SWTRD         (1 << 1)
#define LTC6804_GPIO1         (1 << 3)
#define LTC6804_GPIO2         (1 << 4)
#define LTC6804_GPIO3         (1 << 5)
#define LTC6804_GPIO4         (1 << 6)
#define LTC6804_GPIO5         (1 << 7)

#define LTC6804_CNVT_CELL_ALL   0
#define LTC6804_CNVT_CELL_1_7   1
#define LTC6804_CNVT_CELL_2_8   2
#define LTC6804_CNVT_CELL_3_9   3
#define LTC6804_CNVT_CELL_4_10  4
#define LTC6804_CNVT_CELL_5_11  5
#define LTC6804_CNVT_CELL_6_12  6

#define LTC6804_ADCV_DISCHARGE_PERMITTED     (1 << 4)
#define LTC6804_ADCV_RESERVED ((1 << 6) | (1 << 5) | (1 << 9))

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
} LtcDischargeTimeout;


typedef enum {
  LTC_AFE_ADC_MODE_27KHZ = 0,
  LTC_AFE_ADC_MODE_7KHZ,
  LTC_AFE_ADC_MODE_26HZ,
  LTC_AFE_ADC_MODE_14KHZ,
  LTC_AFE_ADC_MODE_3KHZ,
  LTC_AFE_ADC_MODE_2KHZ,
  LTC_AFE_ADC_MODE_NUM
} LtcAfeAdcMode;

typedef struct LtcAfeSettings {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  const SPIPort spi_port;

  LtcAfeAdcMode adc_mode;
  uint8_t devices_in_chain;
} LtcAfeSettings;

// initialize the LTC6804
StatusCode LtcAfe_init(const LtcAfeSettings *afe);

// read all voltages
StatusCode LtcAfe_read_all_voltage(const LtcAfeSettings *afe, uint16_t *result);

// read all auxilary registers
StatusCode LtcAfe_read_all_aux(const LtcAfeSettings *afe);

// discharge cells
StatusCode LtcAfe_toggle_discharge_cells(const LtcAfeSettings *afe, uint8_t device,
                                          uint8_t cell, bool discharge);

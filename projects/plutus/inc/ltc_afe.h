#pragma once
// Driver for LTC6804-1 AFE chip
// Assumes that:
// - a 16 channel analog MUX is attached to the GPIO outputs
// - GPIO2, GPIO3, GPIO4, GPIO5 are used as AUX channel select outputs
// - GPIO1 is used as a thermistor input
// Requires GPIO, Interrupts and Soft Timers to be initialized
//
// Note that all units are in 100uV.
//
// This module supports AFEs with fewer than 12 cells using the |input_bitset|.
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"
#include "plutus_cfg.h"

#define LTC_AFE_MAX_CELLS_PER_DEVICE 12

#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

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
} LtcAfeAdcMode;

typedef struct LtcAfeSettings {
  GPIOAddress cs;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;

  const SPIPort spi_port;
  uint32_t spi_baudrate;

  LtcAfeAdcMode adc_mode;
  // Cell inputs to include in the measurement arrays
  uint16_t input_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
} LtcAfeSettings;

typedef struct LtcAfeStorage {
  SPIPort spi_port;
  GPIOAddress cs;
  LtcAfeAdcMode adc_mode;

  // Cell inputs to include in the measurement arrays
  uint16_t input_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
  // Precalculate the offset so we can easily insert it into the result array
  // Note that this offset should be subtracted from the actual index
  uint16_t index_offset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN * LTC_AFE_MAX_CELLS_PER_DEVICE];
  // Discharge enabled - device-relative
  uint16_t discharge_bitset[PLUTUS_CFG_AFE_DEVICES_IN_CHAIN];
} LtcAfeStorage;

// Initialize the LTC6804.
// |settings.input_bitset| should be an array of bitsets where bits 0 to 11 represent whether
// we should monitor the cell input for the given device.
StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings);

// Read all cell voltages (in 100uV)
// |result_arr| is an array of size PLUTUS_CFG_TOTAL_CELLS
StatusCode ltc_afe_read_all_voltage(LtcAfeStorage *afe, uint16_t *result_arr, size_t len);

// Read all auxiliary voltages (in 100uV)
// |result_arr| should be an array of size PLUTUS_CFG_TOTAL_CELLS
StatusCode ltc_afe_read_all_aux(LtcAfeStorage *afe, uint16_t *result_arr, size_t len);

// Mark cell for discharging (takes effect after config is re-written)
// |cell| should be [0, PLUTUS_CFG_TOTAL_CELLS)
StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge);

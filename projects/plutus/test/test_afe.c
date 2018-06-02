#include <stddef.h>
#include <stdint.h>
#include "interrupt.h"
#include "ltc_afe.h"
#include "plutus_cfg.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define AFE_TEST_SAMPLES 5
#define AFE_TEST_VOLTAGE_VARIATION 10

static LtcAfeStorage s_afe;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  const LtcAfeSettings afe_settings = {
    .mosi = PLUTUS_CFG_AFE_SPI_MOSI,
    .miso = PLUTUS_CFG_AFE_SPI_MISO,
    .sclk = PLUTUS_CFG_AFE_SPI_SCLK,
    .cs = PLUTUS_CFG_AFE_SPI_CS,

    .spi_port = PLUTUS_CFG_AFE_SPI_PORT,
    .spi_baudrate = PLUTUS_CFG_AFE_SPI_BAUDRATE,
    .adc_mode = PLUTUS_CFG_AFE_MODE,

    .cell_bitset = PLUTUS_CFG_CELL_BITSET_ARR,
    .aux_bitset = PLUTUS_CFG_AUX_BITSET_ARR,
  };

  ltc_afe_init(&s_afe, &afe_settings);
}

void teardown_test(void) {}

void test_ltc_afe_adc_conversion_initiated(void) {
  uint16_t voltages[PLUTUS_CFG_TOTAL_CELLS] = { 0 };
  StatusCode status = ltc_afe_read_all_voltage(&s_afe, voltages, SIZEOF_ARRAY(voltages));
  TEST_ASSERT_OK(status);

  // if the ADC conversion packet is valid, then these should be voltage values
  // otherwise, the reading will correspond to the values the registers are
  // initialized as by default (0xFF)
  for (int i = 0; i < PLUTUS_CFG_TOTAL_CELLS; ++i) {
    TEST_ASSERT_NOT_EQUAL(0xFFFF, voltages[i]);
  }
}

void test_ltc_afe_read_all_voltage_repeated_within_tolerances(void) {
  // the idea here is that we repeatedly take samples and verify that the values being read
  // are within an acceptable tolerance
  uint16_t samples[PLUTUS_CFG_TOTAL_CELLS][AFE_TEST_SAMPLES] = { 0 };

  for (int sample = 0; sample < AFE_TEST_SAMPLES; ++sample) {
    uint16_t voltages[PLUTUS_CFG_TOTAL_CELLS] = { 0 };
    StatusCode status = ltc_afe_read_all_voltage(&s_afe, voltages, SIZEOF_ARRAY(voltages));
    TEST_ASSERT_OK(status);

    for (int cell = 0; cell < PLUTUS_CFG_TOTAL_CELLS; ++cell) {
      samples[cell][sample] = voltages[cell];

      for (int old_sample = 0; old_sample < sample; ++old_sample) {
        uint16_t old_value = samples[cell][old_sample];
        TEST_ASSERT_UINT16_WITHIN(AFE_TEST_VOLTAGE_VARIATION, old_value, voltages[cell]);
      }
    }
  }
}

void test_ltc_afe_read_all_voltage_wrong_size(void) {
  uint16_t voltages[1] = { 0 };
  StatusCode status = ltc_afe_read_all_voltage(&s_afe, voltages, SIZEOF_ARRAY(voltages));

  TEST_ASSERT_NOT_OK(status);
}

void test_ltc_afe_read_all_aux_repeated_within_tolerances(void) {
  // the idea here is that we repeatedly take samples and verify that the values being read
  // are within an acceptable tolerance
  uint16_t samples[PLUTUS_CFG_TOTAL_CELLS][AFE_TEST_SAMPLES] = { 0 };

  for (int sample = 0; sample < AFE_TEST_SAMPLES; ++sample) {
    uint16_t voltages[PLUTUS_CFG_TOTAL_CELLS] = { 0 };
    StatusCode status = ltc_afe_read_all_aux(&s_afe, voltages, SIZEOF_ARRAY(voltages));
    TEST_ASSERT_OK(status);

    for (int cell = 0; cell < PLUTUS_CFG_TOTAL_CELLS; ++cell) {
      samples[cell][sample] = voltages[cell];

      for (int old_sample = 0; old_sample < sample; ++old_sample) {
        uint16_t old_value = samples[cell][old_sample];
        TEST_ASSERT_UINT16_WITHIN(AFE_TEST_VOLTAGE_VARIATION, old_value, voltages[cell]);
      }
    }
  }
}

void test_ltc_afe_read_all_aux_wrong_size(void) {
  uint16_t voltages[1] = { 0 };
  StatusCode status = ltc_afe_read_all_aux(&s_afe, voltages, SIZEOF_ARRAY(voltages));

  TEST_ASSERT_NOT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_valid_range(void) {
  uint16_t valid_cell = 0;
  StatusCode status = ltc_afe_toggle_cell_discharge(&s_afe, valid_cell, true);

  TEST_ASSERT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_invalid_range(void) {
  uint16_t invalid_cell = PLUTUS_CFG_TOTAL_CELLS;
  StatusCode status = ltc_afe_toggle_cell_discharge(&s_afe, invalid_cell, true);

  TEST_ASSERT_NOT_OK(status);
}

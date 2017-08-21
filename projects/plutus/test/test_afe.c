#include <stddef.h>
#include <stdint.h>
#include "ltc_afe.h"
#include "plutus_config.h"
#include "test_helpers.h"
#include "unity.h"

#define AFE_TEST_SAMPLES 5
#define AFE_TEST_VOLTAGE_VARIATION 10

// check that a and b are within tolerance of each other
static bool prv_within_range(uint16_t a, uint16_t b, uint16_t tolerance) {
  return a - b < tolerance || b - a < tolerance;
}

static LtcAfeSettings s_afe_settings = {
  .mosi = { GPIO_PORT_A, 7 },  //
  .miso = { GPIO_PORT_A, 6 },  //
  .sclk = { GPIO_PORT_A, 5 },  //
  .cs = { GPIO_PORT_A, 4 },    //

  .spi_port = SPI_PORT_1,  //
  .spi_baudrate = 250000,  //
  .adc_mode = LTC_AFE_ADC_MODE_27KHZ,
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  ltc_afe_init(&s_afe_settings);
}

void teardown_test(void) {}

void test_ltc_afe_adc_conversion_initiated(void) {
  uint16_t voltages[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };
  StatusCode status = ltc_afe_read_all_voltage(&s_afe_settings, voltages, SIZEOF_ARRAY(voltages));
  TEST_ASSERT_OK(status);

  // if the ADC conversion packet is valid, then these should be voltage values
  // otherwise, the reading will correspond to the values the registers are
  // initialized as by default (0xFF)
  for (int i = 0; i < LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN; ++i) {
    TEST_ASSERT_NOT_EQUAL(0xFFFF, voltages[i]);
  }
}

void test_ltc_afe_read_all_voltage_repeated_within_tolerances(void) {
  // the idea here is that we repeatedly take samples and verify that the values being read
  // are within an acceptable tolerance
  uint16_t samples[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN * AFE_TEST_SAMPLES] = {
    0
  };

  for (int sample = 0; sample < AFE_TEST_SAMPLES; ++sample) {
    uint16_t voltages[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };
    StatusCode status = ltc_afe_read_all_voltage(&s_afe_settings, voltages, SIZEOF_ARRAY(voltages));
    TEST_ASSERT_OK(status);

    for (int cell = 0; cell < LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN; ++cell) {
      samples[sample * LTC6804_CELLS_PER_DEVICE + cell] = voltages[cell];

      for (int old_sample = 0; old_sample < sample; ++old_sample) {
        uint16_t old_value = samples[sample * LTC6804_CELLS_PER_DEVICE + old_sample];
        TEST_ASSERT_TRUE(prv_within_range(old_value, voltages[cell], AFE_TEST_VOLTAGE_VARIATION));
      }
    }
  }
}

void test_ltc_afe_read_all_voltage_wrong_size(void) {
  uint16_t voltages[1] = { 0 };
  StatusCode status = ltc_afe_read_all_voltage(&s_afe_settings, voltages, SIZEOF_ARRAY(voltages));

  TEST_ASSERT_NOT_OK(status);
}

void test_ltc_afe_read_all_aux_repeated_within_tolerances(void) {
  // the idea here is that we repeatedly take samples and verify that the values being read
  // are within an acceptable tolerance
  uint16_t samples[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN * AFE_TEST_SAMPLES] = {
    0
  };

  for (int sample = 0; sample < AFE_TEST_SAMPLES; ++sample) {
    uint16_t voltages[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };
    StatusCode status = ltc_afe_read_all_aux(&s_afe_settings, voltages, SIZEOF_ARRAY(voltages));
    TEST_ASSERT_OK(status);

    for (int cell = 0; cell < LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN; ++cell) {
      samples[sample * LTC6804_CELLS_PER_DEVICE + cell] = voltages[cell];

      for (int old_sample = 0; old_sample < sample; ++old_sample) {
        uint16_t old_value = samples[sample * LTC6804_CELLS_PER_DEVICE + old_sample];
        TEST_ASSERT_TRUE(prv_within_range(old_value, voltages[cell], AFE_TEST_VOLTAGE_VARIATION));
      }
    }
  }
}

void test_ltc_afe_read_all_aux_wrong_size(void) {
  uint16_t voltages[1] = { 0 };
  StatusCode status = ltc_afe_read_all_aux(&s_afe_settings, voltages, SIZEOF_ARRAY(voltages));

  TEST_ASSERT_NOT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_valid_range(void) {
  uint16_t valid_cell = 0;
  StatusCode status = ltc_afe_toggle_discharge_cells(&s_afe_settings, valid_cell, true);

  TEST_ASSERT_OK(status);
}

void test_ltc_afe_toggle_discharge_cells_invalid_range(void) {
  uint16_t invalid_cell = LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN;
  StatusCode status = ltc_afe_toggle_discharge_cells(&s_afe_settings, invalid_cell, true);

  TEST_ASSERT_NOT_OK(status);
}

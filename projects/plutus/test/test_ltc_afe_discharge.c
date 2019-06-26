#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "ms_test_helpers.h"
#include "plutus_cfg.h"
#include "plutus_event.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"
#include "delay.h"

#define TEST_LTC_AFE_NUM_SAMPLES 100
// Maximum total measurement error in filtered mode - +/-2.8mV
// Maximum peak-to-peak at 7kHz: +/-250uV
#define TEST_LTC_AFE_VOLTAGE_VARIATION 5

static LtcAfeStorage s_afe;
static uint16_t s_result_arr[PLUTUS_CFG_AFE_TOTAL_CELLS];

static void prv_conv_cb(uint16_t *result_arr, size_t len, void *context) {
  memcpy(s_result_arr, result_arr, sizeof(s_result_arr));
}

static void prv_wait_conv(void) {
  Event e = { 0 };
  do {
    MS_TEST_HELPER_AWAIT_EVENT(e);
    TEST_ASSERT_NOT_EQUAL(PLUTUS_EVENT_AFE_FAULT, e.id);
    TEST_ASSERT_TRUE(ltc_afe_process_event(&s_afe, &e));
  } while (e.id != PLUTUS_EVENT_AFE_CALLBACK_RUN);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

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

    .cell_result_cb = prv_conv_cb,
    .aux_result_cb = prv_conv_cb,
    .result_context = NULL,
  };

  ltc_afe_init(&s_afe, &afe_settings);
}

void teardown_test(void) {}

void test_ltc_afe_adc_conversion_initiated(void) {
  TEST_ASSERT_OK(ltc_afe_request_cell_conversion(&s_afe));
  prv_wait_conv();

  // if the ADC conversion packet is valid, then these should be voltage values
  // otherwise, the reading will correspond to the values the registers are
  // initialized as by default (0xFF)
  for (int i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; ++i) {
    TEST_ASSERT_NOT_EQUAL(0xFFFF, s_result_arr[i]);
  }
}


void test_ltc_afe_discharge_cell(void) {
//  StatusCode discharge_status = ltc_afe_toggle_cell_discharge(&s_afe, 0, true);
  StatusCode discharge_status; 

  for(int i = 0; i<12; i++){
    discharge_status = ltc_afe_toggle_cell_discharge(&s_afe, i, true); 
    LOG_DEBUG("Status: %i\n", discharge_status); 

    TEST_ASSERT_OK(ltc_afe_request_aux_conversion(&s_afe)); 
    prv_wait_conv();
  }

    delay_s(600);

  for(int i = 0; i<12; i++){
    discharge_status = ltc_afe_toggle_cell_discharge(&s_afe, i, false); 
    LOG_DEBUG("Status: %i\n", discharge_status); 

    TEST_ASSERT_OK(ltc_afe_request_aux_conversion(&s_afe)); 
    prv_wait_conv();
  }


}

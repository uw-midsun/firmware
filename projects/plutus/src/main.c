#include <stddef.h>

#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "plutus_cfg.h"
#include "soft_timer.h"
#include "spi.h"
#include "wait.h"

static LtcAfeStorage s_afe;

static void prv_cell_conv_cb(uint16_t *result_arr, size_t len, void *context) {
  for (size_t i = 0; i < len; i++) {
    LOG_DEBUG("C%d: %d.%dmV\n", i, result_arr[i] / 10, result_arr[i] % 10);
  }

  ltc_afe_request_aux_conversion(&s_afe);
}

static void prv_aux_conv_cb(uint16_t *result_arr, size_t len, void *context) {
  for (size_t i = 0; i < len; i++) {
    LOG_DEBUG("C%d: aux %d.%dmV\n", i, result_arr[i] / 10, result_arr[i] % 10);
  }

  ltc_afe_request_cell_conversion(&s_afe);
}

int main(void) {
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

    .cell_result_cb = prv_cell_conv_cb,
    .aux_result_cb = prv_aux_conv_cb,
    .result_context = NULL,
  };

  ltc_afe_init(&s_afe, &afe_settings);
  ltc_afe_request_cell_conversion(&s_afe);

  Event e = { 0 };
  while (true) {
    wait();
    while (status_ok(event_process(&e))) {
      ltc_afe_process_event(&s_afe, &e);
    }
  }
}

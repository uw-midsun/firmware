#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "log.h"
#include "ltc2484.h"

uint8_t s_filter_modes[NUM_LTC_2484_FILTER_MODES] = {
  LTC2484_REJECTION_50HZ_60HZ,
  LTC2484_REJECTION_50HZ,
  LTC2484_REJECTION_60HZ,
};

StatusCode ltc2484_init(const Ltc2484Settings *config) {
  if (config->filter_mode >= NUM_LTC_2484_FILTER_MODES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // The LTC2484 uses SPI Mode 0 (see Figure 5 on p.20 in the datasheet)
  SPISettings spi_config = {
    .baudrate = config->spi_baudrate,
    .mode = SPI_MODE_0,
    .mosi = config->mosi,
    .miso = config->miso,
    .sclk = config->sclk,
    .cs = config->cs,
  };

  spi_init(config->spi_port, &spi_config);

  uint8_t data[1] = { LTC2484_ENABLE | LTC2484_EXTERNAL_INPUT | LTC2484_AUTO_CALIBRATION |
                      s_filter_modes[config->filter_mode] };

  return spi_exchange(config->spi_port, data, 1, NULL, 0);
}

StatusCode ltc2484_raw_adc_to_uv(uint8_t *spi_data, int32_t *voltage) {
  Ltc2484Response tmp;

  tmp.u8data[0] = spi_data[3];
  tmp.u8data[1] = spi_data[2];
  tmp.u8data[2] = spi_data[1];
  tmp.u8data[3] = spi_data[0];

  // We have an overrange condition when the SIG and MSB bits are both set
  // (see Table 3 on p.16 of the datasheet)
  if ((spi_data[0] & LTC2484_ERROR_CODE_MASK) == LTC2484_OVERRANGE_CODE) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }
  // We have an underrange condition when the SIG and MSB bits are not set and
  // B27-25 are set
  if ((spi_data[0] & LTC2484_ERROR_CODE_MASK) == LTC2484_UNDERRANGE_CODE) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  // Sign extend tmp.i32data as two's complement
  // We don't care about the following values, so we can shift them out:
  // - The higher 3 bits are the SIG, DMY and MSB
  // - The lower 4 bits are the configuration word from the previous conversion
  int32_t adc_value = ((tmp.i32data << 3) >> 8);

  // Convert the voltage to uV
  // 2^24 * 4092 * 1000 = 6.86523679e13 (which fits in an int64_t)
  // dividing by 2^24 gives 4092000, which fits in an int32_t
  *voltage = (int32_t)(((int64_t)(adc_value)*LTC2484_V_REF_MILLIVOLTS * 1000) >> 24);

  return STATUS_CODE_OK;
}

StatusCode ltc2484_read(const Ltc2484Settings *config, int32_t *value) {
  // Pull CS low so we can check for MISO to go low, signalling that the
  // conversion is now complete
  gpio_set_state(&config->cs, GPIO_STATE_LOW);

  // Disable the Alt Fn on MISO so we can read the value
  GPIOSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  gpio_init_pin(&config->miso, &settings);

  // According to the Timing Characteristics (p.5 in the datasheet), we should
  // expect 149.9ms for conversion time (in the worst case).
  //
  // In order to prevent us from blocking forever, we set a timeout to 3x the
  // expected number of times this busy loop cycles.
  for (int i = 0; i < LTC2484_TIMEOUT_CYCLES; ++i) {
    GPIOState state = NUM_GPIO_STATES;
    gpio_get_state(&config->miso, &state);

    if (state == GPIO_STATE_LOW) {
      // MISO has now gone low, signaling that the conversion has finished
      break;
    }

    if (i > LTC2484_TIMEOUT_CYCLES) {
      // Restore CS high
      gpio_set_state(&config->cs, GPIO_STATE_HIGH);

      // Restore the Alt Fn of the MISO pin
      settings.alt_function = GPIO_ALTFN_0;
      settings.direction = GPIO_DIR_IN;
      settings.state = GPIO_STATE_HIGH;

      gpio_init_pin(&config->miso, &settings);

      return status_code(STATUS_CODE_TIMEOUT);
    }
  }

  // Restore CS high so we can trigger a SPI exchange
  gpio_set_state(&config->cs, GPIO_STATE_HIGH);

  // Restore the Alt Fn of the MISO pin
  settings.alt_function = GPIO_ALTFN_0;
  settings.direction = GPIO_DIR_IN;
  settings.state = GPIO_STATE_HIGH;

  gpio_init_pin(&config->miso, &settings);

  // Keep the previous mode and don't do anything special (ie. send a command
  // byte equal to 0). Since our SPI driver sends 0x00 by default, we can just
  // send NULL.
  //
  // Due to the way our SPI driver works, we send NULL bytes in order to ensure
  // that 4 bytes are exchanged in total.
  uint8_t result[4] = { 0 };
  StatusCode status = spi_exchange(config->spi_port, NULL, 0, result, 4);

  return ltc2484_raw_adc_to_uv(result, value);
}

#pragma once

#include "spi.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"
#include "stm32f0xx.h"

static SPI_TypeDef *spi_periph_map[] = { SPI1, SPI2 };

static GPIOAddress s_cs_pin_map[] = { 0, 0 };

static bool prv_is_periph_valid(const SPIPeriph spi_x) {
  return spi_x == 0 || spi_x == 1;
}

static bool prv_are_settings_valid(const SPISettings *settings) {
  return !(settings->polarity > SPI_CPOL_HIGH ||
           settings->phase > SPI_CPHA_2EDGE ||
           settings->baud_rate > SPI_BAUDRATE_256 ||
           settings->first_bit > SPI_FIRSTBIT_LSB);
}

static bool prv_is_state_valid(const GPIOState state) {
  return state < NUM_GPIO_STATE;
}

static uint8_t prv_exchange(SPIPeriph spi_x, uint8_t data) {
  while (!SPI_I2SGetFlagStatus(SPI_I2S_FLAG_TXE)) {}
  SPI_SendData8(spi_periph_map[spi_x], data);

  while (!SPI_I2SGetFlagStatus(SPI_I2S_FLAG_RXNE)) {}
  return SPI_ReceiveData8;
}

StatusCode spi_init(SPIPeriph spi_x, SPISettings *settings) {
  if (!prv_is_periph_valid(spi_x) || !prv_are_settings_valid(settings)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enable peripheral clock
  if (spi_x == 1) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  } else {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  }

  // Note: gpio_init_pin initializes pin clocks as well

  // Initialize MOSI pin
  GPIOSettings pin_settings = { 0 };
  pin_settings.direction = GPIO_DIR_OUT;
  pin_settings.state = GPIO_STATE_LOW;
  pin_settings.resistor = GPIO_RES_NONE;
  pin_settings.alt_function = GPIO_ALTFN_0;
  gpio_init_pin(&settings->mosi_pin, &pin_settings);

  // Initialize SCK pin
  gpio_init_pin(&settings->sck_pin, &pin_settings);

  // Initialize MISO pin
  pin_settings.direction = GPIO_DIR_IN;
  pin_settings.resistor = GPIO_RES_PULLUP;
  gpio_init_pin(&settings->miso_pin, &pin_settings);

  // Initialize CS pin
  pin_settings.direction = GPIO_DIR_OUT;
  pin_settings.state = GPIO_STATE_HIGH;
  gpio_init_pin(&settings->cs_pin, &pin_settings);
  s_cs_pin_map[spi_x] = settings->cs_pin;

  // Fill SPI init struct
  SPI_InitTypeDef spi_init_struct;

  // Default settings
  spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spi_init_struct.SPI_Mode = SPI_Mode_Master;
  spi_init_struct.SPI_NSS = SPI_NSS_Soft;
  spi_init_struct.SPI_CRCPolynomial = 7;
  spi_init_struct.SPI_DataSize = SPI_DataSize_8b;

  // Parse clock polarity
  if (settings->polarity == SPI_CPOL_LOW) {
    spi_init_struct.SPI_CPOL = SPI_CPOL_Low;
  } else {
    spi_init_struct.SPI_CPOL = SPI_CPOL_High;
  }

  // Parse clock phase
  if (settings->phase == SPI_CPHA_1EDGE) {
    spi_init_struct.SPI_CPHA = SPI_CPHA_1Edge;
  } else {
    spi_init_struct.SPI_CPHA = SPI_CPHA_2Edge;
  }

  // Parse Baud Rate prescaler value
  spi_init_struct.SPI_BaudRatePrescaler = ((settings->baud_rate / 2) << 4)
                                      + ((settings->baud_rate % 2) << 3);

  // Parse first bit
  if (settings->first_bit == SPI_FIRSTBIT_MSB) {
    spi_init_struct.SPI_FirstBit = SPI_FirstBit_MSB;
  } else {
    spi_init_struct.SPI_FirstBit = SPI_FirstBit_LSB;
  }

  Spi_Init(spi_periph_map[spi_x], &spi_init_struct);

  // Set FIFO threshold for RXNE event
  SPI_RxFIFOThresholdConfig(spi_periph_map[spi_x], SPI_RxFIFOThreshold_QF);

  // Enable peripheral
  SPI_Cmd(spi_periph_map[spi_x], ENABLE);
  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SPIPeriph spi_x, uint8_t *tx_data, size_t tx_len,
  uint8_t *rx_data, size_t rx_len) {
  size_t num_cycles = tx_len > rx_len ? tx_len : rx_len;
  for (int i = 0; i < num_cycles; ++i) {
    if (i > rx_len) {
      prv_exchange(spi_x, tx_data[i]);
    } else {
      rx_data[i] = prv_exchange(spi_x, i > tx_len ? 0x00 : tx_data[i]);
    }
  }
  return STATUS_CODE_OK;
}


StatusCode spi_set_cs_state(SPIPeriph spi_x, GPIOState state) {
  if (!prv_is_periph_valid(spi_x) || !prv_is_state_valid(state)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  gpio_set_pin_state(&s_cs_pin_map[spi_x], state);
  return STATUS_CODE_OK;
}

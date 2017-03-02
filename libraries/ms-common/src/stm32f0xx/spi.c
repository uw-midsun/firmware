#pragma once

#include "spi.h"

#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"
#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_spi.h"

static SPI_TypeDef *spi_periph_map[] = { SPI1, SPI2 };

static bool prv_is_periph_valid(const uint8_t spi_x) {
	return spi_x == 0 || spi_x = 1;
}

static bool prv_are_settings_valid(const SPISettings *settings) {
	return !(settings->polarity > SPI_CPOLARITY_HIGH ||
           settings->phase > SPI_CPHASE_2EDGE ||
           settings->baud_rate > SPI_BAUDRATE_256 ||
           settings->first_bit > SPI_FIRSTBIT_LSB);
}

static uint8_t prv_exchange(uint8_t spi_x, uint8_t data) {
  while(!SPI_I2S_GetFlagStatus(SPI_I2S_FLAG_TXE));
  SPI_SendData8(spi_periph_map[spi_x], data);

  while(!SPI_I2S_GetFlagStatus(SPI_I2S_FLAG_RXNE));
  return SPI_ReceiveData8(spi_periph_map[spi_x]);
}

StatusCode spi_init(uint8_t spi_x, SPISettings *settings) {
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
  GPIOSettings pin_settings;
  pin_settings.direction = GPIO_OUT;
  pin_settings.state = GPIO_STATE_LOW;
  pin_settings.resistor = GPIO_RES_NONE;
  pin_settings.alt_function = GPIO_ALTFN_0;
  gpio_init_pin(&settings->mosi_pin, &pin_settings);

  // Initialize SCK pin
  gpio_init_pin(&settings->sck_pin, &pin_settings);

  // Initialize MISO pin
  pin_settings.direction = GPIO_IN;
  pin_settings.resistor = GPIO_RES_PULLUP;
  gpio_init_pin(&settings->miso_pin, &pin_settings);

  // Initialize NSS pin
  pin_settings.direction = GPIO_OUT;
  pin_settings.state = GPIO_STATE_HIGH;
  gpio_init_pin(&settings->miso_pin, &pin_settings);

  // Fill SPI init struct
  SPI_InitTypeDef init_struct;

  // Default settings
  init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  init_struct.SPI_Mode = SPI_Mode_Master;
  init_struct.SPI_NSS = SPI_NSS_Soft;
  init_struct.SPI_CRCPolynomial = 7;
  init_struct.SPI_DataSize = SPI_DataSize_8b;

  // Parse clock polarity
  if (settings->polarity == SPI_CPOLARITY_LOW) {
    init_struct.SPI_CPOL = SPI_CPOL_Low;
  } else {
    init_struct.SPI_CPOL = SPI_CPOL_High;
  }
  
  // Parse clock phase
  if (settings->phase == SPI_CPHASE_1EDGE) {
    init_struct.SPI_CPHA = SPI_CPHA_1Edge;
  } else {
    init_struct.SPI_CPHA = SPI_CPHA_2Edge;
  }

  // Parse Baud Rate prescaler value
  init_struct.SPI_BaudRatePrescaler = ((settings->baud_rate / 2) << 4) 
                                      + ((settings->baud_rate % 2) << 3);

  // Parse first bit
  if (settings->first_bit == SPI_FIRSTBIT_MSB) {
    init_struct.SPI_FirstBit = SPI_FirstBit_MSB;
  } else {
    init_struct.SPI_FirstBit = SPI_FirstBit_LSB;
  }

  spi_init(spi_periph_map[spi_x], &init_struct);

  // Set FIFO threshold for RXNE event
  SPI_RxFIFOThresholdConfig(spi_periph_map[spi_x], SPI_RxFIFOThreshold_QF);

  // Enable peripheral
  SPI_Cmd(spi_periph_map[spi_x], ENABLE);
  return STATUS_CODE_OK;
}

StatusCode spi_send(uint8_t spi_x, uint8_t data) {
  if (!prv_is_periph_valid(spi_x)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_exchange(spi_x, data);
  return STATUS_CODE_OK;
}

StatusCode spi_send_array(uint8_t spi_x, uint8_t *data, size_t data_length) {
  if (!prv_is_periph_valid(spi_x)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  for (int i = 0; i * sizeof(uint8_t) < data_length; ++i) {
    prv_exchange(spi_x, data[i]);
  }

  return STATUS_CODE_OK;
}

uint8_t spi_receive(uint8_t spi_x) {
  if (!prv_is_periph_valid(spi_x)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  
  return prv_exchange(spi_x, 0x00); // dummy data
}

uint8_t spi_exchange(uint8_t spi_x, uint8_t data) {
  if (!prv_is_periph_valid(spi_x)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  return prv_exchange(spi_x, data);
}
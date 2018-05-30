#pragma once

#define PLUTUS_CFG_DEBUG

#ifdef PLUTUS_CFG_DEBUG
#define PLUTUS_CFG_AFE_DEVICES_IN_CHAIN 1
#define PLUTUS_CFG_TOTAL_CELLS 6
#define PLUTUS_CFG_INPUT_BITSET_ARR \
  { PLUTUS_CFG_INPUT_BITSET_SPLIT }
#else
// number of devices in daisy chain (including master)
#define PLUTUS_CFG_AFE_DEVICES_IN_CHAIN 4
#define PLUTUS_CFG_TOTAL_CELLS 36
// We're using 18 modules per box -> 2 AFEs each
// clang-format off
#define PLUTUS_CFG_INPUT_BITSET_ARR                                 \
  { PLUTUS_CFG_INPUT_BITSET_FULL, PLUTUS_CFG_INPUT_BITSET_SPLIT,    \
    PLUTUS_CFG_INPUT_BITSET_FULL, PLUTUS_CFG_INPUT_BITSET_SPLIT, }
// clang-format on
#endif

#define PLUTUS_CFG_AFE_SPI_PORT SPI_PORT_1
#define PLUTUS_CFG_AFE_SPI_BAUDRATE 750000
#define PLUTUS_CFG_AFE_SPI_MOSI \
  { .port = GPIO_PORT_A, .pin = 7 }
#define PLUTUS_CFG_AFE_SPI_MISO \
  { .port = GPIO_PORT_A, .pin = 6 }
#define PLUTUS_CFG_AFE_SPI_SCLK \
  { .port = GPIO_PORT_A, .pin = 5 }
#define PLUTUS_CFG_AFE_SPI_CS \
  { .port = GPIO_PORT_A, .pin = 4 }

#define PLUTUS_CFG_CURRENT_SENSE_SPI_PORT SPI_PORT_2
#define PLUTUS_CFG_CURRENT_SENSE_SPI_BAUDRATE 750000
#define PLUTUS_CFG_CURRENT_SENSE_MOSI \
  { .port = GPIO_PORT_B, .pin = 15 }
#define PLUTUS_CFG_CURRENT_SENSE_MISO \
  { .port = GPIO_PORT_B, .pin = 14 }
#define PLUTUS_CFG_CURRENT_SENSE_SCLK \
  { .port = GPIO_PORT_B, .pin = 13 }
#define PLUTUS_CFG_CURRENT_SENSE_CS \
  { .port = GPIO_PORT_B, .pin = 12 }

// Use normal mode
#define PLUTUS_CFG_AFE_MODE LTC_AFE_ADC_MODE_7KHZ

// Using all 12 cell inputs
#define PLUTUS_CFG_INPUT_BITSET_FULL 0xFFF
// Using 6 cell inputs split across the 2 muxes
#define PLUTUS_CFG_INPUT_BITSET_SPLIT (0x7 << 6 | 0x7)

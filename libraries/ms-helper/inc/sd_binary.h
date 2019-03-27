#pragma once
//
// This is a driver to read and write binary data to an SDHC card.
// The suggested usage is to initialize the SD card, and then use
// FatFs to write to it.
//
// This module requires that the SPI port which the card is mounted
// on is already initialized. As well, soft timers and interrupts
// must be initialized
//
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"

typedef enum {
  SD_RESPONSE_R1 = 0,
  SD_RESPONSE_R1B,
  SD_RESPONSE_R2,
  SD_RESPONSE_R3,
  SD_RESPONSE_R4R5,
  SD_RESPONSE_R7,
  NUM_SD_RESPONSES
} SdResponseType;

typedef struct SdResponse {
  uint8_t r1;
  uint8_t r2;
  uint8_t r3;
  uint8_t r4;
  uint8_t r5;
} SdResponse;

// For SDHC and SDXC cards, the address provided to these functions should be the block address

// Initialize the SD card on a given SPI port
StatusCode sd_card_init(SpiPort spi);

// Read block from the SD card. |dest| is where the read blocks will be written into. Make sure that
// this buffer is large enough for the content
StatusCode sd_read_blocks(SpiPort spi, uint32_t *dest, uint32_t readAddr, uint32_t numberOfBlocks);

// Write data from |src| to the specified address on the SD card
StatusCode sd_write_blocks(SpiPort spi, uint32_t *src, uint32_t writeAddr, uint32_t numberOfBlocks);

// Same as |sd_write_blocks|, but uses a different mechanism internally. Use this one for multiple
// blocks. Use the other one for single blocks.
StatusCode sd_multi_write_blocks(SpiPort spi, uint32_t *src, uint32_t writeAddr,
                                 uint32_t numberOfBlocks);

// Determines whether the SD card is ready in on a given SPI port
StatusCode sd_is_initialized(SpiPort spi);

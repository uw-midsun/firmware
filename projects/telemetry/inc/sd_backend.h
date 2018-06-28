#pragma once

// This is the backend for the SD card reader. As a user you only need to initialize
// this module with sd_init_module and then use FatFs. All of these other functions
// are not for public consumption are are only used internally by FatFs.
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "spi.h"

#define SD_BLOCK_SIZE (512)

#define SD_R1_NO_ERROR (0x00)
#define SD_R1_IN_IDLE_STATE (0x01)
#define SD_R1_ILLEGAL_COMMAND (0x04)
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ (0xFE)
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE (0xFE)
#define SD_DUMMY_BYTE (0xFF)

#define SD_CMD_GO_IDLE_STATE (0)
#define SD_CMD_SEND_IF_COND (8)
#define SD_CMD_STATUS (13)
#define SD_CMD_SET_BLOCKLEN (16)
#define SD_CMD_READ_SINGLE_BLOCK (17)
#define SD_CMD_WRITE_SINGLE_BLOCK (24)
#define SD_CMD_SD_APP_OP_COND (41)
#define SD_CMD_APP_CMD (55)
#define SD_CMD_READ_OCR (58)

#define SD_DATA_OK (0x05)
#define SD_DATA_CRC_ERROR (0x0B)
#define SD_DATA_WRITE_ERROR (0x0D)
#define SD_DATA_OTHER_ERROR (0xFF)

typedef struct SdSettings {
  GPIOAddress cs;
  GPIOAddress sclk;
  GPIOAddress mosi;
  GPIOAddress miso;
  SPIPort port;
} SdSettings;

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

void sd_init_module(SdSettings *settings);
uint8_t sd_write_byte(uint8_t byte);
SdResponse sd_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, SdResponseType expected);
bool sd_card_init();

bool sd_read_blocks(uint32_t *pData, uint32_t readAddr, uint32_t numberOfBlocks);
bool sd_write_blocks(uint32_t *pData, uint32_t writeAddr, uint32_t numberOfBlocks);
bool sd_is_initialized();

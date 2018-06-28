/**
 *******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

// Included header since almost all of these functions are ported from
// \en.stm32cubef0\STM32Cube_FW_F0_V1.9.0\Drivers\BSP\STM32072B_EVAL

#include "sd_backend.h"
#include <stdbool.h>
#include <string.h>
#include "delay.h"
#include "gpio.h"
#include "spi.h"

#define SD_SEND_SIZE 6
#define SD_RECV_SIZE 5
#define SD_MAX_RETRY 100
#define SD_ERROR 0x01

static uint8_t s_placeholder = SD_DUMMY_BYTE;

static SdSettings *s_settings;
static bool s_initialized = false;

void sd_init_module(SdSettings *settings) {
  s_settings = settings;
}

void prv_sd_set_cs_line(bool high) {
  if (high) {
    gpio_set_state(&s_settings->cs, GPIO_STATE_HIGH);
  } else {
    gpio_set_state(&s_settings->cs, GPIO_STATE_LOW);
  }
}

static void prv_sd_send_frame(uint8_t cmd, uint32_t arg, uint8_t crc) {
  uint8_t frame[SD_SEND_SIZE];
  uint8_t frameRes[SD_SEND_SIZE];
  uint8_t res[SD_RECV_SIZE] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  // Split the cmd parameter into 8 byte ints
  frame[0] = (cmd | 0x40);
  frame[1] = (uint8_t)(arg >> 24);
  frame[2] = (uint8_t)(arg >> 16);
  frame[3] = (uint8_t)(arg >> 8);
  frame[4] = (uint8_t)(arg);
  frame[5] = (crc | 0x01);
  prv_sd_set_cs_line(false);
  spi_exchange(s_settings->port, frame, SD_SEND_SIZE, frameRes, SD_SEND_SIZE);
}

static uint8_t prv_sd_read_data() {
  uint8_t timeout = 0x08;
  uint8_t readvalue;

  do {
    readvalue = sd_write_byte(SD_DUMMY_BYTE);
    timeout--;
  } while ((readvalue == SD_DUMMY_BYTE) && timeout);

  return readvalue;
}

uint8_t sd_write_byte(uint8_t byte) {
  uint8_t result = 0x00;
  spi_exchange(s_settings->port, &byte, 1, &result, 1);
  return result;
}

SdResponse sd_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, SdResponseType expected) {
  prv_sd_send_frame(cmd, arg, crc);
  SdResponse res = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  switch (expected) {
    case SD_RESPONSE_R1:
      res.r1 = prv_sd_read_data();
      break;
    case SD_RESPONSE_R1B:
      res.r1 = prv_sd_read_data();
      res.r2 = sd_write_byte(SD_DUMMY_BYTE);
      prv_sd_set_cs_line(true);
      delay_ms(1);
      prv_sd_set_cs_line(false);
      uint8_t line = 0x00;
      while (sd_write_byte(SD_DUMMY_BYTE) != 0xFF) {
      }
      break;
    case SD_RESPONSE_R2:
      res.r1 = prv_sd_read_data();
      res.r2 = sd_write_byte(SD_DUMMY_BYTE);
      break;
    case SD_RESPONSE_R3:
    case SD_RESPONSE_R7:
      res.r1 = prv_sd_read_data();
      res.r2 = sd_write_byte(SD_DUMMY_BYTE);
      res.r3 = sd_write_byte(SD_DUMMY_BYTE);
      res.r4 = sd_write_byte(SD_DUMMY_BYTE);
      res.r5 = sd_write_byte(SD_DUMMY_BYTE);
      break;
    default:
      break;
  }
  prv_sd_set_cs_line(true);
  return res;
}

uint8_t prv_sd_get_data_response() {
  uint8_t dataresponse;
  uint8_t rvalue = SD_DATA_OTHER_ERROR;

  dataresponse = sd_write_byte(SD_DUMMY_BYTE);

  // Consumes the busy response byte
  sd_write_byte(SD_DUMMY_BYTE);

  // Masks the bits which are not part of the response and
  // parses the response
  switch (dataresponse & 0x1F) {
    case SD_DATA_OK:
      rvalue = SD_DATA_OK;

      // Quickly pulses the CS line
      prv_sd_set_cs_line(true);
      prv_sd_set_cs_line(false);

      // Wait for IO line to return to 0xFF
      while (sd_write_byte(SD_DUMMY_BYTE) != 0xFF) {
      }
      break;
    case SD_DATA_CRC_ERROR:
      rvalue = SD_DATA_CRC_ERROR;
      break;
    case SD_DATA_WRITE_ERROR:
      rvalue = SD_DATA_WRITE_ERROR;
      break;
    default:
      break;
  }

  // Return response
  return rvalue;
}

void prv_pulse_idle() {
  prv_sd_set_cs_line(true);
  sd_write_byte(SD_DUMMY_BYTE);
}

bool sd_card_init() {
  SdResponse response;
  volatile uint8_t counter = 0;
  // Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and
  // wait for In Idle State Response (R1 Format) equal to 0x01
  do {
    counter++;
    response = sd_send_cmd(SD_CMD_GO_IDLE_STATE, 0, 0x95, SD_RESPONSE_R1);
    prv_sd_set_cs_line(true);
    sd_write_byte(SD_DUMMY_BYTE);
    if (counter >= SD_MAX_RETRY) {
      return false;
    }
  } while (response.r1 != SD_R1_IN_IDLE_STATE);

  // Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status
  // and wait until response (R7 Format) equal to 0xAA and
  response = sd_send_cmd(SD_CMD_SEND_IF_COND, 0x1AA, 0x87, SD_RESPONSE_R7);
  prv_sd_set_cs_line(true);
  sd_write_byte(SD_DUMMY_BYTE);
  if (response.r1 == SD_R1_IN_IDLE_STATE) {
    // initialise card V2
    do {
      // Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors)
      response = sd_send_cmd(SD_CMD_APP_CMD, 0, 0xFF, SD_RESPONSE_R1);
      prv_sd_set_cs_line(true);
      sd_write_byte(SD_DUMMY_BYTE);

      // Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no
      // errors)
      response = sd_send_cmd(SD_CMD_SD_APP_OP_COND, 0x40000000, 0xFF, SD_RESPONSE_R1);
      prv_sd_set_cs_line(true);
      sd_write_byte(SD_DUMMY_BYTE);
    } while (response.r1 == SD_R1_IN_IDLE_STATE);

    if ((response.r1 & SD_R1_ILLEGAL_COMMAND) == SD_R1_ILLEGAL_COMMAND) {
      do {
        // Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
        response = sd_send_cmd(SD_CMD_APP_CMD, 0, 0xFF, SD_RESPONSE_R1);
        prv_sd_set_cs_line(true);
        sd_write_byte(SD_DUMMY_BYTE);
        if (response.r1 != SD_R1_IN_IDLE_STATE) {
          return false;
        }
        // Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00:
        // no errors)
        response = sd_send_cmd(SD_CMD_SD_APP_OP_COND, 0x00000000, 0xFF, SD_RESPONSE_R1);
        prv_sd_set_cs_line(true);
        sd_write_byte(SD_DUMMY_BYTE);
      } while (response.r1 == SD_R1_IN_IDLE_STATE);
    }

    // Send CMD58 (SD_CMD_READ_OCR) to initialize SDHC or SDXC cards: R3 response (0x00: no errors)
    response = sd_send_cmd(SD_CMD_READ_OCR, 0x00000000, 0xFF, SD_RESPONSE_R3);
    prv_sd_set_cs_line(true);
    sd_write_byte(SD_DUMMY_BYTE);
    if (response.r1 != SD_R1_NO_ERROR) {
      return false;
    }
  } else {
    return false;
  }
  s_initialized = true;
  return true;
}

bool sd_wait_data(uint8_t data) {
  uint16_t timeout = 0xFFFF;
  uint8_t readvalue;

  // Check if response matches data

  do {
    readvalue = sd_write_byte(SD_DUMMY_BYTE);
    timeout--;
  } while ((readvalue != data) && timeout);

  if (timeout == 0) {
    // After time out
    return false;
  }

  return true;
}

bool sd_read_blocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumberOfBlocks) {
  uint32_t offset = 0;
  uint8_t ptr[512] = { 0xFF };
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response (0x00: no errors)
  response = sd_send_cmd(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_sd_set_cs_line(1);
  sd_write_byte(SD_DUMMY_BYTE);
  if (response.r1 != SD_R1_NO_ERROR) {
    prv_pulse_idle();
    return false;
  }

  if (ptr == NULL) {
    prv_pulse_idle();
    return false;
  }
  memset(ptr, SD_DUMMY_BYTE, sizeof(uint8_t) * SD_BLOCK_SIZE);

  // Data transfer
  while (NumberOfBlocks--) {
    // Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block
    // Check if the SD acknowledged the read block command: R1 response (0x00: no errors)
    response = sd_send_cmd(SD_CMD_READ_SINGLE_BLOCK, (ReadAddr + offset) / SD_BLOCK_SIZE, 0xFF,
                           SD_RESPONSE_R1);
    if (response.r1 != SD_R1_NO_ERROR) {
      prv_pulse_idle();
      return false;
    }

    // Now look for the data token to signify the start of the data
    if (sd_wait_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ)) {
      // Read the SD block data : read 512 bytes of data
      spi_exchange(s_settings->port, ptr, SD_BLOCK_SIZE, (uint8_t *)pData + offset, SD_BLOCK_SIZE);

      // Set next read address
      offset += SD_BLOCK_SIZE;
      // get CRC bytes (not really needed by us, but required by SD)
      sd_write_byte(SD_DUMMY_BYTE);
      sd_write_byte(SD_DUMMY_BYTE);
    } else {
      prv_pulse_idle();
      return false;
    }

    // Sets the CS line to high to end the read protocol
    prv_sd_set_cs_line(true);
    sd_write_byte(SD_DUMMY_BYTE);
  }

  prv_pulse_idle();
  return true;
}

bool sd_write_blocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumberOfBlocks) {
  uint32_t offset = 0;
  uint8_t ptr[SD_BLOCK_SIZE];
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response (0x00: no errors)
  response = sd_send_cmd(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_sd_set_cs_line(true);
  sd_write_byte(SD_DUMMY_BYTE);
  if (response.r1 != SD_R1_NO_ERROR) {
    prv_pulse_idle();
    return false;
  }

  if (ptr == NULL) {
    prv_pulse_idle();
    return false;
  }

  // Data transfer
  while (NumberOfBlocks--) {
    // Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
    // Check if the SD acknowledged the write block command: R1 response (0x00: no errors)
    response = sd_send_cmd(SD_CMD_WRITE_SINGLE_BLOCK, (WriteAddr + offset) / SD_BLOCK_SIZE, 0xFF,
                           SD_RESPONSE_R1);
    if (response.r1 != SD_R1_NO_ERROR) {
      prv_pulse_idle();
      return false;
    }

    // Send dummy byte for NWR timing : one byte between CMDWRITE and TOKEN
    sd_write_byte(SD_DUMMY_BYTE);
    sd_write_byte(SD_DUMMY_BYTE);

    // Send the data token to signify the start of the data
    sd_write_byte(SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE);

    // Write the block data to SD
    spi_exchange(s_settings->port, (uint8_t *)pData + offset, SD_BLOCK_SIZE, ptr, SD_BLOCK_SIZE);

    // Set next write address
    offset += SD_BLOCK_SIZE;

    // Put CRC bytes (not really needed by us, but required by SD)
    sd_write_byte(SD_DUMMY_BYTE);
    sd_write_byte(SD_DUMMY_BYTE);

    // Read data response
    if (prv_sd_get_data_response() != SD_DATA_OK) {
      // Quit and return failed status
      prv_pulse_idle();
      return false;
    }

    prv_sd_set_cs_line(true);
    sd_write_byte(SD_DUMMY_BYTE);
  }

  prv_pulse_idle();
  return true;
}

bool sd_is_initialized() {
  return s_initialized;
}

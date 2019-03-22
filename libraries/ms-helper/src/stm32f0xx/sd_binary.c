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

#include "sd_backend.h"
#include <stdbool.h>
#include <string.h>
#include "delay.h"
#include "log.h"
#include "gpio.h"
#include "spi.h"

#define SD_SEND_SIZE 6
#define SD_RECV_SIZE 5
#define SD_MAX_RETRY 100
#define SD_ERROR 0x01
#define SD_DUMMY_CONST 8

static uint8_t s_placeholder = SD_DUMMY_BYTE;

static SpiSettings *s_settings;
static SpiPort port;
static bool s_initialized = false;

static uint8_t prv_wait_byte() {
  uint8_t timeout = 0x08;
  uint8_t readvalue;

  do {
    readvalue = prv_write_read_byte(SD_DUMMY_BYTE);
    timeout--;
  } while ((readvalue == SD_DUMMY_BYTE) && timeout);

  return readvalue;
}

static uint8_t prv_read_byte() {
  uint8_t result = 0x00;
  spi_rx(port, &result, 1, 0xFF);
  return result;
}

static void prv_write_dummy(uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    prv_read_byte();
  }
}

static uint8_t prv_write_read_byte(uint8_t byte) {
  spi_tx(port, &byte, 1);
  return prv_read_byte();
}

static SdResponse prv_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, SdResponseType expected) {
  uint8_t frame[SD_SEND_SIZE];

  // Split the cmd parameter into 8 byte ints
  frame[0] = (cmd | 0x40);
  frame[1] = (uint8_t)(arg >> 24);
  frame[2] = (uint8_t)(arg >> 16);
  frame[3] = (uint8_t)(arg >> 8);
  frame[4] = (uint8_t)(arg);
  frame[5] = (uint8_t)(crc);

  prv_write_dummy(SD_DUMMY_CONST);

  spi_cs_set_state(port, GPIO_STATE_LOW);

  prv_write_dummy(SD_DUMMY_CONST);

  spi_tx(port, frame, SD_SEND_SIZE, 0xFF);
  spi_rx(port, frame, SD_SEND_SIZE, 0xFF);

  SdResponse res = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  switch (expected) {
    case SD_RESPONSE_R1:
      res.r1 = prv_wait_byte();
      break;
    case SD_RESPONSE_R1B:
      res.r1 = prv_wait_byte();
      res.r2 = prv_write_read_byte(SD_DUMMY_BYTE);
      spi_cs_set_state(port, GPIO_STATE_HIGH);
      delay_ms(1);
      spi_cs_set_state(port, GPIO_STATE_LOW);
      while (prv_write_read_byte(SD_DUMMY_BYTE) != 0xFF) {
      }
      break;
    case SD_RESPONSE_R2:
      res.r1 = prv_wait_byte();
      res.r2 = prv_write_read_byte(SD_DUMMY_BYTE);
      break;
    case SD_RESPONSE_R3:
    case SD_RESPONSE_R7:
      res.r1 = prv_wait_byte();
      res.r2 = prv_write_read_byte(SD_DUMMY_BYTE);
      res.r3 = prv_write_read_byte(SD_DUMMY_BYTE);
      res.r4 = prv_write_read_byte(SD_DUMMY_BYTE);
      res.r5 = prv_write_read_byte(SD_DUMMY_BYTE);
      break;
    default:
      break;
  }
  return res;
}

uint8_t prv_sd_get_data_response() {
  volatile uint8_t dataresponse;
  uint8_t rvalue = SD_DATA_OTHER_ERROR;
  uint16_t timeout = 0xFFFF;
  while ((dataresponse = prv_read_byte()) == 0xFF && timeout) {
    timeout--;
  }

  // Consumes the busy response byte
  prv_read_byte();

  // Masks the bits which are not part of the response and
  // parses the response
  switch (dataresponse & 0x1F) {
    case SD_DATA_OK:
      rvalue = SD_DATA_OK;

      // Quickly pulses the CS line
      spi_cs_set_state(port, GPIO_STATE_HIGH);
      spi_cs_set_state(port, GPIO_STATE_LOW);

      // Wait for IO line to return to 0xFF
      while (prv_read_byte() != 0xFF) {
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
  spi_cs_set_state(port, GPIO_STATE_HIGH);
  prv_write_read_byte(SD_DUMMY_BYTE);
}

bool sd_card_init() {
  volatile SdResponse response = {0, 0, 0, 0, 0};
  volatile uint16_t counter = 0;
  // Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and
  // wait for In Idle State Response (R1 Format) equal to 0x01
  prv_write_dummy(10);
  do {
    counter++;
    response = prv_send_cmd(SD_CMD_GO_IDLE_STATE, 0, 0x95, SD_RESPONSE_R1);
    spi_cs_set_state(port, GPIO_STATE_HIGH);
    if (counter >= 100) {
      LOG_WARN("sd_card_init failed due to too many retries\n");
      return false;
    }
    delay_ms(20);
  } while (response.r1 != SD_R1_IN_IDLE_STATE);

  // Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status
  // and wait until response (R7 Format) equal to 0xAA and
  response = prv_send_cmd(SD_CMD_SEND_IF_COND, 0x1AA, 0x87, SD_RESPONSE_R7);
  spi_cs_set_state(port, GPIO_STATE_HIGH);
  prv_write_read_byte(SD_DUMMY_BYTE);
  if (response.r1 == SD_R1_IN_IDLE_STATE) {
    // initialise card V2
    do {
      // Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors)
      response = prv_send_cmd(SD_CMD_APP_CMD, 0, 0x65, SD_RESPONSE_R1);
      spi_cs_set_state(port, GPIO_STATE_HIGH);
      prv_write_read_byte(SD_DUMMY_BYTE);

      // Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00: no
      // errors)
      response = prv_send_cmd(SD_CMD_SD_APP_OP_COND, 0x40000000, 0x77, SD_RESPONSE_R1);
      spi_cs_set_state(port, GPIO_STATE_HIGH);
      prv_write_read_byte(SD_DUMMY_BYTE);
    } while (response.r1 == SD_R1_IN_IDLE_STATE);

    if ((response.r1 & SD_R1_ILLEGAL_COMMAND) == SD_R1_ILLEGAL_COMMAND) {
      do {
        // Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00: no errors) */
        response = prv_send_cmd(SD_CMD_APP_CMD, 0, 0x65, SD_RESPONSE_R1);
        spi_cs_set_state(port, GPIO_STATE_HIGH);
        prv_write_read_byte(SD_DUMMY_BYTE);
        if (response.r1 != SD_R1_IN_IDLE_STATE) {
          LOG_WARN("sd_card_init failed due incorrect return code\n");
          return false;
        }
        // Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards: R1 response (0x00:
        // no errors)
        response = prv_send_cmd(SD_CMD_SD_APP_OP_COND, 0x40000000, 0x77, SD_RESPONSE_R1);
        spi_cs_set_state(port, GPIO_STATE_HIGH);
        prv_write_read_byte(SD_DUMMY_BYTE);
      } while (response.r1 == SD_R1_IN_IDLE_STATE);
    }

    // Send CMD58 (SD_CMD_READ_OCR) to initialize SDHC or SDXC cards: R3 response (0x00: no errors)
    response = prv_send_cmd(SD_CMD_READ_OCR, 0x00000000, 0xFF, SD_RESPONSE_R3);
    spi_cs_set_state(port, GPIO_STATE_HIGH);
    prv_write_read_byte(SD_DUMMY_BYTE);
    if (response.r1 != SD_R1_NO_ERROR) {
      LOG_WARN("sd_card_init failed due to SD_R1_NO_ERROR\n");
      return false;
    }
  } else {
    LOG_WARN("sd_card_init failed because card is not in idle state\n");
    return false;
  }
  s_initialized = true;
  return true;
}

bool sd_wait_data(uint8_t data) {
  uint16_t timeout = 0xFFF;
  uint8_t readvalue;

  // Check if response matches data

  do {
    readvalue = prv_read_byte();
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
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response (0x00: no errors)
  response = prv_send_cmd(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle();

  if (response.r1 != SD_R1_NO_ERROR) {
    LOG_CRITICAL("Failed to read because response.r1 != SD_R1_NO_ERROR1\n");
    prv_pulse_idle();
    return false;
  }

  // Data transfer
  while (NumberOfBlocks--) {
    // Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block
    // Check if the SD acknowledged the read block command: R1 response (0x00: no errors)
    response = prv_send_cmd(SD_CMD_READ_SINGLE_BLOCK, (ReadAddr + offset) / SD_BLOCK_SIZE, 0xFF,
                           SD_RESPONSE_R1);
    if (response.r1 != SD_R1_NO_ERROR) {
      LOG_CRITICAL("Failed to read because response.r1 != SD_R1_NO_ERROR2\n");
      prv_pulse_idle();
      return false;
    }

    // Now look for the data token to signify the start of the data
    if (sd_wait_data(SD_TOKEN_START_DATA_SINGLE_BLOCK_READ)) {
      // Read the SD block data : read 512 bytes of data
      spi_rx(port, (uint8_t *)pData + offset, SD_BLOCK_SIZE, 0xFF);

      // Set next read address
      offset += SD_BLOCK_SIZE;
      // get CRC bytes (not really needed by us, but required by SD)
      prv_write_dummy(3);
    } else {
      prv_pulse_idle();
      LOG_CRITICAL("Failed to read because 3\n");
      return false;
    }

    // Sets the CS line to high to end the read protocol
    spi_cs_set_state(GPIO_STATE_HIGH);
    prv_write_read_byte(SD_DUMMY_BYTE);
  }

  prv_pulse_idle();
  return true;
}

bool sd_write_blocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumberOfBlocks) {
  uint32_t offset = 0;
  SdResponse response;
  

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response (0x00: no errors)
  response = prv_send_cmd(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle();
  if (response.r1 != SD_R1_NO_ERROR) {
    printf("\nfalse 1\n");
    return false;
  }

  // Data transfer
  while (NumberOfBlocks--) {
    // Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
    // Check if the SD acknowledged the write block command: R1 response (0x00: no errors)
    
    response = prv_send_cmd(SD_CMD_WRITE_SINGLE_BLOCK, (WriteAddr + offset) / SD_BLOCK_SIZE, 0xFF,
                           SD_RESPONSE_R1);
    if (response.r1 != SD_R1_NO_ERROR) {
      prv_pulse_idle();
      printf("\nfalse 2\n");
      return false;
    }

    prv_write_dummy(SD_DUMMY_CONST);
    

    // Send the data token to signify the start of the data
    uint8_t dat = SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE;
    spi_tx(port, &dat, 1, 0xFF);

    // Write the block data to SD
    //setlogging(true);
    spi_tx(port, (uint8_t *)pData + offset, SD_BLOCK_SIZE, 0xFF);

    // Set next write address
    offset += SD_BLOCK_SIZE;

    // Put CRC bytes (not really needed by us, but required by SD)
    uint8_t crc = 0x00;
    spi_tx(port, &crc, 1, 0xFF);
    spi_tx(port, &crc, 1, 0xFF);
    
    // Read data response
    uint8_t res = prv_sd_get_data_response();
    //printf("final response: %d\n", res);
    if (res != SD_DATA_OK) {
      // Quit and return failed status
      prv_pulse_idle();
      printf("\nfalse 3\n");
      return false;
    }
  }
  volatile uint8_t dataresponse;
  uint8_t rvalue = SD_DATA_OTHER_ERROR;
  uint16_t timeout = 0xFFFF;
  while ((dataresponse = prv_read_byte()) == 0x00 && timeout) {
    timeout--;
  }

  prv_pulse_idle();
  return true;
}

bool sd_multi_write_blocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumberOfBlocks) {
  uint32_t offset = 0;
  SdResponse response;
  

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response (0x00: no errors)
  response = prv_send_cmd(SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle();
  if (response.r1 != SD_R1_NO_ERROR) {
    printf("\nfalse 1\n");
    return false;
  }

  // Data transfer
  response = prv_send_cmd(SD_CMD_WRITE_MULTI_BLOCK, (WriteAddr + offset) / SD_BLOCK_SIZE, 0xFF,
                           SD_RESPONSE_R1);
  if (response.r1 != SD_R1_NO_ERROR) {
    prv_pulse_idle();
    printf("\nfalse 2\n");
    return false;
  }

  prv_write_dummy(SD_DUMMY_CONST);

  
  while (NumberOfBlocks--) {
    // Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
    // Check if the SD acknowledged the write block command: R1 response (0x00: no errors)
    
    // Send the data token to signify the start of the data
    uint8_t dat = SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE;
    spi_tx(port, &dat, 1, 0xFF);

    // Write the block data to SD
    spi_tx(port, (uint8_t *)pData + offset, SD_BLOCK_SIZE, 0xFF);

    // Set next write address
    offset += SD_BLOCK_SIZE;

    // Put CRC bytes (not really needed by us, but required by SD)
    uint8_t crc = 0x00;
    spi_tx(port, &crc, 1, 0xFF);
    spi_tx(port, &crc, 1, 0xFF);
    
    // Read data response
    uint8_t res = prv_sd_get_data_response();
    
    if (res != SD_DATA_OK) {
      // Quit and return failed status
      printf("sd_multi_write_blocks|final response: %d\n", res);
      prv_pulse_idle();
      printf("\nfalse 3\n");
      return false;
    }
    volatile uint8_t dataresponse;
    uint8_t rvalue = SD_DATA_OTHER_ERROR;
    uint16_t timeout = 0xFFFF;
    while ((dataresponse = prv_read_byte()) == 0x00 && timeout) {
      timeout--;
    }
  }

  // Write the block data to SD
  uint8_t end_transmission = SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE;
  spi_tx(port, &end_transmission, 1, 0xFF);
  
  prv_write_dummy(SD_DUMMY_CONST);

  volatile uint8_t dataresponse;
  uint8_t rvalue = SD_DATA_OTHER_ERROR;
  uint16_t timeout = 0xFFFF;
  while ((dataresponse = prv_read_byte()) == 0x00 && timeout) {
    timeout--;
  }

  return true;
}

StatusCode sd_init_module(SpiSettings *settings, SpiPort spi_port) {
  s_settings = settings;
  port = spi_port;
  return spi_init(spi_port, s_settings);
}

bool sd_is_initialized() {
  return s_initialized;
}

#include "i2c.h"
#include <stdbool.h>
#include "stm32f0xx.h"
#include "log.h"

#define I2C_TIMEOUT_WHILE_FLAG(i2c, flag, status) \
do { \
  uint32_t timeout = 1000000; \
  while (I2C_GetFlagStatus(i2c, flag) == status) { \
    timeout--; \
    if (timeout == 0) { \
      LOG_DEBUG("Timeout: %d waiting for %d to change\n", flag, status); \
      return; \
    } \
  } \
} while (0)

// I2C read opcode: S ADDR W A opcode SR ADDR R DOUT P
// I2C write opcode: S ADDR W A opcode SR ADDR W DIN P

static volatile I2C_TypeDef *s_i2c_ports[] = {
  [I2C_PORT_1] = I2C1, [I2C_PORT_2] = I2C2
};

static const uint32_t s_i2c_timing[] = {
  [I2C_SPEED_STANDARD] = 0x10805E89, // 100 kHz
  [I2C_SPEED_FAST] = 0x00901850, // 400 kHz
  [I2C_SPEED_FAST_PLUS] = 0x00700818 // 1 Mhz
};

static void prv_transfer(I2CPort port, uint8_t addr, bool read, uint8_t *data, size_t len,
                         uint32_t end_mode, uint32_t start_stop_mode) {
  I2C_TypeDef *i2c = s_i2c_ports[port];

  I2C_TransferHandling(i2c, addr << 1, len, end_mode, start_stop_mode);

  if (read) {
    for (int i = 0; i < len; i++) {
      I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_RXNE, RESET);
      data[i] = I2C_ReceiveData(i2c);
    }
  } else {
    for (int i = 0; i < len; i++) {
      I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_TXIS, RESET);
      I2C_SendData(i2c, data[i]);
    }
    I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_TC, RESET);
  }
}

static void prv_stop(I2CPort port) {
  I2C_TypeDef *i2c = s_i2c_ports[port];

  I2C_TIMEOUT_WHILE_FLAG(i2c, I2C_FLAG_STOPF, RESET);
  I2C_ClearFlag(i2c, I2C_FLAG_STOPF);
}

StatusCode i2c_init(I2CPort i2c, const I2CSettings *settings) {
  // TODO: support I2C2
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

  GPIOSettings io_settings = {
    .direction = GPIO_DIR_OUT_OD,
    .alt_function = GPIO_ALTFN_1
  };
  gpio_init_pin(&settings->scl, &io_settings);
  gpio_init_pin(&settings->sda, &io_settings);

  I2C_InitTypeDef i2c_init;
  I2C_StructInit(&i2c_init);
  i2c_init.I2C_Mode = I2C_Mode_I2C;
  i2c_init.I2C_Ack = I2C_Ack_Enable;
  i2c_init.I2C_Timing = s_i2c_timing[i2c];

  I2C_Init(s_i2c_ports[i2c], &i2c_init);

  I2C_Cmd(s_i2c_ports[i2c], ENABLE);
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t *rx_data, size_t rx_len) {
  I2C_TIMEOUT_WHILE_FLAG(s_i2c_ports[i2c], I2C_FLAG_BUSY, SET);

  prv_transfer(i2c, addr, true, rx_data, rx_len, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

  prv_stop(i2c);

  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t *tx_data, size_t tx_len) {
  I2C_TIMEOUT_WHILE_FLAG(s_i2c_ports[i2c], I2C_FLAG_BUSY, SET);

  prv_transfer(i2c, addr, false, tx_data, tx_len, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

  prv_stop(i2c);

  return STATUS_CODE_OK;
}

StatusCode i2c_read_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *rx_data, size_t rx_len) {
  I2C_TIMEOUT_WHILE_FLAG(s_i2c_ports[i2c], I2C_FLAG_BUSY, SET);

  prv_transfer(i2c, addr, false, &reg, sizeof(reg), I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
  prv_transfer(i2c, addr, true, rx_data, rx_len, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

  prv_stop(i2c);

  return STATUS_CODE_OK;
}

StatusCode i2c_write_reg(I2CPort i2c, I2CAddress addr, uint8_t reg, uint8_t *tx_data, size_t tx_len) {
  I2C_TIMEOUT_WHILE_FLAG(s_i2c_ports[i2c], I2C_FLAG_BUSY, SET);

  prv_transfer(i2c, addr, false, &reg, sizeof(reg), I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
  prv_transfer(i2c, addr, false, tx_data, tx_len, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);

  prv_stop(i2c);

  return STATUS_CODE_OK;
}

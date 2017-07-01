#include "i2c.h"
#include <stdbool.h>
#include "stm32f0xx.h"

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

// Opcode = addr + R/W
static void prv_transfer(I2CPort port, uint8_t addr, uint8_t opcode, bool read,
                         uint8_t *data, size_t len) {
  I2C_TypeDef *i2c = s_i2c_ports[port];

  while (I2C_GetFlagStatus(i2c, I2C_ISR_BUSY) != RESET) { }

  // Write target address to device
  I2C_TransferHandling(i2c, addr << 1, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);

  while (I2C_GetFlagStatus(i2c, I2C_ISR_TXIS) == RESET) { }
  I2C_SendData(i2c, opcode);
  while (I2C_GetFlagStatus(i2c, I2C_ISR_TC) == RESET) { }

  // Read/write data
  I2C_TransferHandling(i2c, addr << 1, len, I2C_AutoEnd_Mode,
                       read ? I2C_Generate_Start_Read : I2C_Generate_Start_Write);

  if (read) {
    for (int i = 0; i < len; i++) {
      while (I2C_GetFlagStatus(i2c, I2C_FLAG_RXNE) != SET) { }
      data[i] = I2C_ReceiveData(i2c);
    }
  } else {
    for (int i = 0; i < len; i++) {
      while (I2C_GetFlagStatus(i2c, I2C_ISR_TXIS) != SET) { }
      I2C_SendData(i2c, data[i]);
    }
  }

  while (I2C_GetFlagStatus(i2c, I2C_ISR_STOPF) == RESET) { }
  I2C_ClearFlag(i2c, I2C_ICR_STOPCF);
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
  i2c_init.I2C_Mode = I2C_Mode_SMBusHost;
  i2c_init.I2C_Ack = I2C_Ack_Enable;
  i2c_init.I2C_Timing = s_i2c_timing[i2c];

  I2C_Init(s_i2c_ports[i2c], &i2c_init);

  I2C_Cmd(s_i2c_ports[i2c], ENABLE);
}

StatusCode i2c_read(I2CPort i2c, I2CAddress addr, uint8_t opcode, uint8_t *rx_data, size_t rx_len) {
  prv_transfer(i2c, addr, false, opcode, rx_data, rx_len);

  return STATUS_CODE_OK;
}

StatusCode i2c_write(I2CPort i2c, I2CAddress addr, uint8_t opcode, uint8_t *tx_data, size_t tx_len) {
  prv_transfer(i2c, addr, true, opcode, tx_data, tx_len);

  return STATUS_CODE_OK;
}

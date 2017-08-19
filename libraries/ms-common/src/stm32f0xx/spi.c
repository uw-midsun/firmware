#include "spi.h"
#include "gpio.h"
#include "stm32f0xx.h"

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  SPI_TypeDef *base;
  GPIOAddress cs;
} SPIPortData;

static SPIPortData s_port[SPI_MCU_NUM_PORTS] = {
  {
    .rcc_cmd = RCC_APB2PeriphClockCmd,
    .periph = RCC_APB2Periph_SPI1,
    .base = SPI1
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_SPI2,
    .base = SPI2
  }
};

StatusCode spi_init(SPIPort spi, const SPISettings *settings) {
  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  size_t index = __builtin_ffsl(clocks.PCLK_Frequency / settings->baudrate);
  if (index <= 2) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid baudrate");
  }

  s_port[spi].rcc_cmd(s_port[spi].periph, ENABLE);
  s_port[spi].cs = settings->cs;

  GPIOSettings gpio_settings = {
    .alt_function = GPIO_ALTFN_0,
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH
  };

  gpio_init_pin(&settings->miso, &gpio_settings);

  gpio_settings.direction = GPIO_DIR_OUT;
  gpio_init_pin(&settings->mosi, &gpio_settings);
  gpio_init_pin(&settings->sclk, &gpio_settings);

  gpio_settings.alt_function = GPIO_ALTFN_NONE;
  gpio_init_pin(&settings->cs, &gpio_settings);

  SPI_InitTypeDef init = {
    .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .SPI_Mode = SPI_Mode_Master,
    .SPI_DataSize = SPI_DataSize_8b,
    .SPI_CPHA = (settings->mode & 0x01) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge,
    .SPI_CPOL = (settings->mode & 0x02) ? SPI_CPOL_High : SPI_CPOL_Low,
    .SPI_NSS = SPI_NSS_Soft,
    .SPI_BaudRatePrescaler = (index - 2) << 3,
    .SPI_FirstBit = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial = 7
  };
  SPI_Init(s_port[spi].base, &init);

  // Set the RX threshold to 1 byte
  SPI_RxFIFOThresholdConfig(s_port[spi].base, SPI_RxFIFOThreshold_QF);

  SPI_Cmd(s_port[spi].base, ENABLE);

  return STATUS_CODE_OK;
}

StatusCode spi_exchange(SPIPort spi, uint8_t *tx_data, size_t tx_len, uint8_t *rx_data,
                        size_t rx_len) {
  gpio_set_pin_state(&s_port[spi].cs, GPIO_STATE_LOW);

  for (int i = 0; i < tx_len; i++) {
    while (SPI_I2S_GetFlagStatus(s_port[spi].base, SPI_I2S_FLAG_TXE) == RESET) { }
    SPI_SendData8(s_port[spi].base, tx_data[i]);

    while (SPI_I2S_GetFlagStatus(s_port[spi].base, SPI_I2S_FLAG_RXNE) == RESET) { }
    SPI_ReceiveData8(s_port[spi].base);
  }

  for (int i = 0; i < rx_len; i++) {
    while (SPI_I2S_GetFlagStatus(s_port[spi].base, SPI_I2S_FLAG_TXE) == RESET) { }
    SPI_SendData8(s_port[spi].base, 0x00);

    while (SPI_I2S_GetFlagStatus(s_port[spi].base, SPI_I2S_FLAG_RXNE) == RESET) { }
    rx_data[i] = SPI_ReceiveData8(s_port[spi].base);
  }

  gpio_set_pin_state(&s_port[spi].cs, GPIO_STATE_HIGH);

  return STATUS_CODE_OK;
}

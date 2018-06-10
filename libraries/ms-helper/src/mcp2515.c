#include "mcp2515.h"
#include "mcp2515_defs.h"
#include "log.h"
#include "gpio_it.h"
#include <string.h>
#include <stddef.h>

typedef struct Mcp2515TxBuffer {
  uint8_t id;
  uint8_t data;
  uint8_t rts;
} Mcp2515TxBuffer;

typedef struct Mcp2515RxBuffer {
  uint8_t id;
  uint8_t data;
  uint8_t int_flag;
} Mcp2515RxBuffer;

static const Mcp2515TxBuffer s_tx_buffers[] = {
  { .id = MCP2515_LOAD_TXB0SIDH, .data = MCP2515_LOAD_TXB0D0, .rts = MCP2515_RTS_TXB0 },
  { .id = MCP2515_LOAD_TXB1SIDH, .data = MCP2515_LOAD_TXB1D0, .rts = MCP2515_RTS_TXB1 },
  { .id = MCP2515_LOAD_TXB2SIDH, .data = MCP2515_LOAD_TXB2D0, .rts = MCP2515_RTS_TXB2 },
};

static const Mcp2515RxBuffer s_rx_buffers[] = {
  { .id = MCP2515_READ_RXB0SIDH, .data = MCP2515_READ_RXB0D0, .int_flag = MCP2515_CANINT_RX0IE },
  { .id = MCP2515_READ_RXB1SIDH, .data = MCP2515_READ_RXB1D0, .int_flag = MCP2515_CANINT_RX1IE },
};

static void prv_reset(Mcp2515Storage *storage) {
  uint8_t payload[] = { MCP2515_CMD_RESET };
  spi_exchange(storage->settings.spi_port, payload, sizeof(payload), NULL, 0);
}

static void prv_read(Mcp2515Storage *storage, uint8_t addr, uint8_t *read_data, size_t read_len) {
  uint8_t payload[] = { MCP2515_CMD_READ, addr };
  spi_exchange(storage->settings.spi_port, payload, sizeof(payload), read_data, read_len);
}

static void prv_write(Mcp2515Storage *storage, uint8_t addr, uint8_t *write_data,
                      size_t write_len) {
  uint8_t payload[write_len + 2];
  payload[0] = MCP2515_CMD_WRITE;
  payload[1] = addr;
  memcpy(&payload[2], write_data, write_len);
  spi_exchange(storage->settings.spi_port, payload, sizeof(payload), write_data, write_len);
}

static void prv_bit_modify(Mcp2515Storage *storage, uint8_t addr, uint8_t mask, uint8_t data) {
  uint8_t payload[] = { MCP2515_CMD_BIT_MODIFY, addr, mask, data };
  spi_exchange(storage->settings.spi_port, payload, sizeof(payload), NULL, 0);
}

static uint8_t prv_read_status(Mcp2515Storage *storage) {
  uint8_t payload[] = { MCP2515_CMD_READ_STATUS };
  uint8_t read_data[1] = { 0 };
  spi_exchange(storage->settings.spi_port, payload, sizeof(payload), read_data, sizeof(read_data));

  return read_data[0];
}

static void prv_handle_rx(Mcp2515Storage *storage, uint8_t int_flags) {
  for (size_t i = 0; i < SIZEOF_ARRAY(s_rx_buffers); i++) {
    Mcp2515RxBuffer *rx_buf = &s_rx_buffers[i];
    if (int_flags & rx_buf->int_flag) {
      // message RX

      // Read ID
      uint8_t id_payload[] = { MCP2515_CMD_READ_RX | rx_buf->id };
      uint8_t read_id[5] = { 0 };
      spi_exchange(storage->settings.spi_port, id_payload, sizeof(id_payload), read_id,
                   sizeof(read_id));

      // Unpack ID
      // TODO: don't use magic numbers?
      uint32_t id = (uint32_t)((read_id[0] << 3) & 0xF) | (uint32_t)((read_id[1] >> 5) & 0x3) | (uint32_t)((read_id[1] << 27) & 0x2) |
                    (uint32_t)(read_id[2] << 19) | (uint32_t)(read_id[3] << 11);
      bool extended = (read_id[1] >> 3) & 0x1;
      size_t dlc = read_id[4] & MCP2515_TXBNDLC_DLC_MASK;

      uint8_t data_payload[] = { MCP2515_CMD_READ_RX | rx_buf->data };
      uint64_t read_data = 0;
      spi_exchange(storage->settings.spi_port, data_payload, sizeof(data_payload),
                   (uint8_t *)&read_data, sizeof(read_data));

      if (storage->settings.rx_cb != NULL) {
        storage->settings.rx_cb(id, extended, read_data, dlc, storage->settings.context);
      }
    }
  }
}

static void prv_handle_error(Mcp2515Storage *storage, uint8_t int_flags) {
  if (int_flags & MCP2515_CANINT_EFLAG) {
    // Do we bother recording errors?
    uint8_t clear = 0x00;
    prv_write(storage, MCP2515_CTRL_REG_EFLG, &clear, 1);
    prv_write(storage, MCP2515_CTRL_REG_TEC, &clear, 1);
    prv_write(storage, MCP2515_CTRL_REG_REC, &clear, 1);
  }
}

static void prv_handle_int(const GPIOAddress *address, void *context) {
  Mcp2515Storage *storage = context;

  uint8_t int_flags = 0;
  prv_read(storage, MCP2515_CTRL_REG_CANINTF, &int_flags, 1);

  // either RX or error
  prv_handle_rx(storage, int_flags);
  prv_handle_error(storage, int_flags);
}

StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings) {
  storage->settings = *settings;

  const SPISettings spi_settings = {
    .baudrate = settings->baudrate,
    .mode = SPI_MODE_0,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
  };
  spi_init(settings->spi_port, &spi_settings);

  prv_reset(storage);

  // Set to Config mode, CLKOUT /4
  prv_bit_modify(storage, MCP2515_CTRL_REG_CANCTRL,
                 MCP2515_CANCTRL_OPMODE_MASK | MCP2515_CANCTRL_CLKOUT_MASK,
                 MCP2515_CANCTRL_OPMODE_CONFIG | MCP2515_CANCTRL_CLKOUT_CLKPRE_4);

  // Hardcode to 500kbps for now
  const uint8_t registers[] = {
    0x05,  // CNF3: PS2 Length = 6
    MCP2515_CNF2_BTLMODE_CNF3 | MCP2515_CNF2_SAMPLE_3X |
        (0x07 << 3),  // PS1 Length = 8, PRSEG Length = 1
    0x00,             // CNF1: BRP = 1
    MCP2515_CANINT_EFLAG | MCP2515_CANINT_RX1IE |
        MCP2515_CANINT_RX0IE,  // CANINTE: Enable error and receive interrupts
    0x00,                      // CANINTF: clear all IRQ flags
    0x00                       // EFLG: clear all error flags
  };
  prv_write(storage, MCP2515_CTRL_REG_CNF3, registers, 6);

  prv_bit_modify(
      storage, MCP2515_CTRL_REG_CANCTRL, MCP2515_CANCTRL_OPMODE_MASK,
      (settings->loopback ? MCP2515_CANCTRL_OPMODE_LOOPBACK : MCP2515_CANCTRL_OPMODE_NORMAL));

  const GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
  };
  gpio_init_pin(&settings->int_pin, &gpio_settings);
  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  gpio_it_register_interrupt(&settings->int_pin, &it_settings, INTERRUPT_EDGE_FALLING,
                             prv_handle_int, storage);

  // For debug
  volatile uint8_t control_registers[8] = { 0 };
  prv_read(storage, MCP2515_CTRL_REG_CNF3, control_registers, 8);

  for (size_t i = 0; i < 8; i++) {
    printf("%d: 0x%x\n", i, control_registers[i]);
  }

  return STATUS_CODE_OK;
}

StatusCode mcp2515_tx(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                      size_t dlc) {
  // Get free transmit buffer
  uint8_t free_index =
      __builtin_ffs(~prv_read_status(storage) &
                    (MCP2515_STATUS_TX0REQ | MCP2515_STATUS_TX1REQ | MCP2515_STATUS_TX2REQ));
  LOG_DEBUG("TX: free index %d\n", free_index);
  if (free_index == 0) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  Mcp2515TxBuffer *tx_buf = &s_tx_buffers[free_index - 1];
  // Load ID:
  // STD[10:3] in SIDH[7:0], STD[2:0] in SIDL[7:5]
  // EXT[17:16] in SIDL[1:0], EXT[15:8] in EID8[15:8], EXT[7:0] in EID0[7:0]
  // extended bit in SIDL[3]
  uint8_t id_payload[] = {
    MCP2515_CMD_LOAD_TX | tx_buf->id,
    id >> 3,
    id << 5 | (uint8_t)(extended << 3) | id >> 27,
    id >> 19,
    id >> 11,
    dlc,
  };
  spi_exchange(storage->settings.spi_port, id_payload, sizeof(id_payload), NULL, 0);

  // Load data
  struct {
    uint8_t cmd;
    uint64_t data;
  } data_payload = {
    .cmd = MCP2515_CMD_LOAD_TX | tx_buf->data,
    .data = data,
  };
  spi_exchange(storage->settings.spi_port, (uint8_t *)&data_payload, sizeof(data_payload), NULL, 0);

  // Send message
  uint8_t send_payload[] = { MCP2515_CMD_RTS | tx_buf->rts };
  spi_exchange(storage->settings.spi_port, send_payload, sizeof(send_payload), NULL, 0);

  // For debug
  volatile uint8_t registers[14] = { 0 };
  prv_read(storage, MCP2515_CTRL_REG_TXB0CTRL, registers, 14);

  return STATUS_CODE_OK;
}

#pragma once
// MCP2515 SPI CAN controller
// Requires SPI, GPIO, GPIO interrupts, interrupts to be initialized
//
// Note that we aren't bothering to implement filtering on the controller side. We'll just filter
// in software since these are on isolated networks.
#include "gpio.h"
#include "spi.h"
#include "status.h"
#include <stdbool.h>
#include <stdint.h>

typedef void (*Mcp2515RxCb)(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context);

typedef struct Mcp2515Settings {
  SPIPort spi_port;
  uint32_t baudrate;
  GPIOAddress mosi;
  GPIOAddress miso;
  GPIOAddress sclk;
  GPIOAddress cs;

  GPIOAddress int_pin;

  bool loopback;

  Mcp2515RxCb rx_cb;
  void *context;
} Mcp2515Settings;

typedef struct Mcp2515Storage {
  Mcp2515Settings settings;
} Mcp2515Storage;

StatusCode mcp2515_init(Mcp2515Storage *storage, const Mcp2515Settings *settings);

StatusCode mcp2515_tx(Mcp2515Storage *storage, uint32_t id, bool extended, uint64_t data,
                      size_t dlc);

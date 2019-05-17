#pragma once

// MCP23008 I2C address definition (Datasheet Figure 1-4)
#define MCP23008_ADDRESS 0x20

// MCP23008 register address definitions (Datasheet Table 1-3)
#define MCP23008_IODIR 0x0
#define MCP23008_IPOL 0x1
#define MCP23008_GPINTEN 0x2
#define MCP23008_DEFVAL 0x3
#define MCP23008_INTCON 0x4
#define MCP23008_IOCON 0x5
#define MCP23008_GPPU 0x6
#define MCP23008_INTF 0x7
#define MCP23008_INTCAP 0x8
#define MCP23008_GPIO 0x9
#define MCP23008_OLAT 0xA

#define NUM_MCP23008_REGISTERS 11

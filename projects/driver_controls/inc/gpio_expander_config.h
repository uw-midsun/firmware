#pragma once

// MCP23008 I2C address (Datasheet Figure 1-4)
#define GPIO_EXPANDER_ADDRESS   0x20

// MCP23008 register addresses (Datasheet Table 1-3)
#define GPIO_EXPANDER_IODIR     0x0
#define GPIO_EXPANDER_IPOL      0x1
#define GPIO_EXPANDER_GPINTEN   0x2
#define GPIO_EXPANDER_DEFVAL    0x3
#define GPIO_EXPANDER_INTCON    0x4
#define GPIO_EXPANDER_IOCON     0x5
#define GPIO_EXPANDER_GPPU      0x6
#define GPIO_EXPANDER_INTF      0x7
#define GPIO_EXPANDER_INTCAP    0x8
#define GPIO_EXPANDER_GPIO      0x9
#define GPIO_EXPANDER_OLAT      0xA

#define NUM_GPIO_EXPANDER_REGISTERS 11

#pragma once

// ADS1015 I2C address definition (Datasheet Section 8.5.1.1)
#define ADS1015_I2C_ADDRESS             0x48

// ADS1015 I2C reset byte (Datasheet Section 8.5.1.2)
#define ADS1015_RESET_BYTE              0x06

// Used Reference voltage in mv.
#define ADS1015_REFERENCE_VOLTAGE       3300

// Register pointer definitions (Datasheet Table 4)
#define ADS1015_CONVERSION_REGISTER     0x00
#define ADS1015_CONFIG_REGISTER         0x01
#define ADS1015_LO_THRESH_REGISTER      0x02
#define ADS1015_HI_THRESH_REGISTER      0x03

// ADS1015 Bitmasks (According to their positions in their data byte)
#define ADS1015_CONFIG_MODE_CONT(byte)        (byte & 0xFE)

#define ADS1015_CONFIG_MUX_AIN0(byte)         ((byte & 0x8F) | 0x40)
#define ADS1015_CONFIG_MUX_AIN1(byte)         ((byte & 0x8F) | 0x50)
#define ADS1015_CONFIG_MUX_AIN2(byte)         ((byte & 0x8F) | 0x60)
#define ADS1015_CONFIG_MUX_AIN3(byte)         ((byte & 0x8F) | 0x70)

// Number of successive conversions required before asserting ALERT/RDY
#define ADS1015_CONFIG_COMP_QUE_ONE(byte)     ((byte & 0xFC) | 0x0)
#define ADS1015_CONFIG_COMP_QUE_TWO(byte)     ((byte & 0xFC) | 0x1)
#define ADS1015_CONFIG_COMP_QUE_FOUR(byte)    ((byte & 0xFC) | 0x2)

#define ADS1015_CONFIG_DR_128_SPS(byte)       (byte & 0x1F)

#define ADS1015_HI_THRESH_RDY                  0x80
#define ADS1015_LO_THRESH_RDY                  0x00

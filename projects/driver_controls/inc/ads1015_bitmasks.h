#pragma once

// ADS1015 I2C address definition (Datasheet Section 8.5.1.1)
#define ADS1015_I2C_ADDRESS 0x48

// ADS1015 I2C reset byte (Datasheet Section 8.5.1.2)
#define ADS1015_RESET_BYTE 0x06

// Register pointer definitions (Datasheet Table 4)
#define ADS1015_CONVERSION_REGISTER 0x00
#define ADS1015_CONFIG_REGISTER 0x01
#define ADS1015_LO_THRESH_REGISTER 0x02
#define ADS1015_HI_THRESH_REGISTER 0x03

// ADS1015 Bitmasks (According to their positions in their data byte)
#define ADS1015_CONFIG_OS(byte) (byte & 0x7F)
#define ADS1015_CONFIG_MODE_CONT(byte) (byte & 0xFE)

#define ADS1015_CONFIG_MUX(byte, channel) ((byte & 0x8F) | ((0x4 | channel) << 4))

#define ADS1015_CONFIG_DR_128_SPS(byte) (byte & 0x1F)
#define ADS1015_CONFIG_DR_250_SPS(byte) (byte & 0x1F) | 0x20
#define ADS1015_CONFIG_DR_490_SPS(byte) (byte & 0x1F) | 0x40
#define ADS1015_CONFIG_DR_920_SPS(byte) (byte & 0x1F) | 0x60
#define ADS1015_CONFIG_DR_1600_SPS(byte) (byte & 0x1F) | 0x80
#define ADS1015_CONFIG_DR_2400_SPS(byte) (byte & 0x1F) | 0xA0
#define ADS1015_CONFIG_DR_3300_SPS(byte) (byte & 0x1F) | 0xC0

#define ADS1015_CONFIG_COMP_QUE_ONE(byte) ((byte & 0xFC) | 0x0)
#define ADS1015_CONFIG_COMP_QUE_TWO(byte) ((byte & 0xFC) | 0x1)
#define ADS1015_CONFIG_COMP_QUE_FOUR(byte) ((byte & 0xFC) | 0x2)

#define ADS1015_HI_THRESH_RDY 0x80
#define ADS1015_LO_THRESH_RDY 0x00

// ADS1015 Full scale voltages
#define ADS1015_FULL_SCALE_6144(byte) (byte & 0xF1) | 0x0
#define ADS1015_FULL_SCALE_4096(byte) (byte & 0xF1) | 0x2
#define ADS1015_FULL_SCALE_2048(byte) (byte & 0xF1) | 0x4
#define ADS1015_FULL_SCALE_1024(byte) (byte & 0xF1) | 0x6
#define ADS1015_FULL_SCALE_512(byte) (byte & 0xF1) | 0x8
#define ADS1015_FULL_SCALE_256(byte) (byte & 0xF1) | 0xA

// ADS1015 voltage conversion constants
#define ADS1015_REFERENCE_VOLTAGE_6144 6144
#define ADS1015_REFERENCE_VOLTAGE_4096 4096
#define ADS1015_REFERENCE_VOLTAGE_2048 2048
#define ADS1015_REFERENCE_VOLTAGE_1024 1024
#define ADS1015_REFERENCE_VOLTAGE_512 512
#define ADS1015_REFERENCE_VOLTAGE_256 256

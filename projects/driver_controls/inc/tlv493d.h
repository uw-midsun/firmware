#pragma once

// TLV493D default I2C address definition (Datasheet Chapter 4.2)
#define TLV493D_ADDRESS   0x5E

// TLV493D read registers (Datasheet Chapter 7.1)
#define TLV493D_BX        0x0
#define TLV493D_BY        0x1
#define TLV493D_BZ        0x2
#define TLV493D_TEMP      0x3
#define TLV493D_BX2       0x4
#define TLV493D_BZ2       0x5
#define TLV493D_TEMP2     0x6
#define TLV493D_FACTSET1  0x7
#define TLV493D_FACTSET2  0x8
#define TLV493D_FACTSET3  0x9

#define NUM_TLV493D_READ_REGISTERS 10

// TLV493D read registers (Datasheet Chapter 7.1)
#define TLV493D_RES1      0x0
#define TLV493D_MOD1      0x1
#define TLV493D_RES2      0x2
#define TLV493D_MOD2      0x3

#define NUM_TLV493D_WRITE_REGISTERS 4

// Constant to convert from LSB to microteslas (Datasheet chapter 3.1)
#define TLV493D_LSB_TO_TESLA 98

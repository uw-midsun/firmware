#pragma once

// TLV493D default I2C address definition (Datasheet Chapter 4.2)
#define TLV493D_ADDRESS   0x5E

// TLV493D register definitions (Datasheet Chapter 7.1)
typedef enum {
  TLV493D_BX = 0,
  TLV493D_BY,
  TLV493D_BZ,
  TLV493D_TEMP,
  TLV493D_BX2,
  TLV493D_BZ2,
  TLV493D_TEMP2,
  TLV493D_FACTSET1,
  TLV493D_FACTSET2,
  TLV493D_FACTSET3,
  NUM_TLV493D_READ_REGISTERS,
  TLV493D_RES1 = 0,
  TLV493D_MOD1,
  TLV493D_RES2,
  TLV493D_MOD2,
  NUM_TLV493D_WRITE_REGISTERS
} TLV493DRegister;

// TLV493D register bitmasks
#define TLV493D_MASTER_CONTROLLED_MODE  0x3

#define TLV493D_FACTSET1_MASK           0x18
#define TLV493D_FACTSET3_MASK           0x1F


// Constant to convert from LSB to microteslas (Datasheet chapter 3.1)
#define TLV493D_LSB_TO_TESLA 98

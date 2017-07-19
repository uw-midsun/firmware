#include "crc15.h"

// x^{15} + x^{14} + x^{10} + x^{8} + x^{7} + x^{4} + x^{3} + x^{0}
// so divisor is: 0b1100010110011001 (0xC599)
// 0xC599 - (2^15) == 0x4599
#define CRC_POLYNOMIAL 0x4599

static uint16_t s_crc15_table[256];

void crc15_init_table(void) {
  uint32_t remainder;
  for (uint16_t i = 0; i < 256; ++i) {
    remainder = i << 7;
    for (uint8_t bit = 8; bit > 0; --bit) {
      if (remainder & 0x4000) {
        // check MSB
        remainder = (remainder << 1);
        remainder = (remainder ^ CRC_POLYNOMIAL);
      } else {
        remainder = (remainder << 1);
      }
    }

    s_crc15_table[i] = remainder & 0xFFFF;
  }
}

uint16_t crc15_calculate(uint8_t *data, size_t len) {
  // CRC should be initialized to 16 (see datasheet p.44)
  uint16_t remainder = 16;
  uint16_t addr = 0;

  for (size_t i = 0; i < len; i++) {
    addr = ((remainder >> 7) ^ data[i]) & 0xFF;
    remainder = (remainder << 8) ^ s_crc15_table[addr];
  }

  return remainder << 1;
}

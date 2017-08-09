#include "magnetic_sensor.h"
#include "tlv493d.h"

// Registers need to be read out and stored so that configuration registers aren't
// accidentally overwritten on write
static uint8_t s_read_reg[NUM_TLV493D_READ_REGISTERS];
static uint8_t s_write_reg[NUM_TLV493D_WRITE_REGISTERS];

// Returns reading in microteslas. Conversion method described in chapter 3.1 of the datasheet
static int16_t prv_flux_conversion(uint8_t msb, uint8_t lsb) {
  uint16_t data = (msb << 4) | (lsb & 0xF);

  if (data > 1024) {
    data -= 4096;
  }

  return (data * TLV493D_LSB_TO_TESLA);
}

StatusCode magnetic_sensor_init(I2CPort i2c_port) {
  // Reset device by calling address 0x0
  status_ok_or_return(i2c_read(i2c_port, 0x0, s_read_reg, 0));

  // Obtain the values of the read registers
  status_ok_or_return(i2c_read(i2c_port, TLV493D_ADDRESS, s_read_reg,
                                NUM_TLV493D_READ_REGISTERS));

  // Correctly configure the write bits
  s_write_reg[TLV493D_RES1] = 0x00;

  // FACTSET1 [4:3] -> MOD1 [4:3]
  s_write_reg[TLV493D_MOD1] = (s_read_reg[TLV493D_FACTSET1] & 0x18) | 0x3;

  // FACTSET2 [7:0] -> RES2 [7:0]
  s_write_reg[TLV493D_RES2] = s_read_reg[TLV493D_FACTSET2];

  // FACTSET3 [4:0] -> MOD2 [4:0]
  s_write_reg[TLV493D_MOD2] = (s_read_reg[TLV493D_FACTSET2] & 0x1F);

  status_ok_or_return(i2c_write(i2c_port, TLV493D_ADDRESS, s_write_reg,
                                NUM_TLV493D_WRITE_REGISTERS));

  return STATUS_CODE_OK;
}

StatusCode magnetic_sensor_read_data(I2CPort i2c_port, int16_t *reading) {
  status_ok_or_return(i2c_read(i2c_port, TLV493D_ADDRESS, s_read_reg,
                      NUM_TLV493D_READ_REGISTERS));

  reading[TLV493D_BX] = prv_flux_conversion(s_read_reg[TLV493D_BX],
                                            s_read_reg[TLV493D_BX2] >> 4);

  reading[TLV493D_BY] = prv_flux_conversion(s_read_reg[TLV493D_BY],
                                            s_read_reg[TLV493D_BX2] & 0xF);

  reading[TLV493D_BZ] = prv_flux_conversion(s_read_reg[TLV493D_BZ],
                                            s_read_reg[TLV493D_BZ2] & 0xF);

  return STATUS_CODE_OK;
}

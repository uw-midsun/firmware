#include "magnetic_sensor.h"
#include "delay.h"
#include "tlv493d.h"

// Registers need to be read out and stored so that configuration registers
// aren't
// accidentally overwritten on write
static uint8_t s_read_reg[NUM_TLV493D_READ_REGISTERS];
static uint8_t s_write_reg[NUM_TLV493D_WRITE_REGISTERS];

// Returns reading in microteslas. Conversion method described in chapter 3.1 of
// the datasheet
static int16_t prv_flux_conversion(uint8_t msb, uint8_t lsb) {
  int16_t data = (msb << 4) | (lsb & 0xF);

  // If the 12th bit is set, use sign extension to get the proper negative value
  if (data & 2048) {
    data |= 0xF000;
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
  s_write_reg[TLV493D_WRITE_RES1] = 0x00;

  // FACTSET1 [4:3] -> MOD1 [4:3]
  s_write_reg[TLV493D_WRITE_MOD1] =
      (s_read_reg[TLV493D_READ_FACTSET1] & TLV493D_FACTSET1_MASK) |
      TLV493D_MASTER_CONTROLLED_MODE;

  // FACTSET2 [7:0] -> RES2 [7:0]
  s_write_reg[TLV493D_WRITE_RES2] = s_read_reg[TLV493D_READ_FACTSET2];

  // FACTSET3 [4:0] -> MOD2 [4:0]
  s_write_reg[TLV493D_WRITE_MOD2] =
      (s_read_reg[TLV493D_READ_FACTSET3] & TLV493D_FACTSET3_MASK);

  status_ok_or_return(i2c_write(i2c_port, TLV493D_ADDRESS, s_write_reg,
                                NUM_TLV493D_WRITE_REGISTERS));

  return STATUS_CODE_OK;
}

StatusCode magnetic_sensor_read_data(I2CPort i2c_port,
                                     MagneticSensorReading *reading) {
  status_ok_or_return(i2c_read(i2c_port, TLV493D_ADDRESS, s_read_reg,
                               NUM_TLV493D_READ_REGISTERS));

  reading->x = prv_flux_conversion(s_read_reg[TLV493D_READ_BX],
                                   s_read_reg[TLV493D_READ_BX2] >> 4);
  reading->y = prv_flux_conversion(s_read_reg[TLV493D_READ_BY],
                                   s_read_reg[TLV493D_READ_BX2] & 0xF);
  reading->z = prv_flux_conversion(s_read_reg[TLV493D_READ_BZ],
                                   s_read_reg[TLV493D_READ_BZ2] & 0xF);

  delay_ms(10);

  return STATUS_CODE_OK;
}

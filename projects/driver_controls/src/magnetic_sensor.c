#include "magnetic_sensor.h"
#include "delay.h"
#include "tlv493d.h"

// Returns reading in microteslas. Conversion method described in chapter 3.1 of the datasheet
static int16_t prv_flux_conversion(uint8_t msb, uint8_t lsb) {
  int16_t data = (msb << 4) | (lsb & 0xF);

  // If the 12th bit is set, use sign extension to get the proper negative value
  if (data & 2048) {
    data |= 0xF000;
  }

  return (data * TLV493D_LSB_TO_TESLA);
}

StatusCode magnetic_sensor_init(I2CPort i2c_port) {
  uint8_t read_regs[NUM_TLV493D_READ_REGISTERS] = { 0 };
  uint8_t write_regs[NUM_TLV493D_WRITE_REGISTERS] = { 0 };
  // Obtain the values of the read registers
  status_ok_or_return(i2c_read(i2c_port, TLV493D_ADDRESS, read_regs, NUM_TLV493D_READ_REGISTERS));

  // Correctly configure the write bits
  write_regs[TLV493D_WRITE_RES1] = 0x00;

  // FACTSET1 [4:3] -> MOD1 [4:3]
  write_regs[TLV493D_WRITE_MOD1] =
      (read_regs[TLV493D_READ_FACTSET1] & TLV493D_FACTSET1_MASK) | TLV493D_MASTER_CONTROLLED_MODE;

  // FACTSET2 [7:0] -> RES2 [7:0]
  write_regs[TLV493D_WRITE_RES2] = read_regs[TLV493D_READ_FACTSET2];

  // FACTSET3 [4:0] -> MOD2 [4:0]
  write_regs[TLV493D_WRITE_MOD2] = (read_regs[TLV493D_READ_FACTSET3] & TLV493D_FACTSET3_MASK);

  status_ok_or_return(
      i2c_write(i2c_port, TLV493D_ADDRESS, write_regs, NUM_TLV493D_WRITE_REGISTERS));

  return STATUS_CODE_OK;
}

StatusCode magnetic_sensor_read_data(I2CPort i2c_port, MagneticSensorReading *reading) {
  uint8_t read_regs[NUM_TLV493D_READ_REGISTERS] = { 0 };

  status_ok_or_return(i2c_read(i2c_port, TLV493D_ADDRESS, read_regs, NUM_TLV493D_READ_REGISTERS));

  reading->x = prv_flux_conversion(read_regs[TLV493D_READ_BX], read_regs[TLV493D_READ_BX2] >> 4);
  reading->y = prv_flux_conversion(read_regs[TLV493D_READ_BY], read_regs[TLV493D_READ_BX2] & 0xF);
  reading->z = prv_flux_conversion(read_regs[TLV493D_READ_BZ], read_regs[TLV493D_READ_BZ2] & 0xF);

  // TODO: why is this delay necessary?
  delay_ms(10);

  return STATUS_CODE_OK;
}

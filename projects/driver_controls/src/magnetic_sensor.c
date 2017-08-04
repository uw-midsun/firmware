#include "magnetic_sensor.h"
#include "tlv493d.h"

// TODO: Make config file for registers

// Registers need to be read out and stored so that configuration registers aren't
// accidentally overwritten on write
static uint8_t s_read_reg[NUM_TLV493D_READ_REGISTERS];
static uint8_t s_write_reg[NUM_TLV493D_WRITE_REGISTERS];

StatusCode magnetic_sensor_init(I2CPort i2c_port) {
  // Reset device by calling address 0x0
  i2c_read(i2c_port, 0x0, &data, 0);

  // Obtain the values of the read registers
  i2c_read(i2c_port, 0x4E, &s_read_reg, NUM_TLV493D_READ_REGISTERS);

  // Correctly configure the write bits
  s_write_reg[TLV493D_RES1] = 0x00,

  // FACTSET1 [4:3] -> MOD1 [4:3]
  s_write_reg[TLV493D_MOD1] = s_read_reg[TLV493D_FACTSET1] & 0x18;

  // FACTSET2 [7:0] -> RES2 [7:0]
  s_write_reg[TLV493D_RES2] = s_read_reg[TLV493D_FACTSET2]

  // FACTSET3 [4:0] -> MOD2 [4:0]
  s_write_reg[TLV493D_MOD2] = s_read_reg[TLV493D_FACTSET3] & 0x1f;


  i2c_write(i2c_port, )
}

StatusCode magnetic_sensor_read_data(I2CPort i2c_port, TLV493DHallProbe probe) {
  uint8_t data[3];

  i2c_read(I2CPort i2c, 0x4E, data, 3)
}

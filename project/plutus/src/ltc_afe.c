#include "ltc_afe.h"
#include "crc15.h"


// write config to all devices
void prv_write_config(const LtcAfeSettings *afe, uint8_t config) {
  // see p.54 in datasheet
  // (2 bits for WRCFG + 2 bits for WRCFG PEC) +
  // (6 bits for CFGR + 2 bits for CFGR PEC) * devices_in_chain
  uint8_t configuration_cmd[4 + 8 * LTC_DEVICES_IN_CHAIN] = { 0 };
  uint8_t configuration_index = 0;

  // WRCFG
  configuration_cmd[configuration_index++] = 0x00;
  configuration_cmd[configuration_index++] = 0x01;

  uint16_t cmd_crc = crc15_calculate(configuration_cmd, 2);

  union {
    uint16_t data;
    uint8_t bits[2];
  } crc_bits;

  crc_bits.data = cmd_crc;

  // set PEC high bits
  configuration_cmd[configuration_index++] = crc_bits.bits[1];
  // set PEC low bits
  configuration_cmd[configuration_index++] = crc_bits.bits[0];

  // send CFGR registers starting with the bottom slave in the stack
  for (uint8_t device = afe->devices_in_chain; device > 0; --device) {
    uint8_t enable = LTC6804_GPIO1 | LTC6804_GPIO2 | LTC6804_GPIO3 | LTC6804_GPIO4 | LTC6804_GPIO5;
    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;
    uint16_t cells_to_discharge = 0;
    LtcDischargeTimeout timeout = LTC_AFE_DISCHARGE_TIMEOUT_DISABLED;

    // CFGR0, ..., CFGR5 + 2 bytes for PEC
    uint8_t cfgr_registers[8] = { 0 };
    // CFGR0
    configuration_cmd[configuration_index++] = enable;

    // CFGR1: VUV[7...0]
    configuration_cmd[configuration_index++] = (undervoltage & 0xFF);

    // CFGR2: VUV[11...8] in bit3, ..., bit0
    configuration_cmd[configuration_index] = ((undervoltage >> 8) & 0xF0);
    // CFGR2: VOV[3...0] in bit7, ..., bit4
    configuration_cmd[configuration_index++] |= (overvoltage & 0x0F);

    // CFGR3: VOV[11...4]
    configuration_cmd[configuration_index++] = ((overvoltage >> 4) & 0xFF);

    // CFGR4: DCC8, ..., DCC1
    configuration_cmd[configuration_index++] = cells_to_discharge & 0xFF;

    // CFGR5: DCC12, ..., DCC9
    configuration_cmd[configuration_index] = ((cells_to_discharge >> 8) & 0x0F);
    // CFGR5: DCTO5, ... DCTO0 in bit7, ..., bit4
    configuration_cmd[configuration_index++] |= (timeout << 4);

    // adjust the configuration_cmd pointer to point to the start of slave's configuration
    uint16_t cfgr_pec = crc15_calculate(configuration_cmd + (8 * (afe->devices_in_chain - device)) + 2, 6);
    configuration_cmd[configuration_index++] = (uint8_t)(cfgr_pec >> 8);
    configuration_cmd[configuration_index++] = (uint8_t)cfgr_pec;
  }

  //spi_exchange(afe->spi_port, cfgr_registers, 8, );
}

StatusCode LtcAfe_init(const LtcAfeSettings *afe) {
  crc15_init();

  SPISettings spi_config = {
    .baudrate = 1,
    .mode = SPI_MODE_3,
    .mosi = afe->mosi,
    .miso = afe->miso,
    .sclk = afe->sclk,
    .cs = afe->cs
  };
  spi_init(afe->spi_port, &spi_config);

  // write configuration register

  // set adc mode
  // (adc mode enum + 1) % 3
  // (adc mode enum + 1) > 3:
  //    - true: CFGR0[0] = 0
  //    - false: CFGR0[0] = 1


  return STATUS_CODE_OK;
}

// read back raw data from a single cell voltage register
// voltage_register is 0 indexed
StatusCode prv_read_voltage(LtcAfeSettings *afe, uint8_t voltage_register,
                            uint8_t *data) {
  if (voltage_register > 4) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t cmd[4] = { 0 };

  // Cell Voltage Register Group A: 0x04
  // Cell Voltage Register Group B: 0x06
  // Cell Voltage Register Group C: 0x08
  // Cell Voltage Register Group D: 0x0A
  cmd[0] = 0x00;
  cmd[1] = 0x04 + voltage_register;

  uint16_t cmd_crc = crc15_calculate(cmd, 2);

  union {
    uint16_t data;
    uint8_t bits[2];
  } crc_bits;

  crc_bits.data = cmd_crc;

  // set high bits
  cmd[2] = crc_bits.bits[1];
  // set low bits
  cmd[3] = crc_bits.bits[0];

  // wakeup idle.. is this necessary?

  // 6 bytes in register + 2 bytes for PEC
  spi_exchange(afe->spi_port, cmd, 4, data, 8 * afe->devices_in_chain);

  return STATUS_CODE_OK;
}

StatusCode LtcAfe_read_all_voltage(const LtcAfeSettings *afe, uint16_t *result_data) {
  StatusCode result_status = STATUS_CODE_OK;

  // TODO: is this the maximum data?
  // devices_in_chain * (12 * 2) +
  uint8_t afe_data[48] = { 0 };
  uint16_t data_counter = 0;
  for (uint8_t cell_reg = 1; cell_reg < 5; ++cell_reg) {
    prv_read_voltage(afe, cell_reg, afe_data);

    union {
      uint16_t voltage;
      uint8_t data[2];
    } cell_voltage;

    union {
      uint16_t data;
      uint8_t bytes[2];
    } packet_error_code;


    for (uint8_t afe_device = 0; afe_device < afe->devices_in_chain; ++afe_device) {
      // iterate through each cell each device is monitoring
      for (uint8_t cell = 0; cell < LTC_AFE_CELL_IN_REG; ++cell) {
        // set low bits
        cell_voltage.data[0] = afe_data[data_counter];
        // set high bits
        cell_voltage.data[1] = afe_data[data_counter + 1];

        // TODO: figure out how we're going to index this
        //result_data[][] = cell_voltage.voltage;
        data_counter += 2;
      }

      // the Packet Error Code is transmitted after the cell data (see p.45)
      uint16_t received_pec = (afe_data[data_counter] << 8)
                                + afe_data[data_counter + 1];

      //uint16_t data_pec = crc15_calculate(afe_data[afe_device * NUM_RX_BYTES]);
      //if (received_pec != data_pec) {
      //  result_status = STATUS_CODE_UNKNOWN;
      //}
      data_counter += 2;
    }
  }

  return result_status;
}

// aux_register is 0 indexed
StatusCode prv_read_aux_cmd(const LtcAfeSettings *afe, uint8_t aux_register, uint8_t data) {
  if (aux_register > 1) {
    return STATUS_CODE_INVALID_ARGS;
  }

  uint8_t cmd[4] = { 0 };

  // Auxiliary Register Group A: 0x0C (12)
  // Auxiliary Register Group B: 0x0E (14)
  cmd[4] = 0x0C + aux_register;

  uint16_t cmd_crc = crc15_calculate(cmd, 2);

  union {
    uint16_t data;
    uint8_t bits[2];
  } crc_bits;

  crc_bits.data = cmd_crc;

  // set high bits
  cmd[2] = crc_bits.bits[1];
  // set low bits
  cmd[3] = crc_bits.bits[0];

  // wakeup idle.. is this necessary?

  // get data
  spi_exchange(afe->spi_port, cmd, 4, data, 8 * afe->devices_in_chain);

  return STATUS_CODE_OK;
}

StatusCode LtcAfe_read_all_aux(const LtcAfeSettings *afe) {
  // GPIO
  return STATUS_CODE_OK;
}

StatusCode LtcAfe_toggle_discharge_cells(const LtcAfeSettings *afe, uint8_t device,
                                          uint8_t cell, bool discharge) {


  return STATUS_CODE_OK;
}

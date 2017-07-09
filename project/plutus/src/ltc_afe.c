#include "ltc_afe.h"
#include "delay.h"

static void prv_wakeup_idle(const LtcAfeSettings *afe) {
  gpio_toggle_state(&afe->cs);
  delay_us(2);
  gpio_toggle_state(&afe->cs);
}

// write config to all devices
void prv_write_config(const LtcAfeSettings *afe) {
  // see p.54 in datasheet
  // (2 bits for WRCFG + 2 bits for WRCFG PEC) +
  // (6 bits for CFGR + 2 bits for CFGR PEC) * devices_in_chain
  uint8_t configuration_cmd[4 + 8 * LTC_DEVICES_IN_CHAIN] = { 0 };
  uint8_t configuration_index = 0;

  // WRCFG
  configuration_cmd[configuration_index++] = 0x00;
  configuration_cmd[configuration_index++] = 0x01;

  uint16_t cmd_pec = crc15_calculate(configuration_cmd, 2);

  // set PEC high bits
  configuration_cmd[configuration_index++] = (uint8_t)(cmd_pec >> 8);
  // set PEC low bits
  configuration_cmd[configuration_index++] = (uint8_t)cmd_pec;

  // send CFGR registers starting with the bottom slave in the stack
  for (uint8_t device = afe->devices_in_chain; device > 0; --device) {
    // TODO(KARL-1): get these values from config struct
    uint8_t enable = LTC6804_GPIO1 | LTC6804_GPIO2 | LTC6804_GPIO3 | LTC6804_GPIO4 | LTC6804_GPIO5;
    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;
    uint16_t cells_to_discharge = 0;
    LtcDischargeTimeout timeout = LTC_AFE_DISCHARGE_TIMEOUT_DISABLED;

    // CFGR0, ..., CFGR5 + 2 bytes for PEC
    uint8_t cfgr_registers[8] = { 0 };
    // CFGR0
    configuration_cmd[configuration_index] = enable;
    // (adc mode enum + 1) > 3:
    //    - true: CFGR0[0] = 0
    //    - false: CFGR0[0] = 1
    // CFGR0: bit0 is the ADC Mode
    configuration_cmd[configuration_index++] |= !((afe->adc_mode + 1) > 3);

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
    uint8_t *cfg_offset = configuration_cmd + (8 * (afe->devices_in_chain - device)) + (2 + 2);
    uint16_t cfgr_pec = crc15_calculate(cfg_offset, 6);
    configuration_cmd[configuration_index++] = (uint8_t)(cfgr_pec >> 8);
    configuration_cmd[configuration_index++] = (uint8_t)cfgr_pec;
  }

  prv_wakeup_idle(afe);

  // don't care about SPI results
  uint8_t *cfg_offset = (2 + 2) + ((6 + 2) * afe->devices_in_chain);
  spi_exchange(afe->spi_port, configuration_cmd, cfg_offset, NULL, 0);
}

StatusCode LtcAfe_read_config(const LtcAfeSettings *afe) {
  StatusCode status = STATUS_CODE_OK;
  // RDCFG
  uint8_t cmd[4] = { 0 };
  // 6 bytes for data + 2 bytes for CRC
  uint8_t received_data[(6 + 2) * LTC_DEVICES_IN_CHAIN] = { 0 };

  uint8_t configuration_registers[6 * LTC_DEVICES_IN_CHAIN] = { 0 };

  cmd[0] = 0x00;
  cmd[1] = 0x02;
  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  // set PEC high bits
  cmd[3] = (uint8_t)(cmd_pec >> 8);
  cmd[4] = (uint8_t)cmd_pec;

  prv_wakeup_idle(afe);
  spi_exchange(afe->spi_port, cmd, 4, received_data, 8 * afe->devices_in_chain);

  uint16_t index = 0;
  for (uint8_t device = 0; device < afe->devices_in_chain; ++device) {
    for (uint8_t current_byte = 0; current_byte < 6; ++current_byte) {
      configuration_registers[index] = received_data[device * 8 + current_byte];
      index += 1;
    }

    uint16_t received_pec = ((uint16_t)received_data[device * 8 + 6] << 8) + received_data[device * 8 + 7];
    uint16_t calculated_pec = crc15_calculate(configuration_registers + (device * 8), 6);
    if (calculated_pec != received_pec) {
      status = STATUS_CODE_UNKNOWN;
    }
  }

  for (uint16_t i = 0; i < 6 * LTC_DEVICES_IN_CHAIN; ++i) {
    printf("Register %d: %d\n", i, configuration_registers[i]);
  }

  for (uint16_t i = 0; i < (6 + 2) * LTC_DEVICES_IN_CHAIN; ++i) {
    printf("Received %d: %d\n", i, received_data[i]);
  }

  return status;
}

StatusCode LtcAfe_init(const LtcAfeSettings *afe) {
  crc15_init_table();

  SPISettings spi_config = {
    .baudrate = 1000000,
    .mode = SPI_MODE_3,
    .mosi = afe->mosi,
    .miso = afe->miso,
    .sclk = afe->sclk,
    .cs = afe->cs
  };
  spi_init(afe->spi_port, &spi_config);

  prv_write_config(afe);

  return STATUS_CODE_OK;
}

// read back raw data from a single cell voltage register
// voltage_register is 0 indexed
StatusCode prv_read_voltage(LtcAfeSettings *afe, uint8_t voltage_register, uint8_t *data) {
  // RDCV{A,B,C,D}
  if (voltage_register > 4) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t cmd[4] = { 0 };

  // p. 49
  // RDCVA: 0x04 = 0x04 + 0x00
  // RDCVB: 0x06 = 0x04 + 0x02
  // RDCVC: 0x08 = 0x04 + 0x04
  // RDCVD: 0x0A = 0x04 + 0x06
  cmd[0] = 0x00;
  cmd[1] = 0x04 + (2 * voltage_register);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  // set high bits
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  // set low bits
  cmd[3] = (uint8_t)(cmd_pec);

  // wakeup idle.. is this necessary?
  prv_wakeup_idle(afe);

  // 6 bytes in register + 2 bytes for PEC
  spi_exchange(afe->spi_port, cmd, 4, data, (6 + 2) * afe->devices_in_chain);

  return STATUS_CODE_OK;
}

// start cell voltage conversion
static void prv_trigger_adc_conversion(const LtcAfeSettings *afe) {
  // ADCV command
  uint8_t cmd[4] = { 0 };
  cmd[0] = LTC6804_CNVT_CELL_ALL | LTC6804_ADCV_DISCHARGE_PERMITTED | LTC6804_ADCV_RESERVED;
  cmd[0] |= (uint8_t)((afe->adc_mode) % 3);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);

  prv_wakeup_idle(afe);

  spi_exchange(afe->spi_port, cmd, 4, NULL, 0);
}

StatusCode LtcAfe_read_all_voltage(const LtcAfeSettings *afe, uint16_t *result_data) {
  prv_trigger_adc_conversion(afe);

  StatusCode result_status = STATUS_CODE_OK;

  for (uint8_t cell_reg = 0; cell_reg < 4; ++cell_reg) {
    uint8_t afe_data[8 * LTC_DEVICES_IN_CHAIN] = { 0 };
    uint16_t data_counter = 0;

    prv_read_voltage(afe, cell_reg, afe_data);

    for (uint8_t afe_device = 0; afe_device < afe->devices_in_chain; ++afe_device) {
      for (uint16_t cell = 0; cell < LTC_AFE_CELLS_IN_REG; ++cell) {
        uint16_t cell_voltage = afe_data[data_counter] + (afe_data[data_counter + 1] << 8);
        result_data[afe_device * 12 + cell] = cell_voltage;

        data_counter += 2;
      }

      // the Packet Error Code is transmitted after the cell data (see p.45)
      uint16_t received_pec = (afe_data[data_counter] << 8)
                                + afe_data[data_counter + 1];

      uint16_t data_pec = crc15_calculate(&afe_data[afe_device * 8], 6);
      if (received_pec != data_pec) {
        result_status = STATUS_CODE_UNKNOWN;
        printf("Received: %d\n", received_pec);
        printf("Calculated: %d\n", data_pec);
      }
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
  cmd[4] = LTC6804_RDAUX_RESERVED | (aux_register << 1);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  // set high bits
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  // set low bits
  cmd[3] = (uint8_t)cmd_pec;

  // wakeup idle.. is this necessary?

  // get data
  spi_exchange(afe->spi_port, cmd, 4, data, (6 + 2) * afe->devices_in_chain);

  return STATUS_CODE_OK;
}

StatusCode LtcAfe_read_all_aux(const LtcAfeSettings *afe) {
  // GPIO
  uint8_t data_counter = 0;
  for (uint8_t gpio_register = 0; gpio_register < LTC_AFE_CELLS_IN_REG; gpio_register++) {
  }
  return STATUS_CODE_OK;
}

StatusCode LtcAfe_toggle_discharge_cells(const LtcAfeSettings *afe, uint8_t device,
                                          uint8_t cell, bool discharge) {
  return STATUS_CODE_OK;
}

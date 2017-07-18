#include "ltc_afe.h"
#include "delay.h"

static bool s_discharging_cells[LTC_CELLS_PER_DEVICE * LTC_DEVICES_IN_CHAIN] = { false };

static void prv_wakeup_idle(const LtcAfeSettings *afe) {
  gpio_set_pin_state(&afe->cs, GPIO_STATE_LOW);
  delay_us(2);
  gpio_set_pin_state(&afe->cs, GPIO_STATE_HIGH);
}

// write config to all devices
static void prv_write_config(const LtcAfeSettings *afe, uint8_t gpio_pins) {
  // see p.54 in datasheet
  // (2 bits for WRCFG + 2 bits for WRCFG PEC) +
  // (6 bits for CFGR + 2 bits for CFGR PEC) * devices_in_chain
  uint8_t configuration_cmd[(2 + 2) + (6 + 2) * LTC_DEVICES_IN_CHAIN] = { 0 };
  uint8_t configuration_index = 0;

  // WRCFG
  uint16_t wrcfg = LTC6804_WRCFG_RESERVED;
  configuration_cmd[configuration_index + 0] = (uint8_t)(wrcfg >> 8);
  configuration_cmd[configuration_index + 1] = (uint8_t)(wrcfg & 0xFF);

  uint16_t cmd_pec = crc15_calculate(configuration_cmd, 2);

  // set PEC high bits
  configuration_cmd[configuration_index + 2] = (uint8_t)(cmd_pec >> 8);
  // set PEC low bits
  configuration_cmd[configuration_index + 3] = (uint8_t)(cmd_pec & 0xFF);

  configuration_index += 4;
  // send CFGR registers starting with the bottom slave in the stack
  for (uint8_t device = LTC_DEVICES_IN_CHAIN; device > 0; --device) {
    uint8_t enable = gpio_pins;
    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;
    uint16_t cells_to_discharge = 0;
    for (uint8_t cell = 0; cell < LTC_CELLS_PER_DEVICE; ++cell) {
      cells_to_discharge |= (s_discharging_cells[(LTC_DEVICES_IN_CHAIN - device) * LTC_CELLS_PER_DEVICE + cell] << cell);
    }
    LtcDischargeTimeout timeout = LTC_AFE_DISCHARGE_TIMEOUT_DISABLED;

    // CFGR0
    configuration_cmd[configuration_index + 0] = enable;
    // (adc mode enum + 1) > 3:
    //    - true: CFGR0[0] = 1
    //    - false: CFGR0[0] = 0
    // CFGR0: bit0 is the ADC Mode
    configuration_cmd[configuration_index + 0] |= ((afe->adc_mode + 1) > 3);

    // CFGR1: VUV[7...0]
    configuration_cmd[configuration_index + 1] = (undervoltage & 0xFF);

    // CFGR2: VUV[11...8] in bit3, ..., bit0
    configuration_cmd[configuration_index + 2] = ((undervoltage >> 8) & 0x0F);
    // CFGR2: VOV[3...0] in bit7, ..., bit4
    configuration_cmd[configuration_index + 2] |= ((overvoltage << 4) & 0xF0);

    // CFGR3: VOV[11...4]
    configuration_cmd[configuration_index + 3] = ((overvoltage >> 4) & 0xFF);

    // CFGR4: DCC8, ..., DCC1
    configuration_cmd[configuration_index + 4] = cells_to_discharge & 0xFF;

    // CFGR5: DCC12, ..., DCC9
    configuration_cmd[configuration_index + 5] = ((cells_to_discharge >> 8) & 0x0F);
    // CFGR5: DCTO5, ... DCTO0 in bit7, ..., bit4
    configuration_cmd[configuration_index + 5] |= (timeout << 4);

    // adjust the offset to point to the start of slave's configuration
    uint8_t *cfg_offset = configuration_cmd + (8 * (LTC_DEVICES_IN_CHAIN - device)) + (2 + 2);
    uint16_t cfgr_pec = crc15_calculate(cfg_offset, 6);
    configuration_cmd[configuration_index + 6] = (uint8_t)(cfgr_pec >> 8);
    configuration_cmd[configuration_index + 7] = (uint8_t)(cfgr_pec & 0xFF);

    configuration_index += 8;
  }

  prv_wakeup_idle(afe);

  // don't care about SPI results
  uint8_t cfg_len = (2 + 2) + ((6 + 2) * LTC_DEVICES_IN_CHAIN);
  spi_exchange(afe->spi_port, configuration_cmd, cfg_len, NULL, 0);
}

// recieved_data should be len((6 + 2) * LTC_DEVICES_IN_CHAIN)
StatusCode LtcAfe_read_config(const LtcAfeSettings *afe, uint8_t *configuration_registers) {
  StatusCode status = STATUS_CODE_OK;
  // RDCFG
  uint16_t rdcfg = LTC6804_RDCFG_RESERVED;
  uint8_t cmd[4] = { 0 };

  cmd[0] = (uint8_t)(rdcfg >> 8);
  cmd[1] = (uint8_t)(rdcfg & 0xFF);
  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  // set PEC high bits
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec & 0xFF);

  prv_wakeup_idle(afe);
  uint8_t received_data[8 * LTC_DEVICES_IN_CHAIN] = { 0 };
  spi_exchange(afe->spi_port, cmd, 4, received_data, (6 + 2) * LTC_DEVICES_IN_CHAIN);

  uint16_t index = 0;
  for (uint8_t device = 0; device < LTC_DEVICES_IN_CHAIN; ++device) {
    for (uint8_t current_byte = 0; current_byte < 6; ++current_byte) {
      configuration_registers[index] = received_data[device * (6 + 2) + current_byte];
      index += 1;
    }

    uint16_t received_pec = ((uint16_t)received_data[device * (6 + 2) + 6] << 8) + received_data[device * (6 + 2) + 7];
    uint16_t calculated_pec = crc15_calculate(received_data + (device * (6 + 2)), 6);
    if (calculated_pec != received_pec) {
      status = STATUS_CODE_UNKNOWN;
    }
  }

  return status;
}

StatusCode LtcAfe_init(const LtcAfeSettings *afe) {
  crc15_init_table();

  SPISettings spi_config = {
    .baudrate = 250000, // TODO: fix SPI baudrate
    .mode = SPI_MODE_3,
    .mosi = afe->mosi,
    .miso = afe->miso,
    .sclk = afe->sclk,
    .cs = afe->cs
  };
  spi_init(afe->spi_port, &spi_config);

  uint8_t gpio_pins = LTC6804_GPIO1_PD_ON | LTC6804_GPIO2_PD_ON | LTC6804_GPIO3_PD_ON | LTC6804_GPIO4_PD_ON | LTC6804_GPIO5_PD_ON;
  prv_write_config(afe, gpio_pins);

  return STATUS_CODE_OK;
}

// read back raw data from a single cell voltage register
// voltage_register is 0 indexed
StatusCode prv_read_voltage(LtcAfeSettings *afe, uint8_t voltage_register, uint8_t *data) {
  // RDCV{A,B,C,D}
  if (voltage_register > 4) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // p. 49
  // RDCVA: 0x04 = 0x04 + 0x00
  // RDCVB: 0x06 = 0x04 + 0x02
  // RDCVC: 0x08 = 0x04 + 0x04
  // RDCVD: 0x0A = 0x04 + 0x06
  uint16_t rdcv = 0x04 + (2 * voltage_register);
  uint8_t cmd[4] = { 0 };

  cmd[0] = (uint8_t)(rdcv >> 8);
  cmd[1] = (uint8_t)(rdcv & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  // set high bits
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  // set low bits
  cmd[3] = (uint8_t)(cmd_pec & 0xFF);

  prv_wakeup_idle(afe);

  // 6 bytes in register + 2 bytes for PEC
  spi_exchange(afe->spi_port, cmd, 4, data, (6 + 2) * LTC_DEVICES_IN_CHAIN);

  return STATUS_CODE_OK;
}

// start cell voltage conversion
static void prv_trigger_adc_conversion(const LtcAfeSettings *afe) {
  uint8_t mode = (uint8_t)((afe->adc_mode) % 3);
  // ADCV command
  uint16_t adcv = LTC6804_ADCV_RESERVED | LTC6804_ADCV_DISCHARGE_PERMITTED | LTC6804_CNVT_CELL_ALL | (mode << 7);
  uint8_t cmd[4] = { 0 };

  cmd[0] = adcv >> 8;
  cmd[1] = adcv & 0xFF;

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
    uint8_t afe_data[(6 + 2) * LTC_DEVICES_IN_CHAIN] = { 0 };
    uint16_t data_counter = 0;

    prv_read_voltage(afe, cell_reg, afe_data);

    for (uint8_t afe_device = 0; afe_device < LTC_DEVICES_IN_CHAIN; ++afe_device) {
      for (uint16_t cell = 0; cell < LTC_AFE_CELLS_IN_REG; ++cell) {
        uint16_t cell_voltage = (uint16_t)afe_data[data_counter] + (uint16_t)(afe_data[data_counter + 1] << 8);
        result_data[afe_device * 12 + cell + (cell_reg * 3)] = cell_voltage;

        data_counter += 2;
      }

      // the Packet Error Code is transmitted after the cell data (see p.45)
      uint16_t received_pec = (afe_data[data_counter] << 8)
                                + afe_data[data_counter + 1];

      uint16_t data_pec = crc15_calculate(&afe_data[afe_device * 8], 6);
      if (received_pec != data_pec) {
        result_status = STATUS_CODE_UNKNOWN;
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

  // RDAUX
  // Auxiliary Register Group A: 0x0C (12)
  // Auxiliary Register Group B: 0x0E (14)
  uint16_t rdaux = LTC6804_RDAUX_RESERVED | (aux_register << 1);
  uint8_t cmd[4] = { 0 };

  cmd[0] = (uint8_t)(rdaux >> 8);
  cmd[1] = (uint8_t)(rdaux & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  // set high bits
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  // set low bits
  cmd[3] = (uint8_t)(cmd_pec & 0xFF);

  prv_wakeup_idle(afe);

  spi_exchange(afe->spi_port, cmd, 4, data, (6 + 2) * LTC_DEVICES_IN_CHAIN);

  return STATUS_CODE_OK;
}

StatusCode LtcAfe_read_all_aux(const LtcAfeSettings *afe) {
  uint16_t result_data[LTC_CELLS_PER_DEVICE * LTC_DEVICES_IN_CHAIN] = { 0 };
  for (uint8_t cell = 0; cell < LTC_CELLS_PER_DEVICE; ++cell) {
    // setup GPIO5, ..., GPIO2 in CFG register with binary representation of cell
    prv_write_config(afe, cell);

    uint8_t register_data[6 * LTC_DEVICES_IN_CHAIN] = { 0 };

    // RDAUXA
    prv_read_aux_cmd(afe, 0, register_data);

    // read GPIO1 data (we only care about AVAR0 and AVAR1), which actually
    // corresponds to the cell specified by GPIO5, GPIO4, GPIO3, GPIO2 in binary
    for (uint8_t device = 0; device < LTC_DEVICES_IN_CHAIN; ++device) {
      result_data[device * 12 + cell] = (uint16_t)register_data[6 * device] | ((uint16_t)register_data[6 * device + 1] << 8);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode LtcAfe_toggle_discharge_cells(const LtcAfeSettings *afe, uint16_t cell, bool discharge) {
  if (cell < LTC_CELLS_PER_DEVICE * LTC_DEVICES_IN_CHAIN) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_discharging_cells[cell] = discharge;

  return STATUS_CODE_OK;
}

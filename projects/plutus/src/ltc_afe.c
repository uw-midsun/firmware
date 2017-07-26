#include "crc15.h"
#include "delay.h"
#include "ltc_afe.h"
#include "ltc68041.h"

static bool s_discharging_cells[LTC_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN] = { false };

static uint16_t s_read_reg_cmd[NUM_LTC_AFE_REGISTER] = {
  LTC6804_RDCFG_RESERVED,
  LTC6804_RDCVA_RESERVED,
  LTC6804_RDCVB_RESERVED,
  LTC6804_RDCVC_RESERVED,
  LTC6804_RDCVD_RESERVED,
  LTC6804_RDAUXA_RESERVED,
  LTC6804_RDAUXA_RESERVED,
  LTC6804_RDSTATA_RESERVED,
  LTC6804_RDSTATB_RESERVED,
  LTC6804_RDCOMM_RESERVED
};

static uint8_t s_voltage_reg[NUM_LTC_AFE_VOLTAGE_REGISTER] = {
  LTC_AFE_REGISTER_CELL_VOLTAGE_A,
  LTC_AFE_REGISTER_CELL_VOLTAGE_B,
  LTC_AFE_REGISTER_CELL_VOLTAGE_C,
  LTC_AFE_REGISTER_CELL_VOLTAGE_D
};

static void prv_wakeup_idle(const LTCAFESettings *afe) {
  gpio_set_pin_state(&afe->cs, GPIO_STATE_LOW);
  delay_us(2);
  gpio_set_pin_state(&afe->cs, GPIO_STATE_HIGH);
}

static StatusCode prv_read_register(const LTCAFESettings *afe,
                                    LTCAFERegister reg, uint8_t *data, size_t len) {
  if (reg > NUM_LTC_AFE_REGISTER) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t reg_cmd = s_read_reg_cmd[reg];
  uint8_t cmd[4] = { 0 };

  cmd[0] = (uint8_t)(reg_cmd >> 8);
  cmd[1] = (uint8_t)(reg_cmd & 0xFF);
  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec & 0xFF);

  return spi_exchange(afe->spi_port, cmd, 4, data, len);
}

static StatusCode prv_read_voltage(LTCAFESettings *afe, LTCAFEVoltageRegister reg, uint8_t *data) {
  if (reg > NUM_LTC_AFE_VOLTAGE_REGISTER) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // 6 bytes in register + 2 bytes for PEC
  size_t len = ((LTC_AFE_CELLS_IN_REG * 2) + 2) * LTC_AFE_DEVICES_IN_CHAIN;

  return prv_read_register(afe, s_voltage_reg[reg], data, len);
}

// start cell voltage conversion
static void prv_trigger_adc_conversion(const LTCAFESettings *afe) {
  uint8_t mode = (uint8_t)((afe->adc_mode + 1) % 3);
  // ADCV command
  uint16_t adcv = LTC6804_ADCV_RESERVED | LTC6804_ADCV_DISCHARGE_PERMITTED
                  | LTC6804_CNVT_CELL_ALL | (mode << 7);
  uint8_t cmd[4] = { 0 };

  cmd[0] = (uint8_t)(adcv >> 8);
  cmd[1] = (uint8_t)(adcv & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);

  prv_wakeup_idle(afe);

  spi_exchange(afe->spi_port, cmd, 4, NULL, 0);

  // wait for conversions to finish
  delay_ms(100);
}

static void prv_trigger_aux_adc_conversion(const LTCAFESettings *afe) {
  uint8_t mode = (uint8_t)((afe->adc_mode + 1) % 3);
  // ADAX
  uint16_t adax = LTC6804_ADAX_RESERVED | LTC6804_ADAX_GPIO1 | (mode << 7);
  uint8_t cmd[4] = { 0 };
  cmd[0] = (uint8_t)(adax >> 8);
  cmd[1] = (uint8_t)(adax & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);

  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec & 0xFF);

  spi_exchange(afe->spi_port, cmd, 4, NULL, 0);

  // wait for conversions to finish
  delay_ms(10);
}

StatusCode ltc_afe_init(const LTCAFESettings *afe) {
  crc15_init_table();

  SPISettings spi_config = {
    .baudrate = 250000,
    .mode = SPI_MODE_3,
    .mosi = afe->mosi,
    .miso = afe->miso,
    .sclk = afe->sclk,
    .cs = afe->cs
  };
  spi_init(afe->spi_port, &spi_config);

  uint8_t gpio_bits = LTC6804_GPIO1_PD_OFF | LTC6804_GPIO2_PD_ON
                      | LTC6804_GPIO3_PD_ON | LTC6804_GPIO4_PD_ON
                      | LTC6804_GPIO5_PD_ON;
  ltc_afe_write_config(afe, gpio_bits);

  return STATUS_CODE_OK;
}

// write config to all devices
StatusCode ltc_afe_write_config(const LTCAFESettings *afe, uint8_t gpio_enable_pins) {
  // see p.54 in datasheet
  // (2 bits for WRCFG + 2 bits for WRCFG PEC) +
  // (6 bits for CFGR + 2 bits for CFGR PEC) * LTC_AFE_DEVICES_IN_CHAIN
  uint8_t configuration_cmd[(2 + 2) + (6 + 2) * LTC_AFE_DEVICES_IN_CHAIN] = { 0 };
  uint8_t cfg_len = (2 + 2) + ((6 + 2) * LTC_AFE_DEVICES_IN_CHAIN);
  uint8_t configuration_index = 0;

  // WRCFG
  uint16_t wrcfg = LTC6804_WRCFG_RESERVED;
  configuration_cmd[configuration_index + 0] = (uint8_t)(wrcfg >> 8);
  configuration_cmd[configuration_index + 1] = (uint8_t)(wrcfg & 0xFF);

  uint16_t cmd_pec = crc15_calculate(configuration_cmd, 2);

  configuration_cmd[configuration_index + 2] = (uint8_t)(cmd_pec >> 8);
  configuration_cmd[configuration_index + 3] = (uint8_t)(cmd_pec & 0xFF);

  configuration_index += 4;
  // send CFGR registers starting with the bottom slave in the stack
  for (uint8_t device = LTC_AFE_DEVICES_IN_CHAIN; device > 0; --device) {
    uint8_t current_device = LTC_AFE_DEVICES_IN_CHAIN - device;
    uint8_t enable = gpio_enable_pins;
    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;
    uint16_t cells_to_discharge = 0;

    for (uint8_t cell = 0; cell < LTC_CELLS_PER_DEVICE; ++cell) {
      uint16_t cell_index = current_device * LTC_CELLS_PER_DEVICE + cell;
      cells_to_discharge |= (s_discharging_cells[cell_index] << cell);
    }
    LTCAFEDischargeTimeout timeout = LTC_AFE_DISCHARGE_TIMEOUT_DISABLED;

    // CFGR0
    configuration_cmd[configuration_index + 0] = enable;
    // (adc mode enum + 1) > 3:
    //    - true: CFGR0[0] = 1
    //    - false: CFGR0[0] = 0
    // CFGR0: bit0 is the ADC Mode
    configuration_cmd[configuration_index + 0] |= ((afe->adc_mode + 1) > 3);
    configuration_cmd[configuration_index + 0] |= LTC6804_SWTRD;

    // CFGR1: VUV[7...0]
    configuration_cmd[configuration_index + 1] = (undervoltage & 0xFF);

    // CFGR2: VUV[11...8] in bit3, ..., bit0
    configuration_cmd[configuration_index + 2] = ((undervoltage >> 8) & 0x0F);
    // CFGR2: VOV[3...0] in bit7, ..., bit4
    configuration_cmd[configuration_index + 2] |= ((overvoltage << 4) & 0xF0);

    // CFGR3: VOV[11...4]
    configuration_cmd[configuration_index + 3] = ((overvoltage >> 4) & 0xFF);

    // CFGR4: DCC8, ..., DCC1
    configuration_cmd[configuration_index + 4] = (cells_to_discharge & 0xFF);

    // CFGR5: DCC12, ..., DCC9
    configuration_cmd[configuration_index + 5] = ((cells_to_discharge >> 8) & 0x0F);
    // CFGR5: DCTO5, ... DCTO0 in bit7, ..., bit4
    configuration_cmd[configuration_index + 5] |= (timeout << 4);

    // adjust the offset to point to the start of slave's configuration
    uint8_t *offset = configuration_cmd + (2 + 2) + ((6 + 2) * current_device);
    uint16_t cfgr_pec = crc15_calculate(offset, 6);
    configuration_cmd[configuration_index + 6] = (uint8_t)(cfgr_pec >> 8);
    configuration_cmd[configuration_index + 7] = (uint8_t)(cfgr_pec & 0xFF);

    configuration_index += 8;
  }

  prv_wakeup_idle(afe);

  return spi_exchange(afe->spi_port, configuration_cmd, cfg_len, NULL, 0);
}

StatusCode ltc_afe_read_config(const LTCAFESettings *afe, uint8_t *configuration_registers) {
  StatusCode status = STATUS_CODE_OK;

  prv_wakeup_idle(afe);

  uint8_t received_data[(6 + 2) * LTC_AFE_DEVICES_IN_CHAIN] = { 0 };
  size_t len = (6 + 2) * LTC_AFE_DEVICES_IN_CHAIN;

  prv_read_register(afe, LTC_AFE_REGISTER_CONFIG, received_data, len);

  uint16_t index = 0;
  for (uint8_t device = 0; device < LTC_AFE_DEVICES_IN_CHAIN; ++device) {
    for (uint8_t current_byte = 0; current_byte < 6; ++current_byte) {
      configuration_registers[index] = received_data[device * (6 + 2) + current_byte];
      index += 1;
    }

    uint16_t received_pec = (uint16_t)(received_data[device * (6 + 2) + 6] << 8)
                             + received_data[device * (6 + 2) + 7];
    uint16_t calculated_pec = crc15_calculate(received_data + (device * (6 + 2)), 6);
    if (calculated_pec != received_pec) {
      status = status_code(STATUS_CODE_UNKNOWN);
    }
  }

  return status;
}

StatusCode ltc_afe_read_all_voltage(const LTCAFESettings *afe, uint16_t *result_data, size_t len) {
  if (len != LTC_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  StatusCode result_status = STATUS_CODE_OK;

  prv_trigger_adc_conversion(afe);

  for (uint8_t cell_reg = 0; cell_reg < 4; ++cell_reg) {
    uint8_t afe_data[((LTC_AFE_CELLS_IN_REG * 2) + 2) * LTC_AFE_DEVICES_IN_CHAIN] = { 0 };
    uint16_t data_counter = 0;

    prv_read_voltage(afe, cell_reg, afe_data);

    for (uint8_t device = 0; device < LTC_AFE_DEVICES_IN_CHAIN; ++device) {
      for (uint16_t cell = 0; cell < LTC_AFE_CELLS_IN_REG; ++cell) {
        // LSB of the reading is 100 uV
        uint16_t voltage = (uint16_t)afe_data[data_counter]
                            + (uint16_t)(afe_data[data_counter + 1] << 8);
        uint16_t index = device * LTC_CELLS_PER_DEVICE + cell + (cell_reg * LTC_AFE_CELLS_IN_REG);
        result_data[index] = voltage;

        data_counter += 2;
      }

      // the Packet Error Code is transmitted after the cell data (see p.45)
      uint16_t received_pec = (afe_data[data_counter] << 8) + afe_data[data_counter + 1];
      uint16_t data_pec = crc15_calculate(&afe_data[device * 8], 6);
      if (received_pec != data_pec) {
        result_status = status_code(STATUS_CODE_UNKNOWN);
      }
      data_counter += 2;
    }
  }

  return result_status;
}

StatusCode ltc_afe_read_all_aux(const LTCAFESettings *afe, uint16_t *result_data) {
  StatusCode result_status = STATUS_CODE_OK;

  for (uint8_t cell = 0; cell < 12; ++cell) {
    // configure the mux to read from cell
    ltc_afe_write_config(afe, (cell << 4) | LTC6804_GPIO1_PD_OFF);

    prv_trigger_aux_adc_conversion(afe);

    uint8_t register_data[((LTC_AFE_GPIOS_IN_REG * 2) + 2) * LTC_AFE_DEVICES_IN_CHAIN] = { 0 };
    size_t len = ((LTC_AFE_GPIOS_IN_REG * 2) + 2) * LTC_AFE_DEVICES_IN_CHAIN;
    prv_read_register(afe, LTC_AFE_REGISTER_AUX_A, register_data, len);

    for (uint16_t device = 0; device < LTC_AFE_DEVICES_IN_CHAIN; ++device) {
      // data comes in in the form { 1, 1, 2, 2, 3, 3, PEC, PEC }
      // we only care about GPIO1 and the PEC
      uint16_t voltage = (uint16_t)(register_data[device * (LTC_AFE_GPIOS_IN_REG * 2) + 1] << 8)
                          + (uint16_t)register_data[device * (LTC_AFE_GPIOS_IN_REG * 2)];
      result_data[device * LTC_CELLS_PER_DEVICE + cell] = voltage;

      uint16_t received_pec = (register_data[device * (LTC_AFE_GPIOS_IN_REG * 2) + 6] << 8)
                              + register_data[device * (LTC_AFE_GPIOS_IN_REG * 2) + 7];
      uint16_t data_pec = crc15_calculate(register_data, 6);
      if (received_pec != data_pec) {
        result_status = status_code(STATUS_CODE_UNKNOWN);
      }
    }
  }

  return result_status;
}

StatusCode ltc_afe_toggle_discharge_cells(const LTCAFESettings *afe,
                                          uint16_t cell, bool discharge) {
  if (cell < LTC_CELLS_PER_DEVICE * LTC_AFE_DEVICES_IN_CHAIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_discharging_cells[cell] = discharge;

  return STATUS_CODE_OK;
}

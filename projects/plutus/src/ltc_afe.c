#include "crc15.h"
#include "delay.h"
#include "ltc_afe.h"
#include "ltc68041.h"
#include "plutus_config.h"

static bool s_discharging_cells[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN] = { false };

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

static void prv_build_cmd(uint16_t command, uint8_t *cmd) {
  cmd[0] = (uint8_t)(command >> 8);
  cmd[1] = (uint8_t)(command & 0xFF);

  uint16_t cmd_pec = crc15_calculate(cmd, 2);
  cmd[2] = (uint8_t)(cmd_pec >> 8);
  cmd[3] = (uint8_t)(cmd_pec);
}

static StatusCode prv_read_register(const LTCAFESettings *afe,
                                    LTCAFERegister reg, uint8_t *data, size_t len) {
  if (reg > NUM_LTC_AFE_REGISTER) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t reg_cmd = s_read_reg_cmd[reg];

  uint8_t cmd[4] = { 0 };
  prv_build_cmd(reg_cmd, cmd);

  prv_wakeup_idle(afe);
  return spi_exchange(afe->spi_port, cmd, 4, data, len);
}

// read from a voltage register
static StatusCode prv_read_voltage(LTCAFESettings *afe,
                                    LTCAFEVoltageRegister reg, LTCAFEVoltageRegisterGroup *data) {
  if (reg > NUM_LTC_AFE_VOLTAGE_REGISTER) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  size_t len = sizeof(LTCAFEVoltageRegisterGroup) * PLUTUS_AFE_DEVICES_IN_CHAIN;
  return prv_read_register(afe, s_voltage_reg[reg], (uint8_t *)data, len);
}

// start cell voltage conversion
static void prv_trigger_adc_conversion(const LTCAFESettings *afe) {
  uint8_t mode = (uint8_t)((afe->adc_mode + 1) % 3);
  // ADCV command
  uint16_t adcv = LTC6804_ADCV_RESERVED | LTC6804_ADCV_DISCHARGE_NOT_PERMITTED
                  | LTC6804_CNVT_CELL_ALL | (mode << 7);

  uint8_t cmd[4] = { 0 };
  prv_build_cmd(adcv, cmd);

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
  prv_build_cmd(adax, cmd);

  prv_wakeup_idle(afe);
  spi_exchange(afe->spi_port, cmd, 4, NULL, 0);

  // wait for conversions to finish
  delay_ms(10);
}

// write config to all devices
static StatusCode prv_write_config(const LTCAFESettings *afe, uint8_t gpio_enable_pins) {
  // see p.54 in datasheet
  LTCAFEWriteConfigPacket config_packet = { 0 };

  // WRCFG
  config_packet.wrcfg.data_hi = (uint8_t)(LTC6804_WRCFG_RESERVED >> 8);
  config_packet.wrcfg.data_lo = (uint8_t)(LTC6804_WRCFG_RESERVED & 0xFF);

  uint16_t cmd_pec = crc15_calculate((uint8_t *)&config_packet.wrcfg, 2);
  config_packet.wrcfg.pec_hi = (uint8_t)(cmd_pec >> 8);
  config_packet.wrcfg.pec_lo = (uint8_t)(cmd_pec & 0xFF);

  // essentially, each set of CFGR registers are clocked through each device,
  // until the first set reaches the last device (like a giant shift register)
  // thus, we send CFGR registers starting with the bottom slave in the stack
  for (uint8_t device = PLUTUS_AFE_DEVICES_IN_CHAIN; device > 0; --device) {
    uint8_t curr_device = PLUTUS_AFE_DEVICES_IN_CHAIN - device;
    uint8_t enable = gpio_enable_pins;
    uint16_t undervoltage = 0;
    uint16_t overvoltage = 0;

    uint16_t cell_index = curr_device * LTC6804_CELLS_PER_DEVICE;
    config_packet.devices[curr_device].reg.discharge_c1 = s_discharging_cells[cell_index + 0];
    config_packet.devices[curr_device].reg.discharge_c2 = s_discharging_cells[cell_index + 1];
    config_packet.devices[curr_device].reg.discharge_c3 = s_discharging_cells[cell_index + 2];
    config_packet.devices[curr_device].reg.discharge_c4 = s_discharging_cells[cell_index + 3];
    config_packet.devices[curr_device].reg.discharge_c5 = s_discharging_cells[cell_index + 4];
    config_packet.devices[curr_device].reg.discharge_c6 = s_discharging_cells[cell_index + 5];
    config_packet.devices[curr_device].reg.discharge_c7 = s_discharging_cells[cell_index + 6];
    config_packet.devices[curr_device].reg.discharge_c8 = s_discharging_cells[cell_index + 7];
    config_packet.devices[curr_device].reg.discharge_c9 = s_discharging_cells[cell_index + 8];
    config_packet.devices[curr_device].reg.discharge_c10 = s_discharging_cells[cell_index + 9];
    config_packet.devices[curr_device].reg.discharge_c11 = s_discharging_cells[cell_index + 10];
    config_packet.devices[curr_device].reg.discharge_c12 = s_discharging_cells[cell_index + 11];

    config_packet.devices[curr_device].reg.discharge_timeout = LTC_AFE_DISCHARGE_TIMEOUT_1_MIN;

    config_packet.devices[curr_device].reg.adcopt = ((afe->adc_mode + 1) > 3);
    config_packet.devices[curr_device].reg.swtrd = true;

    config_packet.devices[curr_device].reg.undervoltage = undervoltage;
    config_packet.devices[curr_device].reg.overvoltage = overvoltage;

    // GPIO5, ..., GPIO2 are used to MUX data
    config_packet.devices[curr_device].reg.gpio = (enable >> 3);

    uint16_t cfgr_pec = crc15_calculate((uint8_t *)&config_packet.devices[curr_device].reg, 6);
    config_packet.devices[curr_device].pec = SWAP_UINT16(cfgr_pec);
  }

  prv_wakeup_idle(afe);
  return spi_exchange(afe->spi_port, (uint8_t *)&config_packet,
                      sizeof(LTCAFEWriteConfigPacket), NULL, 0);
}

static StatusCode prv_read_config(const LTCAFESettings *afe,
                                  LTCAFEConfigRegisterData *configuration_registers) {
  prv_wakeup_idle(afe);

  LTCAFEWriteDeviceConfigPacket received_data[PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };

  size_t len = sizeof(received_data);
  prv_read_register(afe, LTC_AFE_REGISTER_CONFIG, (uint8_t *)received_data, len);

  for (uint8_t device = 0; device < PLUTUS_AFE_DEVICES_IN_CHAIN; ++device) {
    configuration_registers[device] = received_data[device].reg;

    uint16_t received_pec = SWAP_UINT16(received_data[device].pec);
    uint16_t calculated_pec = crc15_calculate((uint8_t *)&received_data[device].reg,
                                              sizeof(LTCAFEConfigRegisterData));
    if (calculated_pec != received_pec) {
      return status_code(STATUS_CODE_CRC_MISMATCH);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_init(const LTCAFESettings *afe) {
  crc15_init_table();

  SPISettings spi_config = {
    .baudrate = afe->spi_baudrate,
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
  prv_write_config(afe, gpio_bits);

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_all_voltage(const LTCAFESettings *afe, uint16_t *result_data, size_t len) {
  if (len != LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_trigger_adc_conversion(afe);

  for (uint8_t cell_reg = 0; cell_reg < NUM_LTC_AFE_VOLTAGE_REGISTER; ++cell_reg) {
    LTCAFEVoltageRegisterGroup voltage_register[PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };

    prv_read_voltage(afe, cell_reg, &voltage_register);

    for (uint8_t device = 0; device < PLUTUS_AFE_DEVICES_IN_CHAIN; ++device) {
      for (uint16_t cell = 0; cell < LTC6804_CELLS_IN_REG; ++cell) {
        // LSB of the reading is 100 uV
        uint16_t voltage = voltage_register[device].reg.voltages[cell];
        uint16_t index = device * LTC6804_CELLS_PER_DEVICE + cell +
                        (cell_reg * LTC6804_CELLS_IN_REG);
        result_data[index] = voltage;
      }

      // the Packet Error Code is transmitted after the cell data (see p.45)
      uint16_t received_pec = SWAP_UINT16(voltage_register[device].pec);
      uint16_t data_pec = crc15_calculate((uint8_t *)&voltage_register[device], 6);
      if (received_pec != data_pec) {
        // return early on failure
        return status_code(STATUS_CODE_CRC_MISMATCH);
      }
    }
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_read_all_aux(const LTCAFESettings *afe, uint16_t *result_data, size_t len) {
  if (len != LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  for (uint8_t cell = 0; cell < LTC6804_CELLS_PER_DEVICE; ++cell) {
    // configure the mux to read from cell
    // we use GPIO2, GPIO3, GPIO4, GPIO5 to select which input to read
    // corresponding to the binary representation of the cell
    prv_write_config(afe, (cell << 4) | LTC6804_GPIO1_PD_OFF);

    prv_trigger_aux_adc_conversion(afe);

    LTCAFEAuxRegisterGroupPacket register_data[PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };

    size_t len = sizeof(register_data);
    prv_read_register(afe, LTC_AFE_REGISTER_AUX_A, (uint8_t *)register_data, len);

    for (uint16_t device = 0; device < PLUTUS_AFE_DEVICES_IN_CHAIN; ++device) {
      // data comes in in the form { 1, 1, 2, 2, 3, 3, PEC, PEC }
      // we only care about GPIO1 and the PEC
      uint16_t voltage = register_data[device].reg.voltages[0];
      result_data[device * LTC6804_CELLS_PER_DEVICE + cell] = voltage;

      uint16_t received_pec = SWAP_UINT16(register_data[device].pec);
      uint16_t data_pec = crc15_calculate((uint8_t *)&register_data, 6);
      if (received_pec != data_pec) {
        return status_code(STATUS_CODE_CRC_MISMATCH);
      }
    }
  }

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_toggle_discharge_cells(const LTCAFESettings *afe,
                                          uint16_t cell, bool discharge) {
  if (cell > LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN - 1) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  s_discharging_cells[cell] = discharge;

  return STATUS_CODE_OK;
}

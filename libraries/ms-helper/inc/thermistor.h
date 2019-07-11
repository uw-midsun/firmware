#pragma once
// Thermistor Reading interface for NXRT15XH103FA1B (10k Ohm NTC)
// Requires ADC, GPIO and Interrupts to be initialized.
//
// The thermistor and its fixed resistor forms a voltage divider.
// A temperature value ranging from 0~100 degrees is calculated from the voltage divider.
#include "adc.h"
#include "gpio.h"

// A 10kOhm resistor is used as the fixed value
#define THERMISTOR_FIXED_RESISTANCE_OHMS 10000

// ThermistorPosition indicates which resistor the node voltage is measured from
// ex: VDDA ---> R1 ---> R2 ---> GND
typedef enum {
  THERMISTOR_POSITION_R1 = 0,
  THERMISTOR_POSITION_R2,
  NUM_THERMISTOR_POSITIONS,
} ThermistorPosition;

typedef enum {
  NXRT15XH103 = 0,  // 10k ohm nominal resistance thermistor
  NXRT15WF104,      // 100k ohm nominal resistance thermistor
  NUM_SUPPORTED_THERMISTOR_MODELS,
} ThermistorModel;

typedef struct ThermistorSettings {
  GpioAddress thermistor_gpio;
  ThermistorPosition position;
  ThermistorModel model;
  uint16_t dividor_resistor_ohms;
} ThermistorSettings;

typedef struct ThermistorStorage {
  ThermistorSettings *settings;
  AdcChannel adc_channel;
} ThermistorStorage;

// Initializes the GPIO pin and ADC Channel associated with the thermistor
StatusCode thermistor_init(ThermistorStorage *storage, ThermistorSettings *settings);

// Based on a voltage reading and dividor parameters, calculate the resistance of the thermistor.
StatusCode thermistor_get_resistance(ThermistorPosition position, uint16_t dividor_resistor_ohms,
                                     uint16_t reading_mv, uint16_t vdda_mv,
                                     uint32_t *thermistor_resistance_mohms);

// Calculate the temperature in deciCelsius from ohms
StatusCode thermistor_calculate_temp(ThermistorModel model, uint32_t thermistor_resistance_mohms,
                                     uint16_t *temperature_dc);

// Calculates the thermistor resistance given a certain temperature in dC
StatusCode thermistor_calculate_resistance_from_temp(ThermistorModel model, uint16_t temperature_dc,
                                                     uint16_t *thermistor_resistance_ohms);

// Fetch the temperature reading in deciCelsius from the MCU's ADC
// Note: "dc" (deciCelsius) is a tenth of a celsius (0.1C)
StatusCode thermistor_read_and_calculate_temp(ThermistorStorage *storage, uint16_t *temperature_dc);

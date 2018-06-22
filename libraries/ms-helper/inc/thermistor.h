#pragma once
// Thermistor Reading interface for NXRT15XH103FA1B (10k Ohm NTC)
// Requires ADC, GPIO and Interrupts to be initialized.
#include "adc.h"
#include "gpio.h"

// The thermistor and its fixed resistor forms a voltage divider.
// A temperature value ranging from 0~100 degrees is calculated from the voltage divider.

// ThermistorPosition indicates which resistor the node voltage is measured from
// ex: VDDA ---> R1 ---> Thermistor (Proceeding) --- GND
typedef enum {
  PRECEEDING = 0,  // Measured from R1
  PROCEEDING,      // Measured from thermistor
  NUM_THERMISTOR_POSITIONS,
} ThermistorPosition;

typedef struct ThermistorStorage {
  ThermistorPosition position;
  ADCChannel adc_channel;
} ThermistorStorage;

// Initialize the thermistor GPIO pins, and adc channels
StatusCode thermistor_init(ThermistorStorage *storage, GPIOAddress thermistor_gpio,
                           ThermistorPosition position);

// Fetch the temperature reading in milliCelsius from the MCU's ADC
StatusCode thermistor_get_temp(ThermistorStorage *storage, uint32_t *temperature_millicelcius);

// Calculate the temperature in milliCelsius from milliohms
StatusCode thermistor_calculate_temp(uint32_t thermistor_resistance_milliohms,
                                     uint32_t *temperature_millicelcius);

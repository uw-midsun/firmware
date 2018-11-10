#pragma once
#include <stdint.h>

// The following things shouldn't be used outside this module,
// but are public so they can be properly unit tested.

#define SOC_VOLTAGE_STEPS 20

// Since floating point is not supported,
// fractions are used for the few situations that cannot be rounded.
typedef struct SocFraction {
  int32_t numerator;
  int32_t denominator;
} SocFraction;

// Describes the behaviour of the battery.
// There are units for
// - Voltage
// - Current
// - Time
// - Charge
// - Resistance
// The scale of these units (milli, micro, etc.) are not important,
// except the unit of charge must be the product of the unit of current and time,
// and the unit of voltage must be the product of current and resistance.
typedef struct SocBatterySettings {
  // |minimum_voltage| and |voltage_step| are used to map voltages to indices in the
  // |voltage_to_charge| array.
  int32_t minimum_voltage;
  int32_t voltage_step;

  // Maps voltages to an expected charge.
  // It maps in this direction and not the other, since this direction puts more of the data points
  // where the curve is more accurate.
  int32_t voltage_to_charge[SOC_VOLTAGE_STEPS];

  // From those values, other important values can be calculated.
  // The maximum voltage can be determined.
  // The maximum and minimum charges can be determined. The coulomb counting can never go outside
  // these values.

  // How much the voltage can vary from what it should be, based on |voltage_to_charge|.
  // This can be caused by temperature variance, and the difference between charging and
  // discharging.
  int32_t voltage_inaccuracy;

  // Describes the ratio between the measured current and the effect the current actually has.
  // Seems some coulombs get lost somehow.
  SocFraction current_efficiency;

  // The battery has some resistance, so it's rest voltage varies from its apparent voltage.
  SocFraction internal_resistance;
} SocBatterySettings;

// These functions are all pure.
int32_t soc_minimum_charge(SocBatterySettings *batterySettings);
int32_t soc_maximum_charge(SocBatterySettings *batterySettings);
int32_t soc_minimum_voltage(SocBatterySettings *batterySettings);
int32_t soc_maximum_voltage(SocBatterySettings *batterySettings);
int32_t soc_multiply_fraction(int32_t value, SocFraction multiplier);
int32_t soc_charge_for_voltage(int32_t voltage, SocBatterySettings *batterySettings);
int32_t soc_minimum_charge_for_voltage(int32_t voltage, SocBatterySettings *batterySettings);
int32_t soc_maximum_charge_for_voltage(int32_t voltage, SocBatterySettings *batterySettings);
int32_t soc_current_adjusted_voltage(int32_t voltage, int32_t current,
                                     SocBatterySettings *batterySettings);
int32_t soc_charge_after_transition(int32_t old_charge, int32_t voltage, int32_t current,
                                    int32_t elapsed_time, SocBatterySettings *batterySettings);

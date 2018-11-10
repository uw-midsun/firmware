#include "state_of_charge.h"
#include <stdint.h>
#include <stdio.h>

// This multiplies the value by the fractional multiplier.
// This will obviously overflow if the result is too large.
// It will also overflow for 2,147,483,647 * 2,147,483,647/2,147,483,647, so don't do that.
// It rounds the result to the nearest integer, and .5s always round away from zero.
// If this bias becomes a problem, it should be changed to round in some other manner.
int32_t soc_multiply_fraction(int32_t value, SocFraction multiplier) {
  int64_t product = (int64_t)value * (int64_t)multiplier.numerator;
  int64_t round_corrected_product;
  // != is xor. This always rounds away from zero,
  // since integer truncation always goes towards zero.
  if ((product < 0) != (multiplier.denominator < 0)) {
    round_corrected_product = product - (int64_t)(multiplier.denominator / 2);
  } else {
    round_corrected_product = product + (int64_t)(multiplier.denominator / 2);
  }
  return (int32_t)(round_corrected_product / multiplier.denominator);
}

int32_t soc_minimum_charge(SocBatterySettings *batterySettings) {
  return batterySettings->voltage_to_charge[0];
}

int32_t soc_maximum_charge(SocBatterySettings *batterySettings) {
  return batterySettings->voltage_to_charge[SOC_VOLTAGE_STEPS - 1];
}

int32_t soc_minimum_voltage(SocBatterySettings *batterySettings) {
  return batterySettings->minimum_voltage;
}

int32_t soc_maximum_voltage(SocBatterySettings *batterySettings) {
  return batterySettings->minimum_voltage + (SOC_VOLTAGE_STEPS - 1) * batterySettings->voltage_step;
}

int32_t soc_charge_for_voltage(int32_t voltage, SocBatterySettings *batterySettings) {
  if (voltage <= soc_minimum_voltage(batterySettings)) {
    return soc_minimum_charge(batterySettings);
  }

  if (voltage >= soc_maximum_voltage(batterySettings)) {
    return soc_maximum_charge(batterySettings);
  }

  int32_t voltage_over_minimum = voltage - soc_minimum_voltage(batterySettings);

  int32_t step = voltage_over_minimum / batterySettings->voltage_step;
  SocFraction interpolation_amount = { voltage_over_minimum % batterySettings->voltage_step,
                                       batterySettings->voltage_step };

  int32_t charge_below = batterySettings->voltage_to_charge[step];
  int32_t charge_above = batterySettings->voltage_to_charge[step + 1];

  int32_t interpolated_amount =
      soc_multiply_fraction(charge_above - charge_below, interpolation_amount);

  return charge_below + interpolated_amount;
}

int32_t soc_minimum_charge_for_voltage(int32_t voltage, SocBatterySettings *batterySettings) {
  return soc_charge_for_voltage(voltage - batterySettings->voltage_inaccuracy, batterySettings);
}

int32_t soc_maximum_charge_for_voltage(int32_t voltage, SocBatterySettings *batterySettings) {
  return soc_charge_for_voltage(voltage + batterySettings->voltage_inaccuracy, batterySettings);
}

int32_t soc_current_adjusted_voltage(int32_t voltage, int32_t current,
                                     SocBatterySettings *batterySettings) {
  return voltage - soc_multiply_fraction(current, batterySettings->internal_resistance);
}

int32_t soc_charge_after_transition(int32_t old_charge, int32_t voltage, int32_t current,
                                    int32_t elapsed_time, SocBatterySettings *batterySettings) {
  int32_t charge_change =
      soc_multiply_fraction(current * elapsed_time, batterySettings->current_efficiency);
  int32_t new_charge = old_charge + charge_change;
  int32_t adjusted_voltage = soc_current_adjusted_voltage(voltage, current, batterySettings);

  int32_t minimum_charge = soc_minimum_charge_for_voltage(adjusted_voltage, batterySettings);
  if (new_charge <= minimum_charge) {
    return minimum_charge;
  }

  int32_t maximum_charge = soc_maximum_charge_for_voltage(adjusted_voltage, batterySettings);
  if (new_charge >= maximum_charge) {
    return maximum_charge;
  }

  return new_charge;
}

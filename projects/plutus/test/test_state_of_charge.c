#include <stdint.h>
#include "log.h"
#include "state_of_charge.h"
#include "test_helpers.h"
#include "unity.h"

SocBatterySettings batterySettings1 = {
  .minimum_voltage = 1000,  //
  .voltage_step = 1000,     //
  .voltage_to_charge = { 100,   150,   250,   400,   600,   900,   1300,  1800,  5100,  12000,
                         14000, 15000, 15800, 16400, 16900, 17300, 17500, 17650, 17750, 17800 },  //
  .voltage_inaccuracy = 1000,                                                                     //
  .current_efficiency = { 63, 64 },
  .internal_resistance = { 1, 4 }
};

SocBatterySettings batterySettings2 = {
  .minimum_voltage = 10000,  //
  .voltage_step = 2500,      //
  .voltage_to_charge = { 1200,   1700,   2700,   4000,   6000,   9000,   13000,
                         18000,  51000,  120000, 140000, 150000, 158000, 164000,
                         169000, 173000, 175000, 176500, 177500, 178000 },  //
  .voltage_inaccuracy = 500,                                                //
  .current_efficiency = { 3, 4 },
  .internal_resistance = { 1, 64 }
};

void setup_test(void) {}

void teardown_test(void) {}

void test_minimum_charge(void) {
  TEST_ASSERT_EQUAL(100, soc_minimum_charge(&batterySettings1));
  TEST_ASSERT_EQUAL(1200, soc_minimum_charge(&batterySettings2));
}

void test_maximum_charge(void) {
  TEST_ASSERT_EQUAL(17800, soc_maximum_charge(&batterySettings1));
  TEST_ASSERT_EQUAL(178000, soc_maximum_charge(&batterySettings2));
}

void test_minimum_voltage(void) {
  TEST_ASSERT_EQUAL(1000, soc_minimum_voltage(&batterySettings1));
  TEST_ASSERT_EQUAL(10000, soc_minimum_voltage(&batterySettings2));
}

void test_maximum_voltage(void) {
  TEST_ASSERT_EQUAL(20000, soc_maximum_voltage(&batterySettings1));
  TEST_ASSERT_EQUAL(57500, soc_maximum_voltage(&batterySettings2));
}

void test_multiply_fraction(void) {
  // Without rounding
  TEST_ASSERT_EQUAL(5, soc_multiply_fraction(10, (SocFraction){ 1, 2 }));

  // Signs
  TEST_ASSERT_EQUAL(-5, soc_multiply_fraction(-10, (SocFraction){ -1, -2 }));
  TEST_ASSERT_EQUAL(5, soc_multiply_fraction(-10, (SocFraction){ -1, 2 }));
  TEST_ASSERT_EQUAL(5, soc_multiply_fraction(-10, (SocFraction){ 1, -2 }));
  TEST_ASSERT_EQUAL(5, soc_multiply_fraction(10, (SocFraction){ -1, -2 }));
  TEST_ASSERT_EQUAL(-5, soc_multiply_fraction(10, (SocFraction){ 1, -2 }));
  TEST_ASSERT_EQUAL(-5, soc_multiply_fraction(10, (SocFraction){ -1, 2 }));
  TEST_ASSERT_EQUAL(-5, soc_multiply_fraction(-10, (SocFraction){ 1, 2 }));

  // Large numbers
  TEST_ASSERT_EQUAL(1073741823, soc_multiply_fraction(2147483646, (SocFraction){ 8, 16 }));

  // Rounding and sign
  TEST_ASSERT_EQUAL(1, soc_multiply_fraction(4, (SocFraction){ 1, 3 }));
  TEST_ASSERT_EQUAL(-1, soc_multiply_fraction(-4, (SocFraction){ 1, 3 }));
  TEST_ASSERT_EQUAL(2, soc_multiply_fraction(5, (SocFraction){ 1, 3 }));
  TEST_ASSERT_EQUAL(-2, soc_multiply_fraction(-5, (SocFraction){ 1, 3 }));
  TEST_ASSERT_EQUAL(2, soc_multiply_fraction(6, (SocFraction){ 1, 4 }));
  TEST_ASSERT_EQUAL(-2, soc_multiply_fraction(-6, (SocFraction){ 1, 4 }));
}

void test_charge_for_voltage(void) {
  // Voltages below the minimum
  TEST_ASSERT_EQUAL(100, soc_charge_for_voltage(0, &batterySettings1));
  TEST_ASSERT_EQUAL(1200, soc_charge_for_voltage(0, &batterySettings2));

  // Voltages above the maximum
  TEST_ASSERT_EQUAL(17800, soc_charge_for_voltage(21000, &batterySettings1));
  TEST_ASSERT_EQUAL(178000, soc_charge_for_voltage(60000, &batterySettings2));

  // Voltages at the minimum
  TEST_ASSERT_EQUAL(100, soc_charge_for_voltage(1000, &batterySettings1));
  TEST_ASSERT_EQUAL(1200, soc_charge_for_voltage(10000, &batterySettings2));

  // Voltages at the maximum
  TEST_ASSERT_EQUAL(17800, soc_charge_for_voltage(20000, &batterySettings1));
  TEST_ASSERT_EQUAL(178000, soc_charge_for_voltage(57500, &batterySettings2));

  // Voltages at a specific step
  TEST_ASSERT_EQUAL(12000, soc_charge_for_voltage(10000, &batterySettings1));
  TEST_ASSERT_EQUAL(6000, soc_charge_for_voltage(20000, &batterySettings2));

  // Voltages in the middle of the first two steps
  TEST_ASSERT_EQUAL(125, soc_charge_for_voltage(1500, &batterySettings1));
  TEST_ASSERT_EQUAL(1450, soc_charge_for_voltage(11250, &batterySettings2));

  // Voltages in the middle of the last two steps
  TEST_ASSERT_EQUAL(17775, soc_charge_for_voltage(19500, &batterySettings1));
  TEST_ASSERT_EQUAL(177750, soc_charge_for_voltage(56250, &batterySettings2));

  // Voltages with some offset
  TEST_ASSERT_EQUAL(112, soc_charge_for_voltage(1234, &batterySettings1));
  TEST_ASSERT_EQUAL(1669, soc_charge_for_voltage(12345, &batterySettings2));
}

void test_minimum_charge_for_voltage(void) {
  TEST_ASSERT_EQUAL(17700, soc_minimum_charge_for_voltage(19500, &batterySettings1));
  TEST_ASSERT_EQUAL(177750, soc_minimum_charge_for_voltage(56750, &batterySettings2));
  TEST_ASSERT_EQUAL(112, soc_minimum_charge_for_voltage(2234, &batterySettings1));
  TEST_ASSERT_EQUAL(1669, soc_minimum_charge_for_voltage(12845, &batterySettings2));
}

void test_maximum_charge_for_voltage(void) {
  TEST_ASSERT_EQUAL(200, soc_maximum_charge_for_voltage(1500, &batterySettings1));
  TEST_ASSERT_EQUAL(1450, soc_maximum_charge_for_voltage(10750, &batterySettings2));
  TEST_ASSERT_EQUAL(112, soc_maximum_charge_for_voltage(234, &batterySettings1));
  TEST_ASSERT_EQUAL(1669, soc_maximum_charge_for_voltage(11845, &batterySettings2));
}

void test_current_adjusted_voltage(void) {
  TEST_ASSERT_EQUAL(10, soc_current_adjusted_voltage(13, 12, &batterySettings1));
  TEST_ASSERT_EQUAL(121, soc_current_adjusted_voltage(123, 128, &batterySettings2));
}

void test_charge_after_transition(void) {
  // Regular change
  TEST_ASSERT_EQUAL(150, soc_charge_after_transition(126, 2003, 12, 2, &batterySettings1));
  TEST_ASSERT_EQUAL(1700, soc_charge_after_transition(1316, 12502, 128, 4, &batterySettings2));
  // Reverse change
  TEST_ASSERT_EQUAL(150, soc_charge_after_transition(174, 1997, -12, 2, &batterySettings1));
  TEST_ASSERT_EQUAL(1700, soc_charge_after_transition(2084, 12498, -128, 4, &batterySettings2));
  // Calibrating down
  TEST_ASSERT_EQUAL(150, soc_charge_after_transition(10000, 1003, 12, 2, &batterySettings1));
  TEST_ASSERT_EQUAL(1700, soc_charge_after_transition(100000, 12002, 128, 4, &batterySettings2));
  // Calibrating up
  TEST_ASSERT_EQUAL(150, soc_charge_after_transition(-10000, 3003, 12, 2, &batterySettings1));
  TEST_ASSERT_EQUAL(1700, soc_charge_after_transition(-100000, 13002, 128, 4, &batterySettings2));
}

#include "thermistor.h"
#include <limits.h>
#include <stdint.h>
#include "log.h"

// src: https://www.murata.com/en-global/products/productdata/8796836626462/NTHCG83.txt
// Expected resistance in milliohms for a given temperature in celsius
static const uint32_t s_resistance_lookup[] = {
  27218600, 26076000, 24987700, 23950900, 22962900, 22021100, 21123000, 20266600, 19449500,
  18669800, 17925500, 17213900, 16534400, 15885600, 15265800, 14673500, 14107500, 13566400,
  13048900, 12554000, 12080500, 11628100, 11194700, 10779500, 10381500, 10000000, 9634200,
  9283500,  8947000,  8624200,  8314500,  8018100,  7733700,  7460900,  7199100,  6947900,
  6706700,  6475100,  6252600,  6039000,  5833600,  5635700,  5445400,  5262300,  5086300,
  4916900,  4753900,  4597100,  4446100,  4300800,  4160900,  4026200,  3896400,  3771400,
  3651000,  3535000,  3423100,  3315200,  3211300,  3111000,  3014300,  2922400,  2833700,
  2748200,  2665700,  2586100,  2509300,  2435100,  2363500,  2294300,  2227500,  2162700,
  2100100,  2039600,  1981100,  1924500,  1869800,  1817000,  1765800,  1716400,  1668500,
  1622400,  1577700,  1534500,  1492700,  1452100,  1412900,  1374900,  1338100,  1302500,
  1268000,  1234300,  1201600,  1170000,  1139300,  1109600,  1080700,  1052800,  1025600,
  999300,   973800,
};

StatusCode thermistor_init(ThermistorStorage *storage, GPIOAddress thermistor_gpio,
                           ThermistorPosition position) {
  storage->position = position;
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&thermistor_gpio, &gpio_settings);

  adc_get_channel(thermistor_gpio, &(storage->adc_channel));

  // set the channel
  adc_set_channel(storage->adc_channel, true);
  return STATUS_CODE_OK;
}

StatusCode thermistor_get_temp(ThermistorStorage *storage, uint32_t *temperature_millicelcius) {
  // fetch the voltage readings
  uint16_t reading = 0;                          // the divided voltage in millivolts
  uint32_t thermistor_resistance_milliohms = 0;  // resistance in milliohms
  uint16_t vdda = 0;                             // vdda voltage in millivolts

  // get source voltage and node voltage of the voltage divider
  status_ok_or_return(adc_read_converted(ADC_CHANNEL_REF, &vdda));
  status_ok_or_return(adc_read_converted(storage->adc_channel, &reading));

  if (vdda == 0) {
    *temperature_millicelcius = UINT_MAX;
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "No source voltage detected.");
  } else if (reading == 0) {
    *temperature_millicelcius = UINT_MAX;
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "No node voltage detected.");
  }

  // Calculates thermistor resistance in milliOhms based on the position of thermistor and the fixed
  // resistor
  if (storage->position == THERMISTOR_POSITION_R1) {
    thermistor_resistance_milliohms =
        ((uint32_t)(vdda - reading)) * (uint32_t)THERMISTOR_FIXED_RESISTANCE_OHMS / reading * 1000;
  } else {
    thermistor_resistance_milliohms =
        ((uint32_t)THERMISTOR_FIXED_RESISTANCE_OHMS * reading) / (uint32_t)(vdda - reading) * 1000;
  }
  return thermistor_calculate_temp(thermistor_resistance_milliohms, temperature_millicelcius);
}

StatusCode thermistor_calculate_temp(uint32_t thermistor_resistance_milliohms,
                                     uint32_t *temperature_millicelcius) {
  // find the approximate target temperature from the arguments passed
  for (uint16_t i = 0; i < SIZEOF_ARRAY(s_resistance_lookup) - 1; i++) {
    if (thermistor_resistance_milliohms <= s_resistance_lookup[i] &&
        thermistor_resistance_milliohms >= s_resistance_lookup[i + 1]) {
      // return the temperature with the linear approximation in millicelsius
      *temperature_millicelcius =
          ((uint32_t)i * 1000 + ((s_resistance_lookup[i] - thermistor_resistance_milliohms) * 1000 /
                                 (s_resistance_lookup[i] - s_resistance_lookup[i + 1])));
      return STATUS_CODE_OK;
    }
  }
  // Sets the returned temperature to be absurdly large
  *temperature_millicelcius = UINT_MAX;
  return status_msg(STATUS_CODE_OUT_OF_RANGE, "Temperature out of lookup table range.");
}

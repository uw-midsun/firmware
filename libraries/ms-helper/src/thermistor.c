#include "thermistor.h"
#include <stdint.h>
#include "log.h"

// expected resistance in milliohms for a given temperature in celsius
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

StatusCode thermistor_init(ThermistorStorage *storage, ThermistorSettings *settings) {
  storage->sibling_resistance = settings->sibling_resistance;
  storage->adc_channel = settings->adc_channel;

  // set the channel
  adc_set_channel(settings->adc_channel, true);

  return STATUS_CODE_OK;
}

StatusCode thermistor_get_temp(ThermistorStorage *storage, uint32_t *temperature) {
  // fetch the voltage readings
  uint16_t reading = 0;
  uint32_t thermistor_resistance = 0;
  uint16_t vdda = 0;

  // get source voltage and voltage drop
  adc_read_converted(ADC_CHANNEL_REF, &vdda);
  adc_read_converted(storage->adc_channel, &reading);

  thermistor_resistance =
      (storage->sibling_resistance / reading) * (vdda)-storage->sibling_resistance;

  // find the approximate target temperature
  for (uint16_t i = 0; i < SIZEOF_ARRAY(s_resistance_lookup) - 1; i++) {
    if (thermistor_resistance <= s_resistance_lookup[i] &&
        thermistor_resistance >= s_resistance_lookup[i + 1]) {
      // return the temperature with the linear approximation
      *temperature =
          ((uint32_t)i * 1000 + ((s_resistance_lookup[i] - thermistor_resistance) * 1000 /
                                 (s_resistance_lookup[i] - s_resistance_lookup[i + 1])));
      return STATUS_CODE_OK;
    }
  }

  return STATUS_CODE_OUT_OF_RANGE;
}

StatusCode thermistor_calculate_temp(uint16_t resistance, uint32_t *temperature) {
  // find the approximate target temperature from the arguments passed
  for (uint16_t i = 0; i < SIZEOF_ARRAY(s_resistance_lookup) - 1; i++) {
    if (resistance <= s_resistance_lookup[i] && resistance >= s_resistance_lookup[i + 1]) {
      // return the temperature with the linear approximation
      *temperature = ((uint32_t)i * 1000 + ((s_resistance_lookup[i] - resistance) * 1000 /
                                            (s_resistance_lookup[i] - s_resistance_lookup[i + 1])));
      return STATUS_CODE_OK;
    }
  }

  return STATUS_CODE_OUT_OF_RANGE;
}

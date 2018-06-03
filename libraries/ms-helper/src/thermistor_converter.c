#include "thermistor_converter.h"
#include <stdint.h>
#include "log.h"

// resistance to milliohms
uint32_t resistance[RESISTANCE_LEN] = {
  27218600, 26076000, 24987700, 23950900, 22962900, 22021100, 21123000, 20266600, 19449500,
  18669800, 17925500, 17213900, 16534400, 15885600, 15265800, 14673500, 14107500, 13566400,
  13048900, 12554000, 12080500, 11628100, 11194700, 10779500, 10381500, 10000000, 9634200,
  9283500,  8947000,  8624200,  8314500,  8018100,  7733700,  7460900,  7199100,  6947900,
  6706700,  6475100,  6252600,  6039000,  5833600,  5635700,  5445400,  5262300,  5086300,
  4916900,  4753900,  4597100,  4446100,  4300800,  4160900,  4026200,  3896400,  3771400,
  3651000,  3535000,  3423100,  3315200,  3211300,  3111000,  3014300,  2922400,  2833700,
  2748200,  2665700,  2586100,  2509300,  2435100,  2363500,  2294300,  2227500,
};

StatusCode thermistor_converter_init(ThermistorStorage *storage, ThermistorSettings *settings) {
  storage->sibling_resistance = settings->sibling_resistance;
  storage->source_voltage = settings->source_voltage;
  storage->adc_channel = settings->adc_channel;
  storage->context = settings->context;

  // initialize gpio pin
  gpio_init_pin(settings->gpio_addr, settings->gpio_settings);

  // initialize the channel
  adc_init(settings->adc_mode);
  adc_set_channel(settings->adc_channel, true);

  return STATUS_CODE_OK;
}

uint32_t thermistor_converter_get_temp(ThermistorStorage *storage) {
  // fetch the voltage readings
  uint16_t reading = 0;
  uint32_t thermistor_resistance = 0;
  adc_read_converted(storage->adc_channel, &reading);

  thermistor_resistance = (storage->sibling_resistance / reading) * (storage->source_voltage) -
                          storage->sibling_resistance;

  // find the approximate target temperature
  for (uint16_t i = 0; i < RESISTANCE_LEN; i++) {
    if (thermistor_resistance <= resistance[i] && thermistor_resistance >= resistance[i + 1]) {
      // return the temperatue with the linear approximation
      return ((uint32_t)i * 1000 + ((resistance[i] - thermistor_resistance) * 1000 /
                                    (resistance[i] - resistance[i + 1])));
    }
  }

  return 0;
}

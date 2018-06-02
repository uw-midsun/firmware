#include <stdint.h>
#include "thermistor_converter.h"
#include "log.h"

// In milliohms
uint32_t resistance[71] = {
  27218600, 26076000, 24987700, 23950900, 22962900, 22021100, 21123000, 20266600, 19449500, 18669800, 17925500, 17213900,
  16534400, 15885600, 15265800, 14673500, 14107500, 13566400, 13048900, 12554000, 12080500, 11628100, 11194700, 10779500,
  10381500, 10000000, 9634200,  9283500,  8947000,  8624200,  8314500,  8018100,  7733700,  7460900,  7199100,  6947900,
  6706700,  6475100,  6252600,  6039000,  5833600,  5635700,  5445400,  5262300,  5086300,  4916900,  4753900,  4597100,
  4446100,  4300800,  4160900,  4026200,  3896400,  3771400,  3651000,  3535000,  3423100,  3315200,  3211300,  3111000,
  3014300,  2922400,  2833700,  2748200,  2665700,  2586100,  2509300,  2435100,  2363500,  2294300,  2227500,
};

// uint16_t find_temperature(uint16_t sibling_resistance, uint16_t sibling_voltage,
//                           uint16_t source_voltage) {
//   // The target resistance based on previous resistors and source voltage
//   int thermistor_resistance = (source_voltage - sibling_voltage) * sibling_resistance /
//   sibling_voltage;

//   // Finds the target temperature
//   for (uint16_t i = 0; i < 70 - 1; i++) {
//     if (thermistor_resistance <= resistance[i] && thermistor_resistance >= resistance[i + 1]) {
//       // return i * (thermistor_resistance - resistance[0]) / (resistance[i + 1] -
//       resistance[i]); return i;
//     }
//   }
// }

// assuming resistor is r2 for the voltage resistor equation
static uint32_t prv_voltage_to_resistance(uint32_t vin, uint32_t r2, uint32_t vout) {
  return (10000000/vout)*3000 - 10000000;
}

StatusCode thermistor_converter_init(void) {
  // gpio info
  GPIOAddress DCDCTemperature1 = {
    .port = GPIO_PORT_A,
    .pin = 4,
  };

  GPIOSettings settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  // initialize gpio
  gpio_init();
  gpio_init_pin(&DCDCTemperature1, &settings);

  // initialize interrupts
  interrupt_init();

  soft_timer_init();

  // initialize the channel
  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_4, true);

  return STATUS_CODE_OK;
}

uint32_t thermistor_converter_get_temp(void) {
  // The target resistance based on previous resistors and source voltage
  //uint16_t thermistor_resistance = (3 - 1.5) * 10000 / 1.5;

  // get raw reading
  uint16_t reading = 0;
  uint32_t thermistor_resistance = 0;
  //adc_read_raw(ADC_CHANNEL_4, &reading);
  adc_read_converted(ADC_CHANNEL_4, &reading);
  //return reading;
  thermistor_resistance = prv_voltage_to_resistance(3000, 10000000, (uint32_t)reading);
  // return thermistor_resistance;
  // Finds the target temperature
  for (uint32_t i = 0; i < 70; i++) {
    if (thermistor_resistance <= resistance[i] && thermistor_resistance >= resistance[i + 1]) {
      // return i * (thermistor_resistance - resistance[0]) / (resistance[i + 1] - resistance[i]);
      return i;
    }
  }

  return 0;
}

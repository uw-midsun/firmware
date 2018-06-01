#include <math.h>
#include <stdint.h>
#include "adc.h"
#include "gpio.h"
#include "log.h"

// In milliohms
uint16_t resistance[70] = {
  272186, 260760, 249877, 239509, 229629, 220211, 211230, 202666, 194495, 186698, 179255, 172139,
  165344, 158856, 152658, 146735, 141075, 135664, 130489, 125540, 120805, 116281, 111947, 107795,
  103815, 100000, 96342,  92835,  89470,  86242,  83145,  80181,  77337,  74609,  71991,  69479,
  67067,  64751,  62526,  60390,  58336,  56357,  54454,  52623,  50863,  49169,  47539,  45971,
  44461,  43008,  41609,  40262,  38964,  37714,  36510,  35350,  34231,  33152,  32113,  31110,
  30143,  29224,  28337,  27482,  26657,  25861,  25093,  24351,  23635,  22943,  22275,
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

int main(void) {
  adc_init(1);
}

uint16_t find_temperature() {
  // The target resistance based on previous resistors and source voltage
  int thermistor_resistance = (3 - 1.5) * 10000 / 1.5;

  GPIOAddress DCDCTemperature1 =
      {
        .port = GPIO_PORT_A,
        .pin = 4,
      }

  GPIOSettings settings =
  {.direction = GPIO_DIR_IN,
   .state = GPIO_STATE_LOW,
   .resistance = GPIO_RES_NONE,
   .alt_function = GPIO_ALTFN_ANALOG,
  }

  gpio_init();
  gpio_init_pin(&DCDCTemperature1, &settings);
  adc_init();
  adc_get_channel(DCDCTemperature1, )

  // Finds the target temperature
  for (uint16_t i = 0; i < 70 - 1; i++) {
    if (thermistor_resistance <= resistance[i] && thermistor_resistance >= resistance[i + 1]) {
      // return i * (thermistor_resistance - resistance[0]) / (resistance[i + 1] - resistance[i]);
      return i;
    }
  }
}
#include "thermistor.h"
#include <limits.h>
#include <stdint.h>
#include "log.h"

// Used for converting the lookup table index with corresponding temperatures
#define THERMISTOR_LOOKUP_RANGE 100

#define THERMISTOR_MILLIOHMS_TO_OHMS(x) ((x) / 1000)
#define THERMISTOR_DECICELSIUS_TO_CELSIUS(x) ((x) / 10)
#define THERMISTOR_CELSIUS_TO_DECICELSIUS(x) ((x)*10)

// Expected resistance in milliohms for a given temperature in celsius
//
// This table covers the range [0 C, 100 C] in 1 degree steps
static const uint32_t s_resistance_lookup[][THERMISTOR_LOOKUP_RANGE + 1] = {
  // NXRT15XH103
  // src: https://www.murata.com/en-global/products/productdata/8796836626462/NTHCG83.txt
  {
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
  },
  // NXRT15WF104
  // src: https://www.murata.com/~/media/webrenewal/support/library/catalog/products/thermistor/ntc/r44e.ashx
  // values interpolated with cubic spline using scipy.interpolate
  {
  357012000, 338065466, 320192061, 303340723, 287460390, 272500000, 258408490, 245134797, 232627859,
  220836614, 209710000, 199200088, 189271488, 179891944, 171029200, 162651000, 154726534, 147230778,
  140140155, 133431088, 127080000, 121064361, 115365832, 109967123, 104850943, 100000000, 95397831,
  91031277,  86888008,  82955693,  79222000,  75675125,  72305364,  69103541,  66060479,  63167000,
  60414367,  57795595,  55304140,  52933457,  50677000,  48528424,  46482183,  44532929,  42675317,
  40904000,  39213953,  37601442,  36063054,  34595377,  33195000,  31858572,  30582994,  29365231,
  28202245,  27091000,  26028615,  25012829,  24041535,  23112628,  22224000,  21373639,  20559906,
  19781253,  19036133,  18323000,  17640315,  16986572,  16360271,  15759913,  15184000,  14631164,
  14100559,  13591473,  13103191,  12635000,  12186150,  11755751,  11342876,  10946601,  10566000,
  10200205,  9848582,   9510557,   9185555,   8873000,   8572351,   8283201,   8005175,   7737900,
  7481000,   7234086,   6996701,   6768373,   6548631,   6337000,   6133009,   5936187,   5746059,
  5562154,   5384000,
  }
};


StatusCode thermistor_init(ThermistorStorage *storage, ThermistorSettings *settings) {
  storage->settings = settings;
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&settings->thermistor_gpio, &gpio_settings);

  adc_get_channel(settings->thermistor_gpio, &(storage->adc_channel));
  adc_set_channel(storage->adc_channel, true);
  return STATUS_CODE_OK;
}

StatusCode thermistor_get_resistance(ThermistorPosition position, uint16_t dividor_resistor_ohms, uint16_t reading_mv, uint16_t vdda_mv, uint32_t *thermistor_resistance_mohms) {
  // Calculates thermistor resistance in milliOhms based on the position of thermistor and the fixed
  // resistor
  if (position == THERMISTOR_POSITION_R1) {
    *thermistor_resistance_mohms =
        (((uint32_t)(vdda_mv - reading_mv)) * (uint32_t)dividor_resistor_ohms / (uint32_t)reading_mv) * 1000;
  } else {
    *thermistor_resistance_mohms =
        (((uint32_t)dividor_resistor_ohms * (uint32_t)reading_mv) / (uint32_t)(vdda_mv - reading_mv)) * 1000;
  }
  return STATUS_CODE_OK;
}

StatusCode thermistor_calculate_temp(ThermistorModel model, uint32_t thermistor_resistance_mohms, uint16_t *temperature_dc) {
  // Find the approximate target temperature from the arguments passed
  uint32_t *lookup_table = s_resistance_lookup[model];

  for (uint16_t i = 0; i < THERMISTOR_LOOKUP_RANGE + 1; i++) {
    if (thermistor_resistance_mohms <= lookup_table[i] &&
        thermistor_resistance_mohms >= lookup_table[i + 1]) {
      // Return the temperature with the linear approximation in deciCelsius
      *temperature_dc = (uint16_t)(
          ((uint32_t) i * 100 + ((lookup_table[i] - thermistor_resistance_mohms) /
                                 (lookup_table[i] - lookup_table[i + 1]))) / 100);
      return STATUS_CODE_OK;
    }
  }
  // Sets the returned temperature to be absurdly large
  *temperature_dc = UINT16_MAX;
  return status_msg(STATUS_CODE_OUT_OF_RANGE, "Temperature out of lookup table range.");
}

StatusCode thermistor_calculate_resistance_from_temp(ThermistorModel model, uint16_t temperature_dc,
                                           uint16_t *thermistor_resistance_ohms) {
  uint32_t *lookup_table = s_resistance_lookup[model];
                                     
  if (temperature_dc > THERMISTOR_CELSIUS_TO_DECICELSIUS(THERMISTOR_LOOKUP_RANGE)) {
    return status_msg(STATUS_CODE_OUT_OF_RANGE,
                      "Input temperature, exceeds lookup table ranges (0-100 deg).");
  } else if (temperature_dc == THERMISTOR_CELSIUS_TO_DECICELSIUS(THERMISTOR_LOOKUP_RANGE)) {
    // For the higher lookup edge case
    *thermistor_resistance_ohms =
        THERMISTOR_MILLIOHMS_TO_OHMS(lookup_table[THERMISTOR_LOOKUP_RANGE]);
  } else {
    // Linearly interpolate between the two points and then convert to Ohms
    uint16_t lower_temp = THERMISTOR_DECICELSIUS_TO_CELSIUS(temperature_dc);
    *thermistor_resistance_ohms = THERMISTOR_MILLIOHMS_TO_OHMS(
        (lookup_table[lower_temp] * 10 +
         (lookup_table[lower_temp + 1] - lookup_table[lower_temp]) *
             (temperature_dc % 10)) / 10);
  }

  return STATUS_CODE_OK;
}

StatusCode thermistor_read_and_calculate_temp(ThermistorStorage *storage, uint16_t *temperature_dc) {
  ThermistorSettings *settings = storage->settings;
  // Fetch the voltage readings
  uint16_t reading_mv = 0;                     // the divided voltage in millivolts
  uint32_t thermistor_resistance_mohms = 0;    // resistance in milliohms
  uint16_t vdda_mv = 0;                        // vdda voltage in millivolts

  // Get source voltage and node voltage of the voltage divider
  status_ok_or_return(adc_read_converted(ADC_CHANNEL_REF, &vdda_mv));
  status_ok_or_return(adc_read_converted(storage->adc_channel, &reading_mv));

  if (vdda_mv == 0) {
    *temperature_dc = UINT16_MAX;
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "No source voltage detected.");
  } else if (reading_mv == 0) {
    *temperature_dc = UINT16_MAX;
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "No node voltage detected.");
  }

  status_ok_or_return(thermistor_get_resistance(settings->position, settings->dividor_resistor_ohms, reading_mv, vdda_mv, &thermistor_resistance_mohms));
  return thermistor_calculate_temp(settings->model, thermistor_resistance_mohms, temperature_dc);
}
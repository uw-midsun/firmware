#include "driver_display_config.h"

// Overall screen info
static const DriverDisplayConfigGpio screen_info[] = {
  [DRIVER_DISPLAY_SCREEN1] = { .address = { DRIVER_DISPLAY_CONFIG_SCREEN1_PORT,
                                            DRIVER_DISPLAY_CONFIG_SCREEN1_PIN },
                               .timer = DRIVER_DISPLAY_CONFIG_SCREEN1_TIMER,
                               .frequency = DRIVER_DISPLAY_CONFIG_SCREEN1_FREQ },
  [DRIVER_DISPLAY_SCREEN2] = { .address = { DRIVER_DISPLAY_CONFIG_SCREEN2_PORT,
                                            DRIVER_DISPLAY_CONFIG_SCREEN1_PIN },
                               .timer = DRIVER_DISPLAY_CONFIG_SCREEN2_TIMER,
                               .frequency = DRIVER_DISPLAY_CONFIG_SCREEN2_FREQ },
};

static const GPIOAddress adc_address = { .port = DRIVER_DISPLAY_CONFIG_ADC_PORT,
                                         .pin = DRIVER_DISPLAY_CONFIG_ADC_PIN };

void driver_display_config(void) {
  // Init the PWM pin(s)
  GPIOSettings screen_pwm_settings = { .direction = GPIO_DIR_OUT,
                                       .state = GPIO_STATE_HIGH,
                                       .resistor = GPIO_RES_PULLUP,
                                       .alt_function = GPIO_ALTFN_4 };

  // Init the pwm for each screen
  for (uint8_t i = 0; i < SIZEOF_ARRAY(screen_info); i++) {
    gpio_init_pin(&screen_info[i].address, &screen_pwm_settings);
    pwm_init(screen_info[i].timer,
             screen_info[i].frequency * 1000);  // frequency converted to microseconds
    pwm_set_dc(screen_info[i].timer, 50);       // set the screen brightness to 50% initially
  }

  // Init the ADC pin
  GPIOSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  ADCChannel adc_channel;
  gpio_init_pin(&adc_address, &adc_settings);
  adc_get_channel(adc_address, &adc_channel);
  adc_set_channel(adc_channel, true);
}

GPIOAddress driver_display_config_get_adc(void) {
  return adc_address;
}

DriverDisplayConfigGpio *driver_display_config_get_screen_info(void) {
  return screen_info;
}

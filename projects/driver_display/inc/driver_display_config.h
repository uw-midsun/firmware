#pragma once
// Configures the internal screen(s)

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "pwm_mcu.h"

// Temporary defined constant for max value of adc
// These are just temporary until I figure our how to calibrate the photosensor
#define DRIVER_DISPLAY_CONFIG_ADC_MAX 4095
#define DRIVER_DISPLAY_CONFIG_ADC_MIN 0
#define DRIVER_DISPLAY_CONFIG_ADC_RANGE \
  DRIVER_DISPLAY_CONFIG_ADC_MAX - DRIVER_DISPLAY_CONFIG_ADC_MIN
// End of temporary constants

// Defined constants for the screen(s)
#define DRIVER_DISPLAY_CONFIG_NUM_SCREENS 2
// Frequency will currently cause some errors as the pwm function is set to use milliseconds not
// microseconds
#define DRIVER_DISPLAY_CONFIG_SCREEN1_FREQ 30  // Frequency in kHz (subject to change)
#define DRIVER_DISPLAY_CONFIG_SCREEN2_FREQ 30  // Frequency in kHz (subject to change)

#define DRIVER_DISPLAY_CONFIG_SCREEN1_TIMER PWM_TIMER_14
#define DRIVER_DISPLAY_CONFIG_SCREEN2_TIMER PWM_TIMER_14

#define DRIVER_DISPLAY_CONFIG_SCREEN1_PORT GPIO_PORT_A
#define DRIVER_DISPLAY_CONFIG_SCREEN2_PORT GPIO_PORT_A

#define DRIVER_DISPLAY_CONFIG_SCREEN1_PIN 7
#define DRIVER_DISPLAY_CONFIG_SCREEN2_PIN 4

// Defined constants for the ADC
#define DRIVER_DISPLAY_CONFIG_ADC_PORT GPIO_PORT_A
#define DRIVER_DISPLAY_CONFIG_ADC_PIN 0
#define DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD \
  5  // The length of time in seconds before the adc is sampled again

typedef enum {
  DRIVER_DISPLAY_SCREEN1 = 0,
  DRIVER_DISPLAY_SCREEN2,
} DriverDisplayConfigScreen;

typedef struct {
  GPIOAddress address;
  PWMTimer timer;
  uint16_t frequency;  // frequency in kHz
} DriverDisplayConfigGpio;

typedef struct {
  GPIOAddress adc_address;
  DriverDisplayConfigGpio *screen_info;
  uint16_t num_screens;
} DriverDisplayConfigInfo;

// Configures all the gpio pins, must be called upon startup
void driver_display_config(void);

// Returns the address of the ADC if any other function needs to interact with the ADC channel
GPIOAddress driver_display_config_get_adc(void);

// Returns information about a given screen
DriverDisplayConfigGpio *driver_display_config_get_screen_info(void);

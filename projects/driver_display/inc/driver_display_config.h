#pragma once
// Module to configure the display pins and timers used by the display screens.
// Initiaizes the persist page and GPIO pins
// Defines pin functions and other screen parameters such as frequency

#include "adc.h"
#include "gpio.h"
#include "pwm.h"
#include "pwm_mcu.h"
#include "soft_timer.h"

#define DRIVER_DISPLAY_CONFIG_CALIBRATION_TIME 5
// Defined constants for the screen(s)
#define DRIVER_DISPLAY_CONFIG_NUM_SCREENS 2

#define DRIVER_DISPLAY_CONFIG_SCREEN1_FREQ 30  // Frequency in kHz (subject to change)
#define DRIVER_DISPLAY_CONFIG_SCREEN2_FREQ 30  // Frequency in kHz (subject to change)

#define DRIVER_DISPLAY_CONFIG_SCREEN1_TIMER PWM_TIMER_14
#define DRIVER_DISPLAY_CONFIG_SCREEN2_TIMER PWM_TIMER_14

#define DRIVER_DISPLAY_CONFIG_SCREEN1_PORT GPIO_PORT_A
#define DRIVER_DISPLAY_CONFIG_SCREEN2_PORT GPIO_PORT_A

#define DRIVER_DISPLAY_CONFIG_SCREEN1_PIN 7
#define DRIVER_DISPLAY_CONFIG_SCREEN2_PIN 4

#define DRIVER_DISPLAY_CONFIG_PERSIST_PAGE (NUM_FLASH_PAGES - 1)

// Defined constants for the ADC
#define DRIVER_DISPLAY_CONFIG_ADC_PORT GPIO_PORT_A
#define DRIVER_DISPLAY_CONFIG_ADC_PIN 0
#define DRIVER_DISPLAY_CONFIG_REFRESH_PERIOD \
  5  // The length of time in seconds before the adc is sampled again
#define DRIVER_DISPLAY_CONFIG_REFRESH_TIMER 0

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

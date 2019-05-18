#pragma once
// Configuration for defining and loading custom settings for the tutorial module
#include "button.h"
#include "potentiometer.h"

// ADC periodic polling rate
#define CONFIG_ADC_UPDATE_PERIOD_S 2

// Loads the button configurations for the button module
ButtonSettings *config_load_buttons(void);

// Loads the potentiometer configurations for the potentiometer module
PotentiometerSettings *config_load_potentiometer(void);

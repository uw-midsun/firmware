#pragma once

#include "gpio.h"

const GPIOAddress board_type_address;

// front board peripheral definitions
GPIOAddress ADDRESS_SIGNAL_LEFT_FRONT;
GPIOAddress ADDRESS_SIGNAL_RIGHT_FRONT;
GPIOAddress ADDRESS_HORN;
GPIOAddress ADDRESS_HEADLIGHTS;

// rear board peripheral definitions
GPIOAddress ADDRESS_SIGNAL_RIGHT_REAR;
GPIOAddress ADDRESS_SIGNAL_LEFT_REAR;
GPIOAddress ADDRESS_BRAKE;
GPIOAddress ADDRESS_STROBE;

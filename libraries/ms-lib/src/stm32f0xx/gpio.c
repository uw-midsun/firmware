#include "inc/gpio.h"

void io_deinit(const struct IOMap*);

// Set mode for the pin
void io_set_mode(const struct IOMap* io_map, IOMode mode);

// Set the output type and speed of the pin
void io_set_output(const struct IOMap* io_map, IOOutput output, IOSpeed speed);

// Set the pull-up pull-down resistor
void io_set_resistor(const struct IOMap* io_map, IOResistor resistor);

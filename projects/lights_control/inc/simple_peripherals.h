#pragma once

typedef StatusCode (*SimplePeripheralCallback)(Event e);

// it registers the callback for all the events that
// correspond to "simple" peripherals,
// that is: horn, headlights, brakes
StatusCode lights_simple_peripherals_init(SimplePeripheralCallback);

StatusCode lights_simple_peripherals_process_event(Event e);



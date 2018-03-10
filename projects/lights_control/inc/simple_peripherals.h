#pragma once

typedef StatusCode (*SimplePeripheralCallback)(Event e);

StatusCode simple_peripherals_event(Event e);
StatusCode simple_peripherals_init(SimplePeripheralCallback);


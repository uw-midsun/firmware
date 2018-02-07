#pragma once

#define HEADLIGHTS_PORT 0 // TODO: Ask these
#define HEADLIGHTS_PIN  0
// used for: horn, headlights, brake and strobe
// because they all have simple behaviours

StatusCode process_peripheral(Event e);


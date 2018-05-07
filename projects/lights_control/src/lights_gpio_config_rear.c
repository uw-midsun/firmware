#include "misc.h"

#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio.h"
#include "lights_gpio_config.h"
#include "lights_gpio_config_rear.h"

// Peripheral definitions
static const LightsGPIOPeripheral s_rear_peripherals[] =
    { [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_STROBE] =
          {
              //
              .address = { .port = GPIO_PORT_B, .pin = 11 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,    //
          },                                                  //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_BRAKE] =
          {
              //
              .address = { .port = GPIO_PORT_B, .pin = 1 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
          },                                                 //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_OUTER_BRAKE] =
          {
              //
              .address = { .port = GPIO_PORT_B, .pin = 2 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
          },                                                 //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_BRAKE] =
          {
              //
              .address = { .port = GPIO_PORT_B, .pin = 15 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,    //
          },                                                  //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_OUTER_BRAKE] =
          {
              //
              .address = { .port = GPIO_PORT_A, .pin = 8 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
          },                                                 //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_CENTRE_BRAKE] =
          {
              //
              .address = { .port = GPIO_PORT_B, .pin = 14 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,    //
          },                                                  //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_OUTER_TURN] =
          {
              //
              .address = { .port = GPIO_PORT_A, .pin = 10 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,    //
          },                                                  //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_TURN] =
          {
              //
              .address = { .port = GPIO_PORT_A, .pin = 9 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
          },                                                 //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_OUTER_TURN] =
          {
              //
              .address = { .port = GPIO_PORT_B, .pin = 0 },  //
              .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,   //
          },                                                 //
      [LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_TURN] = {
          //
          .address = { .port = GPIO_PORT_B, .pin = 10 },  //
          .polarity = LIGHTS_GPIO_POLARITY_ACTIVE_LOW,    //
      } };

static LightsGPIOEventMapping s_rear_event_mappings[] = {
  { .event_id = LIGHTS_EVENT_STROBE_BLINK,  //
    .peripheral_mapping = LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_STROBE) },
  { .event_id = LIGHTS_EVENT_BRAKES,  //
    .peripheral_mapping =             //
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_BRAKE) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_OUTER_BRAKE) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_BRAKE) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_OUTER_BRAKE) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_CENTRE_BRAKE) },
  { .event_id = LIGHTS_EVENT_SIGNAL_LEFT_BLINK,  //
    .peripheral_mapping =                        //
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_OUTER_TURN) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_TURN) },
  { .event_id = LIGHTS_EVENT_SIGNAL_RIGHT_BLINK,  //
    .peripheral_mapping =                         //
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_OUTER_TURN) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_TURN) },
  { .event_id = LIGHTS_EVENT_SIGNAL_HAZARD_BLINK,  //
    .peripheral_mapping =                          //
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_OUTER_TURN) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_LEFT_TURN) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_OUTER_TURN) |
    LIGHTS_GPIO_PERIPHERAL_BIT(LIGHTS_GPIO_CONFIG_REAR_PERIPHERAL_RIGHT_TURN) },
};

static const LightsGPIO s_lights_gpio_rear = {
  .peripherals = s_rear_peripherals,                          //
  .num_peripherals = SIZEOF_ARRAY(s_rear_peripherals),        //
  .event_mappings = s_rear_event_mappings,                    //
  .num_event_mappings = SIZEOF_ARRAY(s_rear_event_mappings),  //
};

LightsGPIO *lights_config_rear_load(void) {
  return &s_lights_gpio_rear;
}

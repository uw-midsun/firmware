#include "center_console.h"

#include "can_transmit.h"
#include "gpio_it.h"

// GPIO Callback that
static void prv_gpio_toggle_callback(const GpioAddress *address, void *context) {
  // Simple toggles just toggle state with a shared event,
  // CENTER_CONSOLE_EVENT_BUTTON_TOGGLE_STATE, with the data field as the
  // actual IO we are toggling
  CenterConsoleInput *toggle_button = (CenterConsoleInput *)context;

  // Raise event via CAN message
  CAN_TRANSMIT_CENTER_CONSOLE_EVENT(toggle_button->can_event, 0);
}

static void prv_gpio_radio_callback(const GpioAddress *address, void *context) {
  // Radio buttons switch state by using the event id and passing 0 data
  CenterConsoleInput *toggle_button = (CenterConsoleInput *)context;

  // Raise event via CAN message
  CAN_TRANSMIT_CENTER_CONSOLE_EVENT(toggle_button->can_event, 0);
}

static StatusCode prv_initialize_momentary_switch(CenterConsoleInput *input) {
  const GpioSettings button_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  status_ok_or_return(gpio_init_pin(&input->pin_address, &button_input_settings));

  // Initialize GPIO Interrupts to raise events to change LED status
  status_ok_or_return(gpio_it_register_interrupt(&input->pin_address, &interrupt_settings,
                                                 INTERRUPT_EDGE_RISING, prv_gpio_toggle_callback,
                                                 input));

  return STATUS_CODE_OK;
}

static StatusCode prv_initialize_toggle_switch(CenterConsoleInput *input) {
  const GpioSettings button_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  status_ok_or_return(gpio_init_pin(&input->pin_address, &button_input_settings));

  // Initialize GPIO Interrupts to raise events to change LED status
  status_ok_or_return(gpio_it_register_interrupt(&input->pin_address, &interrupt_settings,
                                                 INTERRUPT_EDGE_RISING_FALLING,
                                                 prv_gpio_toggle_callback, input));

  return STATUS_CODE_OK;
}

static StatusCode prv_initialize_toggle_switch_workaround(CenterConsoleInput *input) {
  const GpioSettings button_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  status_ok_or_return(gpio_init_pin(&input->pin_address, &button_input_settings));

  // Note that we aren't initializing a GPIO interrupt because this is being
  // used by another input, due to a HW bug.

  return STATUS_CODE_OK;
}

// Initialize the buttons in the radio button group
static StatusCode prv_initialize_radio_button_group(CenterConsoleInput *input) {
  const GpioSettings button_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  const InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  status_ok_or_return(gpio_init_pin(&input->pin_address, &button_input_settings));

  // Initialize GPIO Interrupts to raise events to change LED status
  status_ok_or_return(gpio_it_register_interrupt(&input->pin_address, &interrupt_settings,
                                                 INTERRUPT_EDGE_RISING, prv_gpio_radio_callback,
                                                 input));

  return STATUS_CODE_OK;
}

StatusCode center_console_init(CenterConsoleStorage *storage) {
  status_ok_or_return(prv_initialize_momentary_switch(&storage->momentary_switch_lights_low_beam));
  status_ok_or_return(prv_initialize_momentary_switch(&storage->momentary_switch_lights_drl));

  // We need a special case for the Hazards button, since it is a toggle switch
  status_ok_or_return(prv_initialize_toggle_switch(&storage->toggle_switch_lights_hazards));

  status_ok_or_return(prv_initialize_radio_button_group(&storage->radio_button_drive));
  status_ok_or_return(prv_initialize_radio_button_group(&storage->radio_button_reverse));
  status_ok_or_return(prv_initialize_radio_button_group(&storage->radio_button_neutral));

  // TODO: Move this into the above once this gets fixed in hardware
  // This gets initialized with the EXTI line
  status_ok_or_return(prv_initialize_toggle_switch_workaround(&storage->toggle_switch_power));

  storage->prev_state = GPIO_STATE_LOW;

  return STATUS_CODE_OK;
}

void center_console_poll(CenterConsoleStorage *storage) {
  volatile GpioState curr_state = GPIO_STATE_LOW;

  gpio_get_state(&storage->toggle_switch_power.pin_address, &curr_state);
  // LOW -> HI
  if (curr_state == GPIO_STATE_HIGH && storage->prev_state == GPIO_STATE_LOW) {
    // Raise event via CAN message
    CAN_TRANSMIT_CENTER_CONSOLE_EVENT(storage->toggle_switch_power.can_event, 0);
  }
  storage->prev_state = curr_state;
}

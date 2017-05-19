#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>

#include "status.h"
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"

static GPIO_TypeDef *s_gpio_port_map[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF };
static uint32_t s_gpio_rcc_ahb_timer_map[] = { RCC_AHBPeriph_GPIOA, RCC_AHBPeriph_GPIOB,
                                               RCC_AHBPeriph_GPIOC, RCC_AHBPeriph_GPIOD,
                                               RCC_AHBPeriph_GPIOE, RCC_AHBPeriph_GPIOF };

// Determines if an GPIOAddress is valid based on the defined number of ports and pins.
static bool prv_is_address_valid(const GPIOAddress *address) {
  return !(address->port >= GPIO_MCU_NUM_PORTS || address->pin >= GPIO_MCU_NUM_PINS_PER_PORT);
}

// TODO(ELEC-20): Consider moving these two functions to the header as they will be used more or
// less universally between the implementations.

// Determines if a GPIOState is valid based on the enums.
static bool prv_is_state_valid(const GPIOState *state) {
  return *state < NUM_GPIO_STATE;
}

// Determines if a GPIOSettings is valid based on the enums.
static bool prv_are_settings_valid(const GPIOSettings *settings) {
  return !(settings->direction >= NUM_GPIO_DIR || settings->state >= NUM_GPIO_STATE ||
           settings->resistor >= NUM_GPIO_RES || settings->alt_function >= NUM_GPIO_ALTFN);
}


StatusCode gpio_init(void) {
  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(GPIOAddress *address, GPIOSettings *settings) {
  if (!prv_is_address_valid(address) || !prv_are_settings_valid(settings)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  GPIO_InitTypeDef init_struct;
  uint16_t pin = 0x01 << address->pin;

  RCC_AHBPeriphClockCmd(s_gpio_rcc_ahb_timer_map[address->port], ENABLE);

  // Parse the GPIOAltFN settings which are used to modify the mode and Alt Function.
  if (settings->alt_function == GPIO_ALTFN_ANALOG) {
    init_struct.GPIO_Mode = GPIO_Mode_AN;
  } else if (settings->alt_function == GPIO_ALTFN_NONE) {
    init_struct.GPIO_Mode = (GPIOMode_TypeDef)settings->direction;
  } else {
    init_struct.GPIO_Mode = GPIO_Mode_AF;
  }
  init_struct.GPIO_PuPd = (GPIOPuPd_TypeDef)settings->resistor;
  init_struct.GPIO_Pin = pin;

  // These are default values which are not intended to be changed.
  init_struct.GPIO_Speed = GPIO_Speed_Level_3;
  init_struct.GPIO_OType = GPIO_OType_PP;
  if (init_struct.GPIO_Mode == GPIO_Mode_AF) {
    // Subtract 1 due to the offset of the enum from the ALTFN_NONE entry
    GPIO_PinAFConfig(s_gpio_port_map[address->port], address->pin, settings->alt_function - 1);
  }

  // Set the pin state.
  gpio_set_pin_state(address, settings->state);

  // Use the init_struct to set the pin.
  GPIO_Init(s_gpio_port_map[address->port], &init_struct);
  return STATUS_CODE_OK;
}

StatusCode gpio_set_pin_state(GPIOAddress *address, GPIOState state) {
  if (!prv_is_address_valid(address) || !prv_is_state_valid(&state)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  GPIO_WriteBit(s_gpio_port_map[address->port], 0x01 << address->pin, (BitAction)state);
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(GPIOAddress *address) {
  if (!prv_is_address_valid(address)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint16_t pin = 0x01 << address->pin;
  uint8_t state = GPIO_ReadOutputDataBit(s_gpio_port_map[address->port], pin);
  if (state) {
    GPIO_ResetBits(s_gpio_port_map[address->port], pin);
  } else {
    GPIO_SetBits(s_gpio_port_map[address->port], pin);
  }
  return STATUS_CODE_OK;
}

StatusCode gpio_get_value(GPIOAddress *address, GPIOState *input_state) {
  if (!prv_is_address_valid(address)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *input_state = GPIO_ReadInputDataBit(s_gpio_port_map[address->port], 0x01 << address->pin);
  return STATUS_CODE_OK;
}

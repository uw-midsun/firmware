#include "adc.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// LOG_DEBUG the adc converted value of context pin when called
void log_adc_output(const GpioAddress *address, void *context) {
    AdcChannel *output_channel = context;
    uint16_t output_data = 0;
    adc_read_converted(*output_channel, &output_data);
    LOG_DEBUG("adc converted pin data: %d\n", output_data);
}


int main(void) {
    interrupt_init();
    soft_timer_init();
    gpio_init();
    adc_init(ADC_MODE_SINGLE);

    // output gpio pin A6
    GpioAddress output_addr = { .port = GPIO_PORT_A, .pin = 6 };
    GpioSettings output_settings = {
        GPIO_DIR_IN,
        GPIO_STATE_LOW,
        GPIO_RES_NONE,
        GPIO_ALTFN_ANALOG,
    };
    gpio_init_pin(&output_addr, &output_settings);
    // output channel
    AdcChannel output_channel = NUM_ADC_CHANNELS;

    adc_get_channel(output_addr, &output_channel);
    adc_set_channel(output_channel, true);

    // button gpio pin
    GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };
    GpioSettings button_settings = {
        GPIO_DIR_IN,
        GPIO_STATE_LOW,
        GPIO_RES_NONE,
        GPIO_ALTFN_ANALOG,
    };
    gpio_init_pin(&button_addr, &button_settings);
    
    InterruptSettings interrupt_settings = {
        INTERRUPT_TYPE_INTERRUPT,
        INTERRUPT_PRIORITY_HIGH,
    };
    
    // calls log_adc_output on the falling edge of button press, 
    // with context of the adc channel for output pin A6
    gpio_it_register_interrupt(&button_addr, &interrupt_settings, 
        INTERRUPT_EDGE_FALLING, log_adc_output, &output_channel);

    LOG_DEBUG("A6 pin ADC Channel: %d\n", output_channel);

    while (true) {
        wait();
    }
}
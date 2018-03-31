#include "power_path.h"

#include <stdbool.h>
#include <string.h>

#include "adc.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "status.h"

// All values in millivolts
#define POWER_PATH_AUX_UV_MILLIV 8460
#define POWER_PATH_AUX_OV_MILLIV 14310
#define POWER_PATH_DCDC_UV_MILLIV 11160
#define POWER_PATH_DCDC_OV_MILLIV 12840

// Evaluates if two GPIO addresses are equal.
static bool prv_addr_eq(GPIOAddress addr1, GPIOAddress addr2) {
  return ((addr1.port == addr2.port) && (addr1.pin == addr2.pin));
}

// Interrupt handler for over and under voltage warnings.
static void prv_voltage_warning(const GPIOAddress *addr, void *context) {
  PowerPathCfg *pp = context;
  PowerPathVCReadings readings = { 0 };
  StatusCode status = STATUS_CODE_OK;
  if (prv_addr_eq(*addr, pp->dcdc.uv_ov_pin) && pp->dcdc.monitoring_active) {
    power_path_read_source(&pp->dcdc, &readings);
    status =
        CAN_TRANSMIT_OVUV_DCDC_AUX((readings.voltage >= POWER_PATH_DCDC_OV_MILLIV),
                                   (readings.voltage <= POWER_PATH_DCDC_UV_MILLIV), false, false);
  } else if (prv_addr_eq(*addr, pp->aux_bat.uv_ov_pin) && pp->aux_bat.monitoring_active) {
    power_path_read_source(&pp->aux_bat, &readings);
    status =
        CAN_TRANSMIT_OVUV_DCDC_AUX(false, false, (readings.voltage >= POWER_PATH_AUX_OV_MILLIV),
                                   (readings.voltage <= POWER_PATH_AUX_UV_MILLIV));
  }
  if (!status_ok(status)) {
    event_raise(CHAOS_EVENT_CAN_TRANSMIT_ERROR, SYSTEM_CAN_MESSAGE_OVUV_DCDC_AUX);
  }
}

static void prv_adc_read(SoftTimerID timer_id, void *context) {
  PowerPathSource *pps = context;
  if (pps->timer_id != timer_id || !pps->monitoring_active) {
    // Guard against accidentally starting multiple timers to monitor the same source concurrently.
    return;
  }

  uint16_t value = 0;
  ADCChannel chan = NUM_ADC_CHANNELS;
  // Read and convert the current values.
  adc_get_channel(pps->current_pin, &chan);
  adc_read_raw(chan, &value);
  pps->readings.current = pps->current_convert_fn(value);

  // Read and convert the voltage values.
  adc_get_channel(pps->voltage_pin, &chan);
  adc_read_raw(chan, &value);
  pps->readings.voltage = pps->voltage_convert_fn(value);

  // Start the next timer.
  soft_timer_start(pps->period_us, prv_adc_read, pps, &pps->timer_id);
}

StatusCode power_path_init(PowerPathCfg *pp) {
  GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  status_ok_or_return(gpio_init_pin(&pp->enable_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->shutdown_pin, &settings));

  // Configure the Interrupt Pins
  settings.direction = GPIO_DIR_IN;
  settings.state = GPIO_STATE_LOW;
  status_ok_or_return(gpio_init_pin(&pp->aux_bat.uv_ov_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->dcdc.uv_ov_pin, &settings));

  // Register interrupts to the same function.
  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  status_ok_or_return(gpio_it_register_interrupt(&pp->aux_bat.uv_ov_pin, &it_settings,
                                                 INTERRUPT_EDGE_RISING, prv_voltage_warning, pp));
  status_ok_or_return(gpio_it_register_interrupt(&pp->dcdc.uv_ov_pin, &it_settings,
                                                 INTERRUPT_EDGE_RISING, prv_voltage_warning, pp));

  // Configure the ADC Pins
  settings.alt_function = GPIO_ALTFN_ANALOG;
  status_ok_or_return(gpio_init_pin(&pp->aux_bat.voltage_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->aux_bat.current_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->dcdc.voltage_pin, &settings));
  return gpio_init_pin(&pp->dcdc.current_pin, &settings);
}

StatusCode power_path_source_monitor_enable(PowerPathSource *source, uint32_t period_us) {
  // Update the period.
  source->period_us = period_us;

  // Avoid doing anything else if monitoring
  if (source->monitoring_active) {
    return STATUS_CODE_OK;
  }

  // Set up adc current/voltage monitoring
  ADCChannel chan = NUM_ADC_CHANNELS;
  adc_get_channel(source->current_pin, &chan);
  status_ok_or_return(adc_set_channel(chan, true));
  adc_get_channel(source->voltage_pin, &chan);
  status_ok_or_return(adc_set_channel(chan, true));

  source->monitoring_active = true;

  return soft_timer_start(source->period_us, prv_adc_read, source, &source->timer_id);
}

StatusCode power_path_source_monitor_disable(PowerPathSource *source) {
  if (!source->monitoring_active) {
    return STATUS_CODE_OK;
  }

  source->monitoring_active = false;
  // Ignore this return since we don't care if the timer is already disabled.
  soft_timer_cancel(source->timer_id);

  // Disable the ADCs
  ADCChannel chan = NUM_ADC_CHANNELS;
  adc_get_channel(source->current_pin, &chan);
  status_ok_or_return(adc_set_channel(chan, false));
  adc_get_channel(source->voltage_pin, &chan);
  return adc_set_channel(chan, false);
}

StatusCode power_path_read_source(const PowerPathSource *source, PowerPathVCReadings *values) {
  if (!source->monitoring_active) {
    // Fail if not actively monitoring as the data is not up to date.
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  *values = source->readings;
  return STATUS_CODE_OK;
}

bool power_path_process_event(PowerPathCfg *cfg, const Event *e) {
  PowerPathSource *source = NULL;
  if (e->data == POWER_PATH_SOURCE_ID_DCDC) {
    source = &cfg->dcdc;
  } else if (e->data == POWER_PATH_SOURCE_ID_AUX_BAT) {
    source = &cfg->aux_bat;
  } else {
    return false;
  }

  if (e->id == CHAOS_EVENT_MONITOR_ENABLE) {
    power_path_source_monitor_enable(source, source->period_us);
    return true;
  } else if (e->id == CHAOS_EVENT_MONITOR_DISABLE) {
    power_path_source_monitor_disable(source);
    return true;
  }

  return false;
}

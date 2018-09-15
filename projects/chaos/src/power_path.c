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
#include "soft_timer.h"
#include "status.h"

// All values in millivolts
#define POWER_PATH_AUX_UV_MILLIV 8460
#define POWER_PATH_AUX_OV_MILLIV 14310
#define POWER_PATH_DCDC_UV_MILLIV 11160
#define POWER_PATH_DCDC_OV_MILLIV 12840

// Evaluates if two GPIO addresses are equal.
static bool prv_addr_eq(GpioAddress addr1, GpioAddress addr2) {
  return ((addr1.port == addr2.port) && (addr1.pin == addr2.pin));
}

static void prv_send(SoftTimerID timer_id, void *context) {
  (void)timer_id;
  PowerPathCfg *cfg = context;
  PowerPathVCReadings aux = { 0 };
  power_path_read_source(&cfg->aux_bat, &aux);
  PowerPathVCReadings dcdc = { 0 };
  power_path_read_source(&cfg->dcdc, &dcdc);
  CAN_TRANSMIT_AUX_DCDC_VC(aux.voltage, aux.current, dcdc.voltage, dcdc.current);
  soft_timer_start_millis(cfg->period_millis, prv_send, context, NULL);
}

// Interrupt handler for over and under voltage warnings.
static void prv_voltage_warning(const GpioAddress *addr, void *context) {
  PowerPathCfg *pp = context;
  StatusCode status = STATUS_CODE_OK;
  if (prv_addr_eq(*addr, pp->dcdc.uv_ov_pin) && pp->dcdc.monitoring_active) {
    PowerPathVCReadings readings = { 0 };
    power_path_read_source(&pp->dcdc, &readings);
    status =
        CAN_TRANSMIT_OVUV_DCDC_AUX((readings.voltage >= POWER_PATH_DCDC_OV_MILLIV),
                                   (readings.voltage <= POWER_PATH_DCDC_UV_MILLIV), false, false);
  } else if (prv_addr_eq(*addr, pp->aux_bat.uv_ov_pin) && pp->aux_bat.monitoring_active) {
    PowerPathVCReadings readings = { 0 };
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
  pps->timer_id = SOFT_TIMER_INVALID_TIMER;

  uint16_t value = 0;
  ADCChannel chan = NUM_ADC_CHANNELS;
  // Read and convert the current values.
  adc_get_channel(pps->current_pin, &chan);
  adc_read_converted(chan, &value);
  pps->readings.current = pps->current_convert_fn(value);

  // Read and convert the voltage values.
  value = 0;
  adc_get_channel(pps->voltage_pin, &chan);
  adc_read_converted(chan, &value);
  pps->readings.voltage = pps->voltage_convert_fn(value);

  // Start the next timer.
  soft_timer_start_millis(pps->period_millis, prv_adc_read, context, &pps->timer_id);
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
  pp->aux_bat.timer_id = SOFT_TIMER_INVALID_TIMER;
  pp->dcdc.timer_id = SOFT_TIMER_INVALID_TIMER;

  // Configure the ADC Pins
  settings.alt_function = GPIO_ALTFN_ANALOG;
  status_ok_or_return(gpio_init_pin(&pp->aux_bat.voltage_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->aux_bat.current_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->dcdc.voltage_pin, &settings));
  status_ok_or_return(gpio_init_pin(&pp->dcdc.current_pin, &settings));
  status_ok_or_return(power_path_source_monitor_disable(&pp->aux_bat));
  return power_path_source_monitor_disable(&pp->dcdc);
}

StatusCode power_path_send_data_daemon(PowerPathCfg *pp, uint32_t period_millis) {
  pp->period_millis = period_millis;
  return soft_timer_start_millis(pp->period_millis, prv_send, pp, NULL);
}

StatusCode power_path_source_monitor_enable(PowerPathSource *source, uint32_t period_millis) {
  // Update the period.
  source->period_millis = period_millis;

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

  return soft_timer_start_millis(source->period_millis, prv_adc_read, source, &source->timer_id);
}

StatusCode power_path_source_monitor_disable(PowerPathSource *source) {
  if (!source->monitoring_active) {
    return STATUS_CODE_OK;
  }

  source->monitoring_active = false;
  // Ignore this return since we don't care if the timer is already disabled.
  if (source->timer_id != SOFT_TIMER_INVALID_TIMER) {
    soft_timer_cancel(source->timer_id);
    source->timer_id = SOFT_TIMER_INVALID_TIMER;
  }

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
  switch (e->id) {
    case CHAOS_EVENT_MONITOR_ENABLE:
      if (e->data == POWER_PATH_SOURCE_ID_DCDC) {
        power_path_source_monitor_enable(&cfg->dcdc, cfg->dcdc.period_millis);
      } else if (e->data == POWER_PATH_SOURCE_ID_AUX_BAT) {
        power_path_source_monitor_enable(&cfg->aux_bat, cfg->aux_bat.period_millis);
      } else {
        return false;
      }
      return true;
    case CHAOS_EVENT_MONITOR_DISABLE:
      if (e->data == POWER_PATH_SOURCE_ID_DCDC) {
        power_path_source_monitor_disable(&cfg->dcdc);
      } else if (e->data == POWER_PATH_SOURCE_ID_AUX_BAT) {
        power_path_source_monitor_disable(&cfg->aux_bat);
      } else {
        return false;
      }
      return true;
    default:
      return false;
  }
}

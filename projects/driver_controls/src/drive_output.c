#include "drive_output.h"
#include <string.h>
#include "log.h"

#define DRIVE_OUTPUT_VALID_WATCHDOG ((1 << NUM_DRIVE_OUTPUT_SOURCES) - 1)

static void prv_watchdog_cb(SoftTimerID timer_id, void *context) {
  DriveOutput *output = context;

  // We're missing at least one updated response
  if (output->watchdog != DRIVE_OUTPUT_VALID_WATCHDOG) {
    // Error - raise a warning, disable periodic drive output
    soft_timer_cancel(output->output_timer);
    event_raise(output->fault_event, 0);
  }

  // Reset watchdog
  output->watchdog = 0;

  soft_timer_start_millis(DRIVE_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, context,
                          &output->watchdog_timer);
}

static void prv_broadcast_cb(SoftTimerID timer_id, void *context) {
  DriveOutput *output = context;

  event_raise(output->update_req_event, 0);
  // note that this will usually output stale data from the previous update request

  // TODO: output CAN message
  LOG_DEBUG("Drive output: throttle %d cruise %d direction %d steering angle %d\n",
            output->data[DRIVE_OUTPUT_SOURCE_THROTTLE], output->data[DRIVE_OUTPUT_SOURCE_CRUISE],
            output->data[DRIVE_OUTPUT_SOURCE_DIRECTION],
            output->data[DRIVE_OUTPUT_SOURCE_STEERING_ANGLE]);

  soft_timer_start_millis(DRIVE_OUTPUT_BROADCAST_MS, prv_broadcast_cb, context,
                          &output->output_timer);
}

StatusCode drive_output_init(DriveOutput *output, EventID fault_event, EventID update_req_event) {
  memset(output, 0, sizeof(*output));
  output->fault_event = fault_event;
  output->update_req_event = update_req_event;

  output->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
  output->output_timer = SOFT_TIMER_INVALID_TIMER;

  return STATUS_CODE_OK;
}

StatusCode drive_output_enable(DriveOutput *output, bool enabled) {
  soft_timer_cancel(output->watchdog_timer);
  soft_timer_cancel(output->output_timer);

  if (enabled) {
    // Reset watchdog
    output->watchdog = 0;

    StatusCode ret = soft_timer_start_millis(DRIVE_OUTPUT_WATCHDOG_MS, prv_watchdog_cb, output,
                                             &output->watchdog_timer);
    status_ok_or_return(ret);

    return soft_timer_start_millis(DRIVE_OUTPUT_BROADCAST_MS, prv_broadcast_cb, output,
                                   &output->output_timer);
  } else {
    output->watchdog_timer = SOFT_TIMER_INVALID_TIMER;
    output->output_timer = SOFT_TIMER_INVALID_TIMER;
  }

  return STATUS_CODE_OK;
}

StatusCode drive_output_update(DriveOutput *output, DriveOutputSource source, int16_t data) {
  if (source >= NUM_DRIVE_OUTPUT_SOURCES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  output->data[source] = data;
  output->watchdog |= 1 << source;

  return STATUS_CODE_OK;
}

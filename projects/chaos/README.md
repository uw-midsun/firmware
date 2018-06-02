# Chaos

Power distribution for MSXII

## Summary

Chaos is the power distribution component of the vehicle. It acts like the
circulatory system of a person distributing resources throughout the body. In
this analogy the blood/oxygen are the electrons flowing through the vehicle to
power different parts. The circulatory system is smart and can reduce output to
unused areas of our body when we are sleeping. Chaos is much the same way and
turns off boards and listen for faults telling it when to turn off or restrict
power flow. However, the human circulatory system is also not automatic it
requires the nervous system in order to tell it what to do. In our analogy this
is driver controls which sends state commands to Chaos.

## HV vs LV

Chaos controls high voltage (HV) logic through relays which it communicates with
via CAN. Low voltage (LV) systems have their power state controlled locally
through GPIO pins.

## States

Chaos has four internal states:

- Idle: the car is turned off, this minimizes power usage and limits
  functionality.
- Charging: the car is capable of charging both from the solar array and
  external sources.
- Drive: the car is fully powered and capable of driving.
- Emergency: A fault state which attempts to turn everything high voltage off
  and keep active only the minimal number of low voltage systems to recover.

## Architecture

Chaos' architecture is based on a central event loop which is subscribed to by
various services. These services read each message from the event loop. If the
message applies to them they handle it, otherwise it is skipped.

The following services are running:

- `charger`: manages whether the charger system should connect or not. Listens
  for open or close events on the main loop to change state.
- `emergency_fault`: listens for emergency events on the main loop. If it gets
  one it attempts to broadcast the fault to driver controls until successful or
  it has transitioned to a different state.
- `gpio_fsm`: a service which handles GPIO pin update requests and executes on
  them. Usually this is performed to a bulk set of GPIO pins but the transition
  always occurs sequentially.
- `power_path`: monitors the power path IC for undervoltage and overvoltage
  protection of the DCDCs (if the battery relay is closed) and the AUX Battery.
  Also monitors current and voltage. Listens for events to start and stop
  monitoring the DCDCs depending on if the relays powering them are closed.
- `relay`: this service manages the opening and closing of relays over CAN. It
  sends a transition request and listens for an ACK. When received it raises an
  affirmative message into the event loop. If no ACK is received it raises a
  message into the event loop asking to attempt a retry. 
- `relay_retry_service`: listens for relay retry requests and decides whether to
  retry of raise a relay failure message if the retry limit is exceeded. Also
  listens for configuration updates over the event loop.
- `powertrain_heartbeat`: listens for the completion of transition to the Drive
  state. In the drive state it starts a watchdog and pulses a heartbeat to
  ensure the motor interface, battery management system and driver controls are
  OK. Will raise an event to fail to the Emergency state if the watchdog
  expires. Turns off the heartbeat/watchdog when outside of the drive state.

Services not on the event loop:

- `bps_heartbeat`: listens for a periodic heartbeat form the BPS board. If not
  received an event is raised to transition to the Emergency state.
- `state_handler`: listens for updates from driver controls to change states.

During a transition between states the events in the event loop are driven by
the `sequencer_fsm`. The FSM first flushes the previous sequence, waiting for
any async (CAN) events to finish before transitioning. During the transition it
manages the `event_queue` monitoring for unexpected events and ensuring proper
ordering of events is occurring. It also has the capability to await relay 
completion allowing the retry logic to run while the sequencer maintains its
state. This is essentially a generator with additional validation logic.


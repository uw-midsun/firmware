# Driver Controls: Pedal

## Summary

The Pedal board is the board responsible for:

* Throttle
* Mechanical brake

This board is chosen to act as the Driver Controls master, as being able to
quickly respond to changes in Throttle and Mechanical brake position is
important in terms of safety. Were the Pedal board not the master, then Pedal
position would be exposed via periodic CAN message to the master, which might
be either:

1. Delayed when sent from the Pedal board
2. Delayed when processed by the master
3. Delayed when read from the ADC

Making the Pedal board the master eliminates the possibility of 1, and reduces
the effects of 2.

## States

The Pedal board is responsible for arbitrating

##

Cruise control is transactional, in that target speeds must be "requested"
and "committed" before they can actually "enable" Cruise Control.

We define 3 separate states:

* Cruise Off: cruise control is off
* Cruise Ready: cruise control is waiting to commit the Cruise Target
* Cruise On: cruise control is active and maintaining the current Cruise Target

In order to activate Cruise, we trigger Cruise Control on the Stalk using the
`CONTROL_STALK_DIGITAL_CC_SET`, which enables the analog input. On the first
time Cruise is activated

Pressing the mechanical brake results in transitioning back to the Cruise Ready
state, which 

The `SET` button is used to "commit" the current target speed.

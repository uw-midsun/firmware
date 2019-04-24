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

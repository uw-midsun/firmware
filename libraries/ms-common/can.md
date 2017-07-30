# CAN Internals

One of our primary goals for this driver is to prevent CAN from starving the main event loop.
Ideally, we should design our network such that that never happens, but we always plan for the worst.
The general concept for achieving this is using our event queue to postpone processing to the CAN FSM.

For received packets, we push the packet into a queue and raise an event to signal the FSM. In the
FSM, we can pop the message and either run the associated RX callback or complete a pending ACK.
Since the ISR is called for each received packet, our RX events are 1-to-1.

For transmitting packets, our goal is to limit how quickly we attempt to transmit. If we chain
transmits using the TX ready interrupt, it's very easy to starve the main loop. Thus, we just use
events to trigger an attempt to transmit. Normally, we use the TX ready ISR to trigger the next
transmit. If no transmit is currently in progress (so no TX ready interrupt), we kickstart the
transmit cycle with a TX event.

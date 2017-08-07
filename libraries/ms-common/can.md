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

Right now, it seems to be better if we just throw away messages that failed to transmit.
Receive seems to work perfectly fine - it's just transmit that's having problems.
Right now, I'm trying to check if any messages are being dropped. Unfortunately, the only way I have
to test this is to increment the message ID and see if there are any gaps in the received packets.
However, this is limited to the maximum message ID, after which it overflows back to 0. Since we
use a min priority queue to buffer the messages, the order may be swapped at that point, giving
a false positive. This is also why we can't rely on the ordering of our messages to be constant,
as the priority queue algorithm isn't stable. This means we can't just increment the data field
and keep the ID the same. This may have been a poor decision for transmits.

I've now moved CAN to use FIFOs instead. Priority queue was not a great idea since we might have
some messages that rely on ordering.

We might want to just keep track of the number of missed transmits. Then, when the TX ready interrupt
fires, it can generate an event for each missed TX. We could even generate a single event, as there
isn't any point in trying to transmit more than once for each TX ready interrupt.

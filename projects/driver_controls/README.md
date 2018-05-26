# Driver Controls

This board handles all user-supplied input and some critical outputs. Inputs have 4 primary sources:

* **Center Console**: Buttons for power, hazards, etc. Also has LEDs for critical feedback such as BPS fault.
* **Pedal Board**: Monitors throttle and mechanical brake state through I2C.
* **Steering Wheel/Control Stalks**: Monitors horn and control stalk states through I2C.
* **Steering Angle Sensor**: Analog rotary sensor attached to the steering column. Purely for telemetry.

## FSMs

There are a number of FSMs that control the state of the car. The most complex set governs drive output messages to the motor controllers. In order of hierarchy, they are:

* **Power FSM**: Highest priority FSM. In all states except On, blocks all FSMs.
* **Mechanical Brake FSM**: If the mechanical brake is active, prevents Cruise FSM from entering cruise control state. Otherwise, blocks Direction FSM from changing direction state.
* **Direction FSM**: If in reverse, prevents Cruise FSM from entering cruise control state.
* **Pedal FSM**: If braking, prevents Cruise FSM from entering cruise control state.
* **Cruise FSM**: Lowest priority FSM. Generates drive output messages.

To decouple FSMs from each other, instead of explicitly blocking other FSMs from transitioning, they report to an event arbiter. As the arbiter handles events, it asks all FSMs for guards which act as event masks. Those masks are ANDed to create a filter. If any FSMs blocks an event, it is blocked from all FSMs. As FSMs transition states, they can choose to change their masks. Note that the change is not explicitly tied to FSM transitions, but it happens to be the primary purpose for now.

The result of the event blocking is:

* Power FSM prevents all events if not On except those required to transition it to On.
* Mechanical FSM prevents Cruise FSM from entering cruise control state if activated.
* If mechanical brake is not activated, the Direction FSM is locked.
* Direction FSM prevents Cruise FSM from entering cruise control state if not forward. Since changing direction requires activating the mechanical brake and the Cruise FSM has a transition to exit cruise on mechanical braking, we cannot change direction without exiting cruise.
* Pedal FSM prevents Cruise FSM from entering cruise control state if braking.

Note that this means cruise control can be entered while regen braking. We assume motor controller interface will ignore throttle position if the mechanical brake is detected.
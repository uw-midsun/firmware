# Driver Controls: Center Console

## Summary

The LED buttons are as follows:

### Buttons

#### Radio Button Groups

These must have 1 of the option enabled

```
+-------+---------+---------+
| Drive | Reverse | Neutral |
+-------+---------+---------+
```

The status of these LEDs should reflect the current state of the car. In other
words, the LED status changes only when the `DRIVE_OUTPUT` message direction
changes.

#### Toggle Buttons

These are your normal On/Off toggle buttons.

```
+-------+  +---------+  +-----------+  +------------------------+
| Power |  | Hazards |  | Low Beams |  | Daytime Running Lights |
+-------+  +---------+  +-----------+  +------------------------+
```

The status of these LEDs should reflect the current state of the car.

In other words, the LED status changes only when the `POWER_STATE` message's
Power State changes.

The `LIGHTS_STATE` message is used to update the state of the LEDs indicating
which lights mode we are in.

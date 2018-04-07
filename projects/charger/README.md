# Charger

This is the master side of the two microcontroller level 2 charger interface for
MSXII. The charger in question is the
[Elcon HK-J-H198-23](https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/92635386/Charger+and+Required+Parts?preview=/92635386/105545732/HK-J-H198-23-verA.doc)
This charger utilizes a standard J1939 CAN specification commonly used in EVs.

## Functions

This board has three primary functions
- Request permission to charge when plugged in
- Translate messages between the charger CAN spec and the MSXII CAN spec
- Maintain communications with the charger such that it doesn't enter a fault
condition

## States

- Disconnected: there is no connected charger so the board shouldn't send charge
requests.
- Connected: an external charger is connected but charging is not permitted.
Send requests to charge.
- Charging: an external charger is connected and charging the car.

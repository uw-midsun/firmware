# Firmware Tutorial

This is the Midnight Sun firmware tutorial module which introduces fundamental hardware
and firmware concepts from reading schematics to writing functional code following our 
standards. This module requires the use of the Midnight Sun Tutorial Board and is a series
of 2 modules where once completed, will enable one to become a contributing member to our
firmware team! 

## Module 1: Digital and Analog I/O

The first module introduces to concept of writing firmware, more specifically, Midnight
Sun approved firmware! The module goes over key concepts such as file layout/structure,
reading schematics, logging/debugging and writing unit tests. By the end of this module, 
one should be familiar with interacting with hardware peripherals and reading in both 
analog and digital signals. 

Problem Statement: 
- Control LEDs with corresponding buttons through a microcontroller
- Adjust brightness of LEDs using potentiometer while logging the analog data

## Module 2: CAN

The second module goes over our most commonly used protocol, CAN. This module takes one
through the fundamentals of CAN messages, creating new CAN messages, utilizing CAN to 
send messages across boards and debugging CAN messages via a Peak CAN. By the end of this
module, one should be familiar with our codegen repository and have to ability to 
initiate effective board-to-board communication through the use of CAN messages.

Problem Statement:
- Define and generate custom CAN message through our codegen repository
- Control LEDs with through CAN messages from one board to another

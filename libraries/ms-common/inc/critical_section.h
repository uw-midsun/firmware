#pragma once
// Critical sections to protect code that should not be interrupted.

#include <stdbool.h>

// To protect critical code use the following two functions:
// EXAMPLE:
//
// bool disabled = critical_section_start();
// // Critical code here.
// critical_section_end(disabled);
// // ...
//
// This will also protect nested attempts from enabling and disabling interrupts from prematurely
// ending the critical section.

// Disables all interrupts across all lines/inputs. Returns true if the function disabled
// interrupts.
bool critical_section_start(void);

// Enables all registered interrupts on all lines/inputs. Passing true to this function can be used
// to forcibly end all critical sections.
void critical_section_end(bool disabled_in_scope);

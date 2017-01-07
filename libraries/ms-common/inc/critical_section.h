#pragma once

#include <stdbool.h>

// To protect critical code use the following two functions:
// EXAMPLE:
//
// bool disabled = critical_section_start();
// // Critical code here.
// critical_section_end(disabled);
// ...
//
// This will also protect nested attempts from enabling and disabling interupts from prematurely
// ending the critical section.

// Disables all interrupts accross all lines/inputs. Returns true if the function disabled
// interrupts.
bool critical_section_start(void);

// Enables all registered interrupts on all line/inputs. Passing true to this can be used to
// forcibly end all critical sections.
void critical_section_end(bool disabled_in_scope);

#pragma once
// Critical sections to protect code that should not be interrupted.

#include <stdbool.h>
#include <stdint.h>

// To protect critical code use the following two functions:
// Example:
//
// CriticalSection section = critical_section_start();
// // Critical code here.
// critical_section_end(&section);
// // ...
//
// This will also protect nested attempts from enabling and disabling interrupts from prematurely
// ending the critical section.
//
// For functions that are completely wrapped in critical sections, use CRITICAL_SECTION_AUTOEND to
// mark the whole function as critical. The critical section will automatically be ended after the
// function returns.
// Example:
//
// void foo(void) {
//   CRITICAL_SECTION_AUTOEND;
//   // Critical code here.
// }

typedef struct CriticalSection {
  bool disabled_in_scope;
  bool requires_cleanup;
} CriticalSection;

#define CRITICAL_SECTION_AUTOEND                                            \
  __attribute__((cleanup(critical_section_end))) CriticalSection _section = \
      critical_section_start();

// Disables all interrupts across all lines/inputs.
// Returns |CriticalSection.disabled_in_scope| = true if the function disabled interrupts.
CriticalSection critical_section_start(void);

// Enables all registered interrupts on all lines/inputs.
// Passing |section.disabled_in_scope| = true to this function can be used
// to forcibly end all critical sections.
void critical_section_end(CriticalSection *section);

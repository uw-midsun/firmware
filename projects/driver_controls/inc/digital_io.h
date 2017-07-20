#pragma once

// Provides an interface to configure the GPIO pins for the driver control devices.
// This allows a cleaner device initialization than having to initialize each pin with
// different settings

// Initialize digital gpio pins and interrupts
void digital_io_init();

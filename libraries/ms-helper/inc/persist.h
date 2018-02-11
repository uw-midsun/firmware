#pragma once
// Implements a flash-based persistance layer
// Requires flash and soft timers to be initialized.
//
// Allocates one page of flash to keep data across resets.
// Data is written to flash periodically. Note that to reduce necessary wear on the flash, data is
// only committed if changes have occurred.
#include <stddef.h>
#include "status.h"
#include "flash.h"

// Commit data every second if dirty
#define PERSIST_COMMIT_TIMEOUT_MS 1000
#define PERSIST_FLASH_PAGE (NUM_FLASH_PAGES - 1)

typedef struct PersistStorage {
  void *blob;
  size_t blob_size;
  uintptr_t flash_addr;
  uintptr_t prev_flash_addr;
} PersistStorage;

// Attempt to load stored data into the provided blob and retains the blob to commit periodically
// Note that the blob must be a multiple of FLASH_WRITE_BYTES and must persist
StatusCode persist_init(PersistStorage *persist, void *blob, size_t blob_size);

// Force a data commit - this should be avoided if possible.
StatusCode persist_commit(PersistStorage *persist);

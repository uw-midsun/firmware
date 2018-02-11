#include "persist.h"
#include "log.h"
// The persistance layer allocates one page of flash so we can erase the entire page when full.
// To reduce the number of erases, we partition the page into a number of sections. Each section
// is the size of the specified blob plus a header. The header is used to mark the section as valid
// and record the blob's size.
//
// By default, all sections are invalid. Only one section should be valid at a time. At each write,
// all previous sections are marked as invalid and the new section is considered valid.
//
// Section format: [ marker (u32) | size (u32) | blob (u32 * n) ]
// We initially write to the size field and blob data, skipping the marker. We then consider the
// default unwritten value of marker as "valid" (0xFFFFFFFF). To invalidate the section, we write
// 0x0 to the marker field. Because we skipped it earlier, this write succeeds and does not require
// erasing the page. If all sections are invalid, we erase the entire page.
//
// At init, the persistance layer attemps to load the blob with the data stored in flash.

// Erased flash defaults to all 1's
#define PERSIST_VALID_MARKER 0xFFFFFFFF
#define PERSIST_INVALID_SIZE 0xFFFFFFFF
#define PERSIST_INVALID_ADDR UINTPTR_MAX
#define PERSIST_BASE_ADDR FLASH_PAGE_TO_ADDR(PERSIST_FLASH_PAGE)
#define PERSIST_END_ADDR (PERSIST_BASE_ADDR + FLASH_PAGE_BYTES)

typedef struct PersistHeader {
  uint32_t marker;
  uint32_t size_bytes;
} PersistHeader;

StatusCode persist_init(PersistStorage *persist, void *blob, size_t blob_size) {
  if (blob_size > FLASH_PAGE_BYTES) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  } else if (blob_size % FLASH_WRITE_BYTES != 0) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  persist->blob = blob;
  persist->blob_size = blob_size;
  persist->prev_flash_addr = PERSIST_INVALID_ADDR;

  // Load stored data
  PersistHeader header = {
    .marker = PERSIST_VALID_MARKER, //
    .size_bytes = 0 //
  };
  persist->flash_addr = PERSIST_BASE_ADDR;

  // Essentially, we want to exit in three scenarios:
  // * The page has been erased and there are no valid sections. Use the base address.
  //   Marker == 0xFFFFFFFF, Size == 0xFFFFFFFF
  // * This is an invalid section. Increment the address by (header + blob).
  //   Marker != 0xFFFFFFFF, Size != 0xFFFFFFFF
  // * This is a valid section. Use this address.
  //   Marker == 0xFFFFFFFF, Size != 0xFFFFFFFF
  do {
    status_ok_or_return(flash_read(persist->flash_addr, sizeof(header),
                        (uint8_t *)&header, sizeof(header)));
    if (header.marker != PERSIST_VALID_MARKER) {
      persist->flash_addr += sizeof(header) + header.size_bytes;
    }

    if (persist->flash_addr >= PERSIST_END_ADDR) {
      // Somehow had zero valid sections remaining - erase page
      LOG_DEBUG("Somehow started with a full invalid page - erasing page\n");
      status_ok_or_return(flash_erase(PERSIST_FLASH_PAGE));
      persist->flash_addr = PERSIST_BASE_ADDR;
    }
  } while (header.marker != PERSIST_VALID_MARKER);

  if (header.size_bytes == PERSIST_INVALID_SIZE) {
    LOG_DEBUG("No valid sections found! New persist data will live at 0x%lx\n",
              persist->flash_addr);
  } else {
    LOG_DEBUG("Found valid section at 0x%lx (0x%x bytes), loading data\n",
              persist->flash_addr, header.size_bytes);
    StatusCode ret = flash_read(persist->flash_addr + sizeof(header), persist->blob_size,
                                (uint8_t *)persist->blob, persist->blob_size);
    status_ok_or_return(ret);
  }

  return STATUS_CODE_OK;
}

StatusCode persist_commit(PersistStorage *persist) {
  // Mark previous section as invalid
  if (persist->prev_flash_addr != PERSIST_INVALID_ADDR) {
    uint32_t invalid = 0;
    flash_write(persist->prev_flash_addr, (uint8_t *)&invalid, sizeof(invalid));
  }

  // Check if we're overrunning the page
  if (persist->flash_addr + sizeof(PersistHeader) + persist->blob_size > PERSIST_END_ADDR) {
    // It should be okay if we're right on the boundary?
    flash_erase(PERSIST_FLASH_PAGE);
    persist->flash_addr = PERSIST_BASE_ADDR;
  }

  // Write persist blob size, skipping the marker
  PersistHeader header = { .size_bytes = persist->blob_size };
  StatusCode ret = flash_write(persist->flash_addr + sizeof(header.marker),
                               (uint8_t *)&header.size_bytes, sizeof(header.size_bytes));
  status_ok_or_return(ret);

  // Write persist blob
  ret = flash_write(persist->flash_addr + sizeof(header), (uint8_t *)persist->blob, persist->blob_size);
  status_ok_or_return(ret);

  persist->prev_flash_addr = persist->flash_addr;
  persist->flash_addr += sizeof(header) + persist->blob_size;

  return STATUS_CODE_OK;
}

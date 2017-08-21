# Specify toolchain
COMPILER := gcc
VALID_COMPILERS := gcc clang
override COMPILER := $(filter $(VALID_COMPILERS),$(COMPILER))
ifeq (,$(COMPILER))
  $(error Invalid compiler. Expected: $(VALID_COMPILERS))
endif

CC := $(COMPILER)
LD := $(COMPILER)
OBJCPY := objcopy
OBJDUMP := objdump
SIZE := size
AR := ar
GDB := gdb

# Set the library to include if using this platform
PLATFORM_LIB := x86
PLATFORM_EXT :=

# Architecture dependent variables
ARCH_CFLAGS :=

# Linker and startup script locations
LDSCRIPT := $(PLATFORM_DIR)/ldscripts

# Build flags for the device
CDEFINES :=

ifeq (gcc, $(COMPILER))
  CSFLAGS := -g -Os
else ifeq (asan, $(COPTIONS))
  CSFLAGS := -O1 -g -fsanitize=address -fno-omit-frame-pointer
else ifeq (tsan, $(COPTIONS))
  CSFLAGS := -fsanitize=thread -O1 -g
else
  CSFLAGS := -O1 -g
endif

CFLAGS := $(CSFLAGS) -Wall -Wextra -Werror -std=gnu11 -Wno-discarded-qualifiers \
          -Wno-unused-variable -Wno-unused-parameter -Wsign-conversion -Wpointer-arith \
          -ffunction-sections -fdata-sections \
          $(ARCH_CFLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS := -lrt

# Platform targets
.PHONY: run gdb

run: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
	@$<

gdb: $(GDB_TARGET)
	@$(GDB) $<

define session_wrapper
$1
endef

# Defines command to run for unit testing
define test_run
echo "\nRunning $(notdir $1)" && ./$1
endef

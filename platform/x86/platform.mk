# Specify toolchain
CC := gcc
LD := gcc
OBJCPY := objcopy
OBJDUMP := objdump
SIZE := size
AR := ar
GDB := gdb

# Set the library to include if using this platform
PLATFORM_LIB :=
PLATFORM_EXT :=

# Architecture dependent variables
ARCH_CLAGS :=

# Linker and startup script locations
# TODO: Move platform startup to somewhere else?
#PLATFORM_STARTUP := $(PLATFORM_DIR)/startup_stm32f0xx.s
LDSCRIPT := $(PLATFORM_DIR)/ldscripts

# Build flags for the device
CDEFINES :=
CFLAGS := -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
          -ffunction-sections -fdata-sections \
          -Wl,-Map=$(BIN_DIR)/$(PROJECT).map \
          $(ARCH_CLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS :=

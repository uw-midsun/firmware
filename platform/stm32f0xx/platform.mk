# Specify toolchain
CC := $(GCC_ARM_BASE)arm-none-eabi-gcc
LD := $(GCC_ARM_BASE)arm-none-eabi-gcc
OBJCPY := $(GCC_ARM_BASE)arm-none-eabi-objcopy
OBJDUMP := $(GCC_ARM_BASE)arm-none-eabi-objdump
SIZE := $(GCC_ARM_BASE)arm-none-eabi-size
AR := $(GCC_ARM_BASE)arm-none-eabi-gcc-ar
GDB := $(GCC_ARM_BASE)arm-none-eabi-gdb
OPENOCD := openocd

# Set the library to include if using this platform
PLATFORM_LIB := stm32f0xx
PLATFORM_EXT := .elf

# Architecture dependent variables
ARCH_CFLAGS := -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

# Linker script location
LDSCRIPT_DIR := $(PLATFORM_DIR)/ldscripts

# Helper scripts location
SCRIPT_DIR := $(PLATFORM_DIR)/scripts

# Build flags for the device
CDEFINES := USE_STDPERIPH_DRIVER STM32F072 HSE_VALUE=32000000
CFLAGS := -Wall -Wextra -Werror -g3 -Os -std=c11 -Wno-discarded-qualifiers \
					-Wno-unused-variable -Wno-unused-parameter -Wsign-conversion -Wpointer-arith \
					-ffunction-sections -fdata-sections -fno-builtin -flto \
					$(ARCH_CFLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS := -L$(LDSCRIPT_DIR) -Tstm32f0.ld -fuse-linker-plugin \
           --specs=nosys.specs --specs=nano.specs

# Device openocd config file
# Use PROBE=stlink-v2 for discovery boards
PROBE=cmsis-dap
OPENOCD_SCRIPT_DIR := /usr/share/openocd/scripts/
OPENOCD_CFG := -s $(OPENOCD_SCRIPT_DIR) \
               -f interface/$(PROBE).cfg -f target/stm32f0x.cfg \
               -f $(SCRIPT_DIR)/stm32f0-openocd.cfg

# Platform targets
.PHONY: program gdb target

program: $(GDB_TARGET:$(PLATFORM_EXT)=.bin)
	@$(OPENOCD) $(OPENOCD_CFG) -c "stm_flash $<" -c shutdown

gdb: $(GDB_TARGET)
	@pkill $(OPENOCD) || true
	@setsid $(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
	@$(GDB) $< -x "$(SCRIPT_DIR)/gdb_flash"
	@pkill $(OPENOCD)

define session_wrapper
setsid $(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
$1; pkill $(OPENOCD)
endef

# Defines command to run for unit testing
define test_run
clear && $(GDB) $1 -x "$(SCRIPT_DIR)/gdb_flash" -ex "b LoopForever" -ex "c" -ex "set confirm off" -ex "q"
endef

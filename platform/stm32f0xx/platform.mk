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
ARCH_CLAGS := -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

# Linker script location
LDSCRIPT := $(PLATFORM_DIR)/ldscripts

# Build flags for the device
CDEFINES := USE_STDPERIPH_DRIVER STM32F072
CFLAGS := -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
          -ffunction-sections -fdata-sections -fno-builtin -flto \
          --specs=nosys.specs --specs=nano.specs \
          $(ARCH_CLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS := $(CLFLAGS) -L$(LDSCRIPT) -Tstm32f0.ld -fuse-linker-plugin

# Device openocd config file
OPENOCD_SCRIPT_DIR := /usr/share/openocd/scripts/
OPENOCD_CFG := -s $(OPENOCD_SCRIPT_DIR) \
               -f interface/stlink-v2.cfg -f target/stm32f0x.cfg \
               -f $(PLATFORM_DIR)/stm32f0-openocd.cfg

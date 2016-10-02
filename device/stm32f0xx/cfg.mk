CC = arm-none-eabi-gcc
OBJCPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
AR = arm-none-eabi-ar

ARCH = -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb
LIB = -include $(LIB_DIR)/stm32f0xx_conf.h \
			-isystem $(LIB_DIR)/CMSIS/Include \
			-isystem $(LIB_DIR)/CMSIS/Device/ST/STM32F0xx/Include \
			-isystem $(LIB_DIR)/STM32F0xx_StdPeriph_Driver/inc \
			-isystem $(LIB_DIR)/STM32F0xx_StdPeriph_Driver/src

STARTUP = device/stm32f0xx/startup_stm32f0xx.s
LDSCRIPT = device/stm32f0xx/ldscripts

#lib = $(LIB_DIR)/libstm32f0.a

CFLAGS = -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
				 $(ARCH) -ffunction-sections -fdata-sections -Wl,--gc-sections 

OPENOCD_CFG = $(LIB_DIR)/stm32f0-openocd.cfg

include $(LIB_DIR)/lib.mk

project: $(foreach project,$(MAIN_PROJECT),$(BIN_DIR)/$(project).elf)

$(BIN_DIR)/%.elf: $(MAIN_FILE) $(STARTUP)
	@mkdir -p $(BIN_DIR)/
	@$(CC) $(CFLAGS) -Wl,-Map=$(basename $@).map $^ -o $@ \
		-L$(LIB_DIR) -lstm32f0 -L$(MSLIB_DIR) $(MSLIB_DIR)/mslib.a \
		-L$(LDSCRIPT) -Tstm32f0.ld \
		-isystem $(LIB_DIR) $(LIB) 	
	@$(OBJCPY) -O binary $@ $(basename $@).bin
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	$(SIZE) $@

# OPTIONAL:
# $(OBJCOPY) -o ihex $(basename $@).hex

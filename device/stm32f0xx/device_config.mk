CC = arm-none-eabi-gcc
OBJCPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
AR = arm-none-eabi-ar

ARCH = -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb
INC = -include $(STM32F0_DIR)/stm32f0xx_conf.h \
			-isystem $(STM32F0_DIR)/CMSIS/Include \
			-isystem $(STM32F0_DIR)/CMSIS/Device/ST/STM32F0xx/Include \
			-isystem $(STM32F0_DIR)/STM32F0xx_StdPeriph_Driver/inc \
			-isystem $(STM32F0_DIR)/STM32F0xx_StdPeriph_Driver/src \
			-I $(STM32F0_DIR)

STARTUP = device/stm32f0xx/startup_stm32f0xx.s
LDSCRIPT = device/stm32f0xx/ldscripts

CFLAGS = -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
				 $(ARCH) -ffunction-sections -fdata-sections -Wl,--gc-sections \
				 -Wl,-Map=$(BIN_DIR)/$(PROJECT_NAME).map 

LINKER = $(INC) -L$(LDSCRIPT) -Tstm32f0.ld 

OPENOCD_CFG = $(STM32F0_DIR)/stm32f0-openocd.cfg

#project: $(PROJECT_NAME).elf

#$(BIN_DIR)/%.elf: $(MAIN_FILE) $(STARTUP)
#	@mkdir -p $(BIN_DIR)/
#	@$(CC) $(CFLAGS) -Wl,-Map=$(basename $@).map $^ -o $@ \
		-L$(STM32F0_DIR) -lstm32f0 -L$(MSSTM32F0_DIR) $(MSSTM32F0_DIR)/mslib.a \
		-L$(LDSCRIPT) -Tstm32f0.ld \
		-isystem $(STM32F0_DIR) $(LIB) 	
#	@$(OBJCPY) -O binary $@ $(basename $@).bin
#	@$(OBJDUMP) -St $@ >$(basename $@).lst
#	$(SIZE) $@

# OPTIONAL:
# $(OBJCOPY) -o ihex $(basename $@).hex

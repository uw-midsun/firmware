CC = arm-none-eabi-gcc
OBJCPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
AR = arm-none-eabi-ar

ARCH = -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb
LIB = -include $(lib_dir)/stm32f0xx_conf.h \
			-isystem $(lib_dir)/CMSIS/Include \
			-isystem $(lib_dir)/CMSIS/Device/ST/STM32F0xx/Include \
			-isystem $(lib_dir)/STM32F0xx_StdPeriph_Driver/inc \
			-isystem $(lib_dir)/STM32F0xx_StdPeriph_Driver/src

startup = device/stm32f0xx/startup_stm32f0xx.s
ldscript = device/stm32f0xx/ldscripts

lib = $(lib_dir)/libstm32f0.a

CFLAGS = -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
				 $(ARCH) -ffunction-sections -fdata-sections -Wl,--gc-sections 

openocdfile = $(lib_dir)/stm32f0-openocd.cfg

include $(lib_dir)/stm32f0lib.mk

project: $(foreach project,$(mainprojectname),$(bindir)/$(project).elf)

$(bindir)/%.elf: $(mainprojectfile) $(startup)
	@mkdir -p $(bindir)/
	@$(CC) $(CFLAGS) -Wl,-Map=$(basename $@).map $^ -o $@ \
		-L$(lib_dir) -lstm32f0 -L$(mslibdir) $(mslibdir)/mslib.a \
		-L$(ldscript) -Tstm32f0.ld \
		-isystem $(lib_dir) $(LIB) 	
	@$(OBJCPY) -O binary $@ $(basename $@).bin
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	$(SIZE) $@

# OPTIONAL:
# $(OBJCOPY) -o ihex $(basename $@).hex

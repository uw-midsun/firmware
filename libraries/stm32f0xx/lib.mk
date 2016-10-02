prefixlib = libraries/stm32f0xx

libsrcdir = $(prefixlib)/STM32F0xx_StdPeriph_Driver/src
libincdir = $(prefixlib)/STM32F0xx_StdPeriph_Driver/inc
libobjdir = $(prefixlib)/obj

libsrc = $(notdir $(wildcard $(libsrcdir)/*.c)) 
libobj = $(addprefix $(libobjdir)/,$(libsrc:.c=.o))

libCFLAGS = -g -O2 -Wall $(ARCH) $(LIB) -ffreestanding -nostdlib

$(prefixlib)/libstm32f0.a: $(libobj)
	@$(AR) -r $@ $(libobj)

$(libobjdir)/%.o: $(libsrcdir)/%.c 
	@mkdir -p $(libobjdir)
	@$(CC) -w -c -o $@ $< $(libCFLAGS)

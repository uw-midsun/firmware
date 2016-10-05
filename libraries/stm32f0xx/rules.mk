# Specifies the SRC, INC and OBJ directories for the library
stm32f0xx_SRC_DIR = $(stm32f0xx_DIR)/STM32F0xx_StdPeriph_Driver/src
stm32f0xx_INC_DIR = $(stm32f0xx_DIR)/STM32F0xx_StdPeriph_Driver/inc
stm32f0xx_OBJ_DIR = $(stm32f0xx_DIR)/obj

# Specifies the SRC and OBJ files DO NOT TOUCH 
stm32f0xx_SRC = $(notdir $(wildcard $(stm32f0xx_SRC_DIR)/*.c)) 
stm32f0xx_OBJ = $(addprefix $(stm32f0xx_OBJ_DIR)/,$(stm32f0xx_SRC:.c=.o))

# Specifies library specific build flags
stm32f0xx_CFLAGS = -g -O2 -Wall $(ARCH) $(INC) -ffreestanding -nostdlib

# Specifies library build rules
$(OBJ_CACHE)/libstm32f0xx.a: $(stm32f0xx_OBJ)
	@mkdir -p $(OBJ_CACHE)
	@$(AR) -r $@ $(stm32f0xx_OBJ)

$(stm32f0xx_OBJ_DIR)/%.o: $(stm32f0xx_SRC_DIR)/%.c 
	@mkdir -p $(stm32f0xx_OBJ_DIR)
	@$(CC) -w -c -o $@ $< $(stm32f0xx_CFLAGS)

STM32F0_DIR = libraries/stm32f0xx

STM32F0_SRC_DIR = $(STM32F0_DIR)/STM32F0xx_StdPeriph_Driver/src
STM32F0_INC_DIR = $(STM32F0_DIR)/STM32F0xx_StdPeriph_Driver/inc
STM32F0_OBJ_DIR = $(STM32F0_DIR)/obj

STM32F0_SRC = $(notdir $(wildcard $(STM32F0_SRC_DIR)/*.c)) 
STM32F0_OBJ = $(addprefix $(STM32F0_OBJ_DIR)/,$(STM32F0_SRC:.c=.o))

STM32F0_CFLAGS = -g -O2 -Wall $(ARCH) $(INC) -ffreestanding -nostdlib

$(OBJ_CACHE)/libstm32f0.a: $(STM32F0_OBJ)
	@mkdir -p $(OBJ_CACHE)
	@$(AR) -r $@ $(STM32F0_OBJ)

$(STM32F0_OBJ_DIR)/%.o: $(STM32F0_SRC_DIR)/%.c 
	@mkdir -p $(STM32F0_OBJ_DIR)
	@$(CC) -w -c -o $@ $< $(STM32F0_CFLAGS)

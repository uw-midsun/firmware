# Specifies the path from the makefile to this file
$(LIB_DIR)_DIR = libraries/stm32f0xx

# Specifies the SRC, INC and OBJ directories for the library
$(LIB_DIR)_SRC_DIR = $($(LIB_DIR)_DIR)/STM32F0xx_StdPeriph_Driver/src
$(LIB_DIR)_INC_DIR = $($(LIB_DIR)_DIR)/STM32F0xx_StdPeriph_Driver/inc
$(LIB_DIR)_OBJ_DIR = $($(LIB_DIR)_DIR)/obj

# Specifies the SRC and OBJ files DO NOT TOUCH 
$(LIB_DIR)_SRC = $(notdir $(wildcard $($(LIB_DIR)_SRC_DIR)/*.c)) 
$(LIB_DIR)_OBJ = $(addprefix $($(LIB_DIR)_OBJ_DIR)/,$($(LIB_DIR)_SRC:.c=.o))

# Specifies library specific build flags
$(LIB_DIR)_CFLAGS = -g -O2 -Wall $(ARCH) $(INC) -ffreestanding -nostdlib

# Specifies library build rules
$(warning $(LIB_DIR))$(OBJ_CACHE)/lib$(LIB_DIR).a: $($(LIB_DIR)_OBJ)
	@mkdir -p $(OBJ_CACHE)
	@$(AR) -r $@ $($(LIB_DIR)_OBJ)

$($(LIB_DIR)_OBJ_DIR)/%.o: $($(LIB_DIR)_SRC_DIR)/%.c 
	@mkdir -p $($(LIB_DIR)_OBJ_DIR)
	@$(CC) -w -c -o $@ $< $($(LIB_DIR)_CFLAGS)

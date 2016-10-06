# Specifies the path from the makefile to this file
$(LIB)_DIR := $(LIB_DIR)/$(LIB)

# Specifies the SRC, INC and OBJ directories for the library
$(LIB)_SRC_DIR := $($(LIB)_DIR)/STM32F0xx_StdPeriph_Driver/src
$(LIB)_INC_DIR := $($(LIB)_DIR)/STM32F0xx_StdPeriph_Driver/inc
$(LIB)_OBJ_DIR := $(OBJ_CACHE)/$(LIB)
DIRS += $($(LIB)_OBJ_DIR)

# Specifies the SRC and OBJ files DO NOT TOUCH
$(LIB)_SRC := $(notdir $(wildcard $($(LIB)_SRC_DIR)/*.c))
$(LIB)_OBJ := $(addprefix $($(LIB)_OBJ_DIR)/,$($(LIB)_SRC:.c=.o))

# Specifies library specific build flags
$(LIB)_CFLAGS := -g -O2 -Wall $(ARCH) $(INC) -ffreestanding -nostdlib

# Specifies library build rules
$(STATIC_LIB_DIR)/lib$(LIB).a: $($(LIB)_OBJ) | $(STATIC_LIB_DIR)
	@echo "Linking $@"
	@$(AR) -r $@ $^

$($(LIB)_OBJ_DIR)/%.o: $($(LIB)_SRC_DIR)/%.c | $(LIB) $($(LIB)_OBJ_DIR)
	@echo "$(notdir $<) -> $(notdir $@)"
	@$(CC) -w -c -o $@ $< $($(firstword $|)_CFLAGS)

$(LIB):
	@echo super hack $@

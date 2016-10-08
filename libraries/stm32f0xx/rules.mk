# Specifies the path from the makefile to this file
$(LIB)_DIR := $(LIB_DIR)/$(LIB)

# Specifies the SRC, INC and OBJ directories for the library
$(LIB)_SRC_DIR := $($(LIB)_DIR)/STM32F0xx_StdPeriph_Driver/src
$(LIB)_INC_DIR := $($(LIB)_DIR)/STM32F0xx_StdPeriph_Driver/inc
$(LIB)_OBJ_DIR := $(OBJ_CACHE)/$(LIB)
$(LIB)_DEPS :=

# Specifies the SRC and OBJ files DO NOT TOUCH
$(LIB)_SRC := $(foreach dir,$($(LIB)_SRC_DIR),$(wildcard $(dir)/*.c))
$(LIB)_OBJ := $(patsubst $($(LIB)_SRC_DIR)/%.c,$($(LIB)_OBJ_DIR)/%.o,$($(LIB)_SRC))

-include $($(LIB)_OBJ:.o=.d)

# Specifies library specific build flags
$(LIB)_CFLAGS := -g -O2 -Wall $(ARCH) $(INC) -ffreestanding -nostdlib

# Specifies library build rules
$(STATIC_LIB_DIR)/lib$(LIB).a: $($(LIB)_OBJ) $(call dep_to_lib,$($(LIB)_DEPS)) | $(STATIC_LIB_DIR)
	@echo "Linking $@"
	@$(AR) -r $@ $^

$($(LIB)_OBJ_DIR)/%.o: $($(LIB)_SRC_DIR)/%.c | $(LIB) $(dir $($(LIB)_OBJ))
	@echo "$(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS)

.PHONY: $(LIB)
$(LIB):
	@echo super hack $@

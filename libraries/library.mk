# Default library makefile - expects LIB to be declared as the project name.
# Defines a few variables, includes the appropriate rules.mk,
# and sets the static library and object file targets.

# Defines default variables and targets
$(LIB)_DIR := $(LIB_DIR)/$(LIB)

$(LIB)_SRC_ROOT := $($(LIB)_DIR)/src
$(LIB)_INC_ROOT := $($(LIB)_DIR)/inc
$(LIB)_OBJ_ROOT := $(OBJ_CACHE)/$(LIB)

# Include library variables - we expect to have $(LIB)_SRC, $(LIB)_DEPS, and $(LIB)_CFLAGS
include $($(LIB)_DIR)/rules.mk

# Define objects and include generated dependencies
$(LIB)_OBJ := $(patsubst $($(LIB)_SRC_ROOT)/%.c,$($(LIB)_OBJ_ROOT)/%.o,$($(LIB)_SRC))
-include $($(LIB)_OBJ:.o=.d) #:

# Static library link target
$(STATIC_LIB_DIR)/lib$(LIB).a: $($(LIB)_OBJ) $(call dep_to_lib,$($(LIB)_DEPS)) | $(STATIC_LIB_DIR)
	@echo "Linking $@"
	@$(AR) -r $@ $^

# Object target - use first order-only dependency to expand the library name for subshells
$($(LIB)_OBJ_ROOT)/%.o: $($(LIB)_SRC_ROOT)/%.c | $(LIB) $(dir $($(LIB)_OBJ))
	@echo "$(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS)

.PHONY: $(LIB)
$(LIB):
	@echo "Processing $@"
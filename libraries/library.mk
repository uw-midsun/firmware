# Default library makefile - expects LIB to be declared as the project name.
# Defines a few variables, includes the appropriate rules.mk,
# and sets the static library and object file targets.

# Defines default variables and targets
$(LIB)_DIR := $(LIB_DIR)/$(LIB)

$(LIB)_SRC_ROOT := $($(LIB)_DIR)/src
$(LIB)_OBJ_ROOT := $(OBJ_CACHE)/$(LIB)
$(LIB)_INC_DIRS := $($(LIB)_DIR)/inc

$(LIB)_SRC := $(wildcard $($(LIB)_SRC_ROOT)/*.c) \
              $(wildcard $($(LIB)_SRC_ROOT)/*.s)
$(LIB)_INC := $(wildcard $($(LIB)_INC_DIRS)/*.h)

$(LIB)_CFLAGS := $(CFLAGS)

# Include library variables - we expect to have $(LIB)_SRC, $(LIB)_INC, $(LIB)_DEPS, and $(LIB)_CFLAGS
include $($(LIB)_DIR)/rules.mk

# Define objects and include generated dependencies
# Note that without some very complex rules, we can only support one root source directory.
$(LIB)_OBJ := $($(LIB)_SRC:$($(LIB)_SRC_ROOT)/%.c=$($(LIB)_OBJ_ROOT)/%.o)
$(LIB)_OBJ := $($(LIB)_OBJ:$($(LIB)_SRC_ROOT)/%.s=$($(LIB)_OBJ_ROOT)/%.o)
-include $($(LIB)_OBJ:.o=.d) #:

# Static library link target
$(STATIC_LIB_DIR)/lib$(LIB).a: $($(LIB)_OBJ) $(call dep_to_lib,$($(LIB)_DEPS)) | $(STATIC_LIB_DIR)
	@echo "Linking $@"
	@$(AR) -r $@ $^

# Object target - use first order-only dependency to expand the library name for subshells
$($(LIB)_OBJ_ROOT)/%.o: $($(LIB)_SRC_ROOT)/%.c | $(LIB) $(dir $($(LIB)_OBJ))
	@echo "$(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

$($(LIB)_OBJ_ROOT)/%.o: $($(LIB)_SRC_ROOT)/%.s | $(LIB) $(dir $($(LIB)_OBJ))
	@echo "$(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

.PHONY: $(LIB)
$(LIB):
	$(eval APP_DEPS += $($(@)_DEPS))
	@echo "Processing $@"
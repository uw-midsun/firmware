# Specifies the SRC, INC and OBJ directories for the library
ms-lib_SRC_DIR := $(ms-lib_DIR)/src/$(DEVICE_FAMILY)
ms-lib_INC_DIR := $(ms-lib_DIR)/inc
ms-lib_OBJ_DIR := $(ms-lib_DIR)/obj

# Specifies the SRC and OBJ files DO NOT TOUCH
ms-lib_SRC := $(notdir $(wildcard $(ms-lib_SRC_DIR)/*.c)) 
ms-lib_OBJ := $(addprefix $(ms-lib_OBJ_DIR)/,$(ms-lib_SRC:.c=.o))

# Specifies library specific build flags
ms-lib_CFLAGS := -g -O2 -Wall -Werror -pedantic -Wno-unused-variable \
					 $(ARCH) $(DEVICE_LIB) -I$(ms-lib_DIR) -ffreestanding -nostdlib

# Specifies library build rules
$(OBJ_CACHE)/libms-lib.a: $(ms-lib_OBJ)
	@mkdir -p $(OBJ_CACHE)
	@$(AR) -r $@ $(ms-lib_OBJ)

$(ms-lib_OBJ_DIR)/%.o: $(ms-lib_SRC_DIR)/%.c 
	@mkdir -p $(ms-lib_OBJ_DIR)
	@$(CC) -w -c -o $@ $< $(ms-lib_CFLAGS)

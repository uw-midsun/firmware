MS_DIR := libraries/ms-lib

MS_SRC_DIR := $(MS_DIR)/src/$(DEVICE_FAMILY)
MS_INC_DIR := $(MS_DIR)/inc
MS_OBJ_DIR := $(MS_DIR)/obj

MS_SRC := $(notdir $(wildcard $(MS_SRC_DIR)/*.c)) 
MS_OBJ := $(addprefix $(MS_OBJ_DIR)/,$(MS_SRC:.c=.o))

MS_CFLAGS := -g -O2 -Wall -Werror -pedantic -Wno-unused-variable \
					 $(ARCH) $(DEVICE_LIB) -I$(MS_DIR) -ffreestanding -nostdlib

$(OBJ_CACHE)/libmslib.a: $(MS_OBJ)
	@mkdir -p $(OBJ_CACHE)
	@$(AR) -r $@ $(MS_OBJ)

$(MS_OBJ_DIR)/%.o: $(MS_SRC_DIR)/%.c 
	@mkdir -p $(MS_OBJ_DIR)
	@$(CC) -w -c -o $@ $< $(MS_CFLAGS)

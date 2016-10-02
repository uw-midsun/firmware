MSLIB := libraries/ms-lib

MSSRC_DIR := $(MSLIB)/src/$(DEVICE_FAMILY)
MSINC_DIR := $(MSLIB)/inc
MSOBJ_DIR := $(MSLIB)/obj

MSSRC := $(notdir $(wildcard $(MSSRC_DIR)/*.c)) 
MSOBJ := $(addprefix $(MSOBJ_DIR)/,$(MSSRC:.c=.o))

MSCFLAGS := -g -O2 -Wall -Werror -pedantic -Wno-unused-variable \
					 $(ARCH) $(LIB) -I$(MSLIB) \
					 -ffreestanding -nostdlib

$(MSLIB)/mslib.a: $(MSOBJ)
	@$(AR) -r $@ $(MSOBJ)

$(MSOBJ_DIR)/%.o: $(MSSRC_DIR)/%.c 
	@mkdir -p $(MSOBJ_DIR)
	@$(CC) -w -c -o $@ $< $(MSCFLAGS)

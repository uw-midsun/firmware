mslib = libraries/ms-lib

mssrcdir = $(mslib)/src/$(devicefamily)
msincdir = $(mslib)/inc
msobjdir = $(mslib)/obj

mssrc = $(notdir $(wildcard $(mssrcdir)/*.c)) 
msobj = $(addprefix $(msobjdir)/,$(mssrc:.c=.o))

msCFLAGS = -g -O2 -Wall -Werror -pedantic -Wno-unused-variable \
					 $(ARCH) $(LIB) -I$(mslib) \
					 -ffreestanding -nostdlib

$(mslib)/mslib.a: $(msobj)
	@$(AR) -r $@ $(msobj)

$(msobjdir)/%.o: $(mssrcdir)/%.c 
	@mkdir -p $(msobjdir)
	@$(CC) -w -c -o $@ $< $(msCFLAGS)

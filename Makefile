#
# Makefile for CS452 Kernel
#
XDIR = /u/cs452/public/xdev
XBINDIR = $(XDIR)/bin
XLIBDIR1 = $(XDIR)/arm-none-eabi/lib
XLIBDIR2 = $(XDIR)/lib/gcc/arm-none-eabi/9.2.0
CC = $(XBINDIR)/arm-none-eabi-gcc
LD = $(XBINDIR)/arm-none-eabi-ld
WHOAMI = $(shell whoami)
TARGET = main.elf
BINDIR = bin
DOCDIR = doc
OBJDIR = obj
SRCDIR = src

CSRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)
COBJS = $(CSRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
ASMSRCS = $(wildcard $(SRCDIR)/*.S $(SRCDIR)/*/*.S)
ASMOBJS = $(ASMSRCS:$(SRCDIR)/%.S=$(OBJDIR)/%.o)

# -c: compile and assemble, do not link
# -std=gnu99: use C99 standard with asm support
# -g: include debug information for gdb
# -O2: enable level 2 optimization for gcc
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -Wextra: report more detail warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -c -std=gnu99 -g -O2 -fPIC -Wall -Wextra -Wconversion -mcpu=arm920t -msoft-float -I include

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# --gc-sections: remove unused sections
# -T: use linker script
LDFLAGS = -static -e kernel_entry -nmagic --gc-sections -T linker.ld -L $(XLIBDIR1) -L $(XLIBDIR2)

.PHONY: all
all: $(BINDIR)/$(TARGET)

$(ASMOBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(COBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(BINDIR)/$(TARGET): $(COBJS) $(ASMOBJS)
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) -o $@ $(COBJS) $(ASMOBJS) -lc -lgcc

.PHONY: install
install:
	cp $(BINDIR)/$(TARGET) /u/cs452/tftp/ARM/$(WHOAMI)

.PHONY: doc
doc:
	doxygen $(DOCDIR)/Doxyfile

.PHONY: clean
clean:
	rm -rf $(BINDIR) $(OBJDIR) $(DOCDIR)/doxygen

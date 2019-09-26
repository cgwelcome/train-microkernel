#
# Makefile for CS452 Kernel
#
XDIR=/u/cs452/public/xdev
XBINDIR=$(XDIR)/bin
XLIBDIR1=$(XDIR)/arm-none-eabi/lib
XLIBDIR2=$(XDIR)/lib/gcc/arm-none-eabi/9.2.0
CC = $(XBINDIR)/arm-none-eabi-gcc
AR = $(XBINDIR)/arm-none-eabi-ar
AS = $(XBINDIR)/arm-none-eabi-as
LD = $(XBINDIR)/arm-none-eabi-ld

# List of object files
OBJECTS = $(patsubst %.c, %.o, $(wildcard *c */*.c))

# List of libraries
LIBDIR = utils
LIBNAMES = $(notdir $(basename $(wildcard $(LIBDIR)/*.c)))
LIBFILES = $(foreach libname,$(LIBNAMES),$(LIBDIR)/lib$(libname).a)
LDLIBS = $(foreach libname,$(LIBNAMES),-l$(notdir $(basename $(libname)))) -lc -lgcc

# -std=gnu99: use C99 standard with asm support
# -g: include debug information for gdb
# -O0: disable any optimization for gcc
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -std=gnu99 -g -O2 -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I../include

# c: create archive, if necessary
# r: insert with replacement
# s: add index to archive
ARFLAGS = crs

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -static -e kernel_entry -nmagic -T linker.ld -L $(LIBDIR) -L $(XLIBDIR1) -L $(XLIBDIR2)

.PHONY: all
all: main.elf

%.s: %.c
	$(CC) -S $(CFLAGS) $^

$(LIBDIR)/lib%.a: $(LIBDIR)/%.o
	$(AR) $(ARFLAGS) $@ $^

main.elf: $(OBJECTS) $(LIBFILES)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS) $(LIBFILES)

.PHONY: clean
clean:
	-rm -f kern/*.o user/*.o utils/*.a utils/*.o *.o *.elf

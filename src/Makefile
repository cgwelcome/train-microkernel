#
# Makefile for CS452 Kernel
#
XDIR=/u/cs452/public/xdev
XBINDIR=$(XDIR)/bin
XLIBDIR1=$(XDIR)/arm-none-eabi/lib
XLIBDIR2=$(XDIR)/lib/gcc/arm-none-eabi/9.2.0
CC = $(XBINDIR)/arm-none-eabi-gcc
LD = $(XBINDIR)/arm-none-eabi-ld

# List of object files
OBJECTS = $(patsubst %.c, %.o, $(wildcard *c */*.c)) $(patsubst %.S, %.o, $(wildcard *S */*.S))

# -std=gnu99: use C99 standard with asm support
# -g: include debug information for gdb
# -O2: enable level 2 optimization for gcc
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -Wextra: report more detail warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -std=gnu99 -g -O2 -fPIC -Wall -Wextra -Wconversion -mcpu=arm920t -msoft-float -I../include

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# --gc-sections: remove unused sections
# -T: use linker script
LDFLAGS = -static -e kernel_entry -nmagic --gc-sections -T linker.ld -L $(XLIBDIR1) -L $(XLIBDIR2)

.PHONY: all
all: main.elf

%.s: %.c
	$(CC) -S $(CFLAGS) $^

%.o: %.S
	$(CC) -c $(CFLAGS) -o $@ $^

main.elf: $(OBJECTS) $(LIBFILES)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) -lc -lgcc

.PHONY: clean
clean:
	-rm -f */*.o *.o *.elf

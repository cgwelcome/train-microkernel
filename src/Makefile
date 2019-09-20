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
OBJECTS = kern/io.o kern/tasks.o user/io.o user/tasks.o application.o kernel.o

# -std=gnu99: use C99 standard with asm support
# -g: include debug information for gdb
# -O0: disable any optimization for gcc
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -std=gnu99 -g -O0 -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I../include

# c: create archive, if necessary
# r: insert with replacement
# s: add index to archive
ARFLAGS = crs

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -static -e kernel_entry -nmagic -T linker.ld -L . -L ../lib -L $(XLIBDIR1) -L $(XLIBDIR2)

all: main.elf

%.s: %.c
	$(CC) -S $(CFLAGS) $^

libbwio.a: bwio.o
	$(AR) $(ARFLAGS) $@ bwio.o

main.elf: libbwio.a $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) -lbwio -lc -lgcc

clean:
	-rm -f kern/*.o user/*.o *.s *.o *.a *.elf

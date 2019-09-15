#
# Makefile for Assignment 0
#
XDIR=/u/cs452/public/gnuarm-4.0.2
XBINDIR=$(XDIR)/bin
XLIBDIR1=$(XDIR)/arm-elf/lib
XLIBDIR2=$(XDIR)/lib/gcc/arm-elf/4.0.2
CC = $(XBINDIR)/arm-elf-gcc
AR = $(XBINDIR)/arm-elf-ar
AS = $(XBINDIR)/arm-elf-as
LD = $(XBINDIR)/arm-elf-ld

# -g: include debug information for gdb
# -O0: disable any optimization for gcc
# -S: only compile and emit assembly
# -fPIC: emit position-independent code
# -Wall: report all warnings
# -mcpu=arm920t: generate code for the 920t architecture
# -msoft-float: no FP co-processor
CFLAGS = -g -O0 -S -fPIC -Wall -mcpu=arm920t -msoft-float -I. -I../include

# c: create archive, if necessary
# r: insert with replacement
# s: add index to archive
ARFLAGS = crs

# -static: force static linking
# -e: set entry point
# -nmagic: no page alignment
# -T: use linker script
LDFLAGS = -static -e main -nmagic -T linker.ld -L ../lib -L $(XLIBDIR2)

all: main.elf

main.s: main.c
	$(CC) -S $(CFLAGS) main.c

main.o: main.s
	$(AS) $(ASFLAGS) -o main.o main.s

main.elf: main.o
	$(LD) $(LDFLAGS) -o $@ main.o -lgcc

clean:
	-rm -f *.s *.o *.a *.elf

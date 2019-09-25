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

# List of object files
LIBS = $(addprefix lib, $(patsubst %.c, %.a, $(wildcard *.c)))
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

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

all: $(LIBS)

%.s: %.c
	$(CC) -S $(CFLAGS) $^

lib%.a: %.o
	$(AR) $(ARFLAGS) $@ $^

clean:
	-rm -f *.a *.o

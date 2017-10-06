#
# Broadcom Makefile for libusb
#
# The main libusb distribution contains an evil, hopeless morass of long
# and complicated Makefiles, bash scripts, library description files,
# automatically translated documentation, and on and on, totaling
# almost 132,000 lines of cryptic garbage.  The documentation is nearly
# content-free.  I think it's all some kind of extended April Fools joke.
#
# This simplified Makefile compiles the necessary 2,500 lines of code.
#

CC = gcc
AR = ar
IFLAGS = -I.
CFLAGS = -g -MD $(IFLAGS)
RM = rm

LIBUSB = libusb.a

all: $(LIBUSB)

sinclude *.d

testecho: testecho.o $(LIBUSB)
	$(CC) -o testecho testecho.o $(LIBUSB) -lpthread

OBJS = usb.o descriptors.o error.o linux.o

$(LIBUSB): $(OBJS)
	$(AR) rv $(LIBUSB) $(OBJS)

clean:
	$(RM) -rf $(LIBUSB) *.o *.d

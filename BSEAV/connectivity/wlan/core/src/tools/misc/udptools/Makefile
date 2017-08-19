# Makefile for udptools

LIBS	=
CFLAGS	= -O -Wall -Werror
#CC	= arm-linux-gcc		# uncomment to compile for PXA

all:	udpsend udprecv

#
# Internet datagram version (UDP protocol).
#

udpsend.o udprecv.o: udptools.h

udprecv: udprecv.o
	$(CC) $(CFLAGS) -o $@ udprecv.o $(MYLIB) $(LIBS)

udpsend: udpsend.o
	$(CC) $(CFLAGS) -o $@ udpsend.o $(MYLIB) $(LIBS)

clean:
	$(RM) *.o *.exe udpsend udprecv

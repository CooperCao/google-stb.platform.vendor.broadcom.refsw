#
# $Id: Makefile 403251 2013-05-20 05:15:55Z $
#

ifneq ($(STBANDROID),1)
CROSS =
CC = $(CROSS)gcc
AR = $(CROSS)ar
STRIP = $(CROSS)strip
endif

ALL_OBJS = igsu.o igsu_linux.o

CCFLAGS = -Os -I../../../include ${WLAN_StdIncPathA} -I../../../common/include -I../../shared -I../../../include/emf/igs
ifneq ($(STBANDROID),1)
CCFLAGS	+= -Wall -Werror
endif

ifneq ($(wildcard ./prebuilt),)
all:
	-cp prebuilt/igs igs
else
all: igs
endif

igs: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(CCFLAGS) $(LDFLAGS) -o $@ $(ALL_OBJS) $(LDLIBS)
	$(STRIP) $@

$(ALL_OBJS): %.o: %.c
	@echo " "
	@echo "Making $<"
	$(CC) $(CFLAGS) $(CCFLAGS) -c $<

install: all
	install -d $(INSTALLDIR)/igs/usr/sbin
	install -m 755 igs $(INSTALLDIR)/igs/usr/sbin

.PHONY:	clean
clean:
	-rm -f *.o *~ core igs

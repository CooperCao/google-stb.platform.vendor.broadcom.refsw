#
# $Id: Makefile 403251 2013-05-20 05:15:55Z $
#

ifneq ($(STBANDROID),1)
CROSS =
CC = $(CROSS)gcc
AR = $(CROSS)ar
endif

ALL_OBJS = emfu.o emfu_linux.o

CCFLAGS = -Os -I../../../include ${WLAN_StdIncPathA} -I../../../common/include -I../../shared -I../../../include/emf/emf
ifneq ($(STBANDROID),1)
CCFLAGS	+= -Wall -Werror
endif

ifneq ($(wildcard ./prebuilt),)
all:
	-cp prebuilt/emf emf
else
all: emf
endif

emf: $(ALL_OBJS)
	$(CC) $(CFLAGS) $(CCFLAGS) $(LDFLAGS) -o $@ $(ALL_OBJS) $(LDLIBS)
	$(STRIP) $@

$(ALL_OBJS): %.o: %.c
	@echo " "
	@echo "Making $<"
	$(CC) $(CFLAGS) $(CCFLAGS) -c $<

install: all
	install -d $(INSTALLDIR)/emf/usr/sbin
	install -m 755 emf $(INSTALLDIR)/emf/usr/sbin

.PHONY:	clean
clean:
	-rm -f *.o *~ core emf

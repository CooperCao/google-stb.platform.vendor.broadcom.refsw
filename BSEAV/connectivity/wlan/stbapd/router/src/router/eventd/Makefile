#
# Linux eventd Makefile
#
# $Copyright Broadcom Corporation$
#
# $Id:  $
#

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -Wall -Werror
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += $(EXTRA_LDFLAGS)

#ifdef BCMINTERNAL
ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
#endif

vpath %.c $(SRCBASE)/shared/

all: eventd

OBJS = eventd_main.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install eventd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/eventd

clean:
	rm -f *.o *.a *.so eventd

${OBJS}: eventd.h

eventd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)


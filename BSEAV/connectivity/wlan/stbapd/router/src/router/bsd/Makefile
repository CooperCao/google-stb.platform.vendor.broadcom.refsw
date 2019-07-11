#
# Linux bsd Makefile
#
# $Copyright Broadcom Corporation$
#
# $Id: Makefile 401201 2013-05-09 03:06:42Z kenlo $
#

include $(SRCBASE)/router/.config
include $(SRCBASE)/router/.config.plt

PREBUILTDIR = prebuilt

CFLAGS	+= -I. -I$(TOP)/shared -I$(TOP)/bshared/include -I$(SRCBASE)/include -I$(SRCBASE)/common/include -I$(SRCBASE)/router/appeventd
ifneq ($(STBANDROID),1)
CFLAGS	+= -Wall -Werror
endif
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifneq ($(STBANDROID),1)
LDFLAGS += -lpthread
else
CFLAGS  += -fPIC
endif
LDFLAGS += -L$(TOP)/bshared/src -L$(INSTALLDIR)/bshared/usr/lib -lbshared
ifneq ($(STBANDROID),1)
LDFLAGS += $(EXTRA_LDFLAGS)
endif

#ifdef BCMINTERNAL
ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
#endif

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs)

all: bsd

OBJS := bsd_main.o bsd.o bsd_engine.o  bsd_util.o bsd_comm.o bcmwifi_channels.o bsd_default.o bsd_appevent.o

ifneq (2_6,$(LINUX_VERSION))
ifeq ($(CONFIG_WBD),y)
OBJS += bsd_wbd.o
endif
endif

CFILES := $(patsubst %.o,%.c,$(OBJS))
SRC_EXIST := $(wildcard $(CFILES))

install: all
ifneq ($(wildcard bsd),)
	install -d $(INSTALLDIR)/usr/sbin/
	install bsd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/bsd
	install -d $(PREBUILTDIR)
	install bsd $(PREBUILTDIR)
	$(STRIP) $(PREBUILTDIR)/bsd
endif

clean:
	rm -f *.o *.a *.so bsd
	rm -rf $(PREBUILTDIR)

ifneq (,$(SRC_EXIST))
${OBJS}: bsd.h

bsd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

else
bsd:
	@true
endif

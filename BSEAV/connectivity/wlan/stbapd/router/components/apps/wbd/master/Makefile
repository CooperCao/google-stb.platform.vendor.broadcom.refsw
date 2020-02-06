#
# Linux WBD Master Makefile
#
# $Copyright Broadcom Corporation$
#
# <<Broadcom-WL-IPTag/Open:>>
# $Id: Makefile 766304 2018-07-30 08:14:14Z pp888947 $
#

include $(SRCBASE_ROUTER)/.config

ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG

SHAREDDIR = ../shared
PREBUILTDIR = ../prebuilt
TARGET = wbd_master
WBDINSTALLDIR = $(INSTALLDIR)/wbd
BLANKETDIR = ../blanket
IEEE1905DIR = ieee1905
IEEE1905PATH = ../../$(IEEE1905DIR)

CFLAGS	+= -I. -I$(TOP)/shared -I$(TOP)/bshared/include -I$(LIBJSONC_DIR) \
		-I$(SRCBASE)/include -I$(SRCBASE)/../components/shared \
		-I$(SRCBASE)/common/include -I$(SRCBASE)/../components/wlioctl/include	\
		-I$(SRCBASE)/common/include/proto -I$(SRCBASE)/../components/shared/proto \
		-I$(SRCBASE)/../components/proto/include

ifeq ($(CONFIG_BCM_APPEVENTD), y)
	CFLAGS += -I$(SRCBASE)/router/appeventd
	CFLAGS += -I$(SRCBASE)/../components/router/appeventd
	CFLAGS += -I$(SRCBASE)/../components/apps/appeventd
endif
CFLAGS += -I$(SHAREDDIR) -I$(IEEE1905PATH) -I$(BLANKETDIR)
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -s -O2
LDFLAGS += -L$(LIBJSONC_DIR)/lib
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifneq ($(STBANDROID),1)
LDFLAGS += -lpthread
endif
LDFLAGS += -L$(TOP)/bshared/src -L$(INSTALLDIR)/bshared/usr/lib -lbshared
LDFLAGS += -L$(SHAREDDIR) -L$(INSTALLDIR)/wbd/usr/lib -lwbdshared
LDFLAGS += -lm
LDFLAGS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/usr/bin/libbmcrypto -lbcmcrypto
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -lieee1905
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -li5api

LDFLAGS += $(EXTRA_LDFLAGS)

ifneq ($(STBANDROID),1)
CFLAGS += -Wall -Werror
endif
LDFLAGS += -L$(LIBJSONC_DIR)/.libs -ljson-c

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs) $(BLANKETDIR)

all: $(TARGET)

ifneq ($(STBANDROID),1)
MASTER_INC = wbd_master_control.h wbd_master_com_hdlr.h
endif

OBJS = $(MASTER_INC) bcm_app_utils.o bcmxtlv.o wbd_master_control.o wbd_master_com_hdlr.o wbd_master.o

install: all
	install $(TARGET) $(PREBUILTDIR)

clean:
	rm -f *.o *.a *.so *.bak $(TARGET) *~

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

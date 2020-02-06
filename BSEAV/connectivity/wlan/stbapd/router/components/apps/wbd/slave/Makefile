#
# Linux WBD Slave Makefile
#
# $Copyright Broadcom Corporation$
#
# $Id: Makefile 766117 2018-07-25 02:45:53Z pp888947 $
#

include $(SRCBASE_ROUTER)/.config

ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
CFLAGS  += -DMULTIAP
CFLAGS  += -DMAP_TEMP_DEBUG

SHAREDDIR = ../shared
PREBUILTDIR = ../prebuilt
BLANKETDIR = ../blanket
IEEE1905DIR = ieee1905
IEEE1905PATH = ../../$(IEEE1905DIR)
TARGET = wbd_slave
WBDINSTALLDIR = $(INSTALLDIR)/wbd
SRCSHAREDDIR = $(SRCBASE)/shared

CFLAGS	+= -I. -I$(TOP)/shared -I$(TOP)/bshared/include -I$(LIBJSONC_DIR) \
		-I$(SRCBASE)/include -I$(SRCBASE)/../components/shared \
		-I$(SRCBASE)/common/include -I$(SRCBASE)/../components/wlioctl/include	\
		-I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/math/include \
		-I$(SRCBASE)/common/include/proto -I$(SRCBASE)/../components/shared/proto

ifeq ($(CONFIG_BCM_APPEVENTD), y)
	CFLAGS += -I$(SRCBASE)/router/appeventd
	CFLAGS += -I$(SRCBASE)/../components/router/appeventd
	CFLAGS += -I$(SRCBASE)/../components/apps/appeventd
endif
CFLAGS += -I$(SHAREDDIR) -I$(IEEE1905PATH) -I$(BLANKETDIR)
CFLAGS  += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -s -O2
LDFLAGS += -L$(LIBJSONC_DIR)/lib
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifneq ($(STBANDROID),1)
LDFLAGS += -lpthread
endif
LDFLAGS += -L$(TOP)/bshared/src -L$(INSTALLDIR)/bshared/usr/lib -lbshared
LDFLAGS += -L$(SHAREDDIR) -L$(INSTALLDIR)/wbd/usr/lib -lwbdshared
LDFLAGS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/usr/bin/libbmcrypto -lbcmcrypto
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -lieee1905
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -li5api

ifeq ($(PLC_WBD),1)
CFLAGS	+= -I$(SRCBASE)/include/homeplugctl_drv
LDFLAGS += -L$(TOP)/homeplugctl_drv -L$(INSTALLDIR)/homeplugctl_drv/usr/lib -lhomeplugctl_drv
endif

LDFLAGS += $(EXTRA_LDFLAGS)

ifneq ($(STBANDROID),1)
CFLAGS += -Wall -Werror
endif
LDFLAGS += -L$(LIBJSONC_DIR)/.libs -ljson-c

vpath %.c $(SRCBASE)/shared $(WLAN_ComponentSrcDirs)

all: $(TARGET)

ifneq ($(STBANDROID),1)
SLAVE_INC = wbd_wl_utility.h wbd_blanket_utility.h wbd_slave_control.h wbd_slave_com_hdlr.h
endif

OBJS = $(SLAVE_INC) bcmutils.o bcmxtlv.o bcm_app_utils.o wbd_wl_utility.o \
	wbd_blanket_utility.o wbd_slave_control.o wbd_slave_com_hdlr.o wbd_slave.o
ifeq ($(PLC_WBD),1)
OBJS +=  wbd_plc_utility.o
endif

install: all
	install $(TARGET) $(PREBUILTDIR)

clean:
	rm -f *.o *.a *.so *.bak $(TARGET) *~

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

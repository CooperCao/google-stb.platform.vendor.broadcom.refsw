#
# Linux WBD Shared Makefile
#
# $Copyright Broadcom Corporation$
#
# <<Broadcom-WL-IPTag/Open:>>
# $Id: Makefile 767294 2018-09-04 09:57:56Z la888950 $
#

include $(SRCBASE_ROUTER)/.config

ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
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
CFLAGS += -I$(BLANKETDIR) -I$(IEEE1905PATH)
CFLAGS	+= $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -s -O2
ifeq ($(STBANDROID),1)
LDFLAGS	= $(SHARED_LDFLAGS)
endif
LDFLAGS += -L$(LIBJSONC_DIR)/lib
LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifneq ($(STBANDROID),1)
LDFLAGS += -lpthread
endif
LDFLAGS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/usr/bin/libbmcrypto -lbcmcrypto
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -lieee1905
LDFLAGS += -L$(IEEE1905PATH) -L$(INSTALLDIR)/$(IEEE1905DIR)/usr/lib -li5api

LDFLAGS += $(EXTRA_LDFLAGS)

CFLAGS += -fPIC
ifneq ($(STBANDROID),1)
CFLAGS	+= -Wall -Werror
endif
LDFLAGS += -L$(LIBJSONC_DIR)/.libs -ljson-c
ifneq ($(STBANDROID),1)
LDFLAGS	= 
endif

PREBUILTDIR = ../prebuilt
TARGET = libwbdshared.so
WBDINSTALLDIR = $(INSTALLDIR)/wbd

SOURCES = wbd_utils.c wbd_sock_utility.c \
	wbd_json_utility.c wbd_shared.c wbd_ds.c _wbd_ds.c \
	wbd_com.c wbd_tlv.c $(BLANKETDIR)/blanket.c

SHARED_INC= wbd_sock_utility.h wbd.h \
	wbd_json_utility.h wbd_error.h wbd_tlv.h

ifeq ($(CONFIG_BCM_APPEVENTD), y)
	SOURCES += wbd_appeventd.c
	SHARED_INC += wbd_appeventd.h
endif
OBJS = $(SOURCES:.c=.o)

all: $(TARGET)

install: all
	install -m 755 $(TARGET) $(PREBUILTDIR)

clean:
	rm -f *.o *.so $(BLANKETDIR)/*.o $(TARGET)

$(TARGET): $(SHARED_INC) $(OBJS)
	$(LD) $(LDFLAGS) -shared -o $(TARGET) $(OBJS)

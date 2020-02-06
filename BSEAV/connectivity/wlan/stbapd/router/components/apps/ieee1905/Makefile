#
# Linux ieee1905 Makefile
#
# $Copyright Broadcom Corporation$
#
# $Id: Makefile 712953 2017-07-27 06:29:01Z vk016271 $
#

export USEBCMUSCHED = 1

# Create a EXE to test it individually
export CREATE_IEEE1905EXE = 1

IEEE1905INSTALLDIR = $(INSTALLDIR)/ieee1905
IEEE1905_PREBUILT = prebuilt

CFLAGS  += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
CFLAGS	+= -I.  -I$(TOP)/shared -I$(SRCBASE)/include -I$(WPSSOURCE_DIR)/common/include -I$(SRCBASE)/include/bcmcrypto -I$(SRCBASE)/../components/bcmcrypto/include -I$(SRCBASE)/../components/wlioctl/include -I$(SRCBASE)/../components/proto/include -I$(SRCBASE)/../components/shared/proto -I$(SRCBASE)/common/include/proto -I$(WPSSOURCE_DIR)/brcm_apps/include ${WLAN_StdIncPathA} -s -O2 -fPIC
ifneq ($(STBANDROID),1)
CFLAGS	+= -Wall -Werror
endif
CFLAGS  += -DWIRELESS -DSUPPORT_BRIDGE_DEDICATED_PORT -DSUPPORT_FBCTL
CFLAGS  += -DSUPPORT_IEEE1905_AUTO_WDS -DSUPPORT_FCCTL
ifeq ($(USEBCMUSCHED),1)
CFLAGS  += -DUSEBCMUSCHED
endif #USEBCMUSCHED

# CFLAGS  += -DSUPPORT_IEEE1905_FM -DWANDEV_SUPPORT -DIEEE1905_KERNEL_MODULE_PB_SUPPORT -DSUPPORT_ETHSWCTL

#ifdef BCMINTERNAL
ifneq ($(NO_BCMINTERNAL), 1)
CFLAGS	+= -DBCMINTERNAL
endif
CFLAGS	+= -DBCMDBG
#endif

IEEE1905_LIB	:= libieee1905.so
IEEE1905_LIB_SRCS1	:= $(shell find . -name "ieee*.c")
IEEE1905_LIB_SRCS1_TMP := $(IEEE1905_LIB_SRCS1)
IEEE1905_LIB_SRCS1 = $(filter-out ./ieee1905_main.c, $(IEEE1905_LIB_SRCS1_TMP))
IEEE1905_LIB_OBJS1	:= $(sort $(IEEE1905_LIB_SRCS1:.c=.o))

ifeq ($(CREATE_IEEE1905EXE), 1)
IEEE1905_EXE	:= ieee1905
IEEE1905_EXE_SRCS1	:= $(shell find . -name "ieee*.c")
IEEE1905_EXE_OBJS1	:= $(sort $(IEEE1905_EXE_SRCS1:.c=.o))
endif #CREATE_IEEE1905EXE

EXE2	:= i5ctl
LN2	:= i5
SRCS2	:= $(shell find . -name "i5ctl*.c")
OBJS2	:= $(SRCS2:.c=.o)

EXE3	:= ncap
SRCS3	:= $(shell find . -name "i5ncap*.c")
OBJS3	:= $(SRCS3:.c=.o)

LIB1	:= libi5api.so
LIBOBJS1:= i5api.o

LIBS = $(LIB1) $(EXTRA_LDFLAGS)
LIBS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
ifneq ($(STBANDROID),1)
LIBS += -lpthread
endif
LIBS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LIBS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib -lbcmcrypto

ifeq ($(CREATE_IEEE1905EXE), 1)
all: $(LIB1) $(IEEE1905_EXE) $(IEEE1905_LIB) $(EXE2) $(EXE3)
else
all: $(LIB1) $(IEEE1905_LIB) $(EXE2) $(EXE3)
endif

$(LIB1): $(LIBOBJS1)
	$(CC) $(CFLAGS) $(SHARED_LDFLAGS) -shared -o $@ $?

$(IEEE1905_LIB): $(IEEE1905_LIB_OBJS1) $(LIB1)
	$(CC) $(CFLAGS) $(SHARED_LDFLAGS) -shared -o $@ $(IEEE1905_LIB_OBJS1) $(LIBS)

$(IEEE1905_EXE): $(IEEE1905_EXE_OBJS1) $(LIB1)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(IEEE1905_EXE_OBJS1) $(LIBS)

$(EXE2): $(OBJS2) $(LIB1)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS2) $(LIBS)

$(EXE3): $(OBJS3) $(LIB1)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS3) $(LIBS)


install: all
	install -d $(IEEE1905INSTALLDIR)/usr/sbin/
	install -d $(IEEE1905INSTALLDIR)/usr/lib/
	install $(IEEE1905_LIB) $(IEEE1905INSTALLDIR)/usr/lib/
	install $(EXE2) $(IEEE1905INSTALLDIR)/usr/sbin/
	install $(EXE3) $(IEEE1905INSTALLDIR)/usr/sbin/
	install $(LIB1) $(IEEE1905INSTALLDIR)/usr/lib/
	install -d $(IEEE1905_PREBUILT)
	install $(EXE2) $(IEEE1905_PREBUILT)
	install $(EXE3) $(IEEE1905_PREBUILT)
	install $(IEEE1905_LIB) $(IEEE1905_PREBUILT)
	install $(LIB1) $(IEEE1905_PREBUILT)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/lib/$(IEEE1905_LIB)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/sbin/$(EXE2)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/sbin/$(EXE3)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/lib/$(LIB1)
ifeq ($(CREATE_IEEE1905EXE), 1)
	install $(IEEE1905_EXE) $(IEEE1905INSTALLDIR)/usr/sbin/
	install $(IEEE1905_EXE) $(IEEE1905_PREBUILT)
	$(STRIP) $(IEEE1905INSTALLDIR)/usr/sbin/$(IEEE1905_EXE)
endif #CREATE_IEEE1905EXE

clean:
	rm -rf *.o *.a *.so $(IEEE1905_LIB) $(IEEE1905_EXE) $(EXE2) $(EXE3) $(IEEE1905_PREBUILT)


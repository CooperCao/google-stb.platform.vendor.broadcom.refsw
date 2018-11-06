#
# 802.1x Authenticator (Network Access Server) Embedded Linux Build Makefile
#
# $Copyright (C) 2003 Broadcom Corporation$
#
# $Id: Makefile 660908 2016-09-22 10:46:46Z kugaurav $
#

include $(SRCBASE_ROUTER)/.config

CFLAGS	+= -I.. -I$(SRCBASE_SYS)/include ${WLAN_StdIncPathA} -I$(SRCBASE_SYS)/../components/shared -I$(SRCBASE_SYS)/common/include -I$(SRCBASE_ROUTER)/shared -Wall -Werror
CFLAGS	+= $(WLAN_ComponentIncPath)

CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s
CFLAGS  += -DNAS_WKSP_BUILD_NAS_AUTH
CFLAGS  += -DNAS_WKSP_BUILD_NAS_SUPPL
#CFLAGS  += -DNAS_GTK_PER_STA
CFLAGS  += -DNAS_RADIUS
CFLAGS  += -DNAS_WKSP_ON_DEMAND

CFLAGS	+= -DBCMSUPPL

# EAPD include path
CFLAGS	+= -I$(SRCBASE_ROUTER)/eapd

# NAS FBT include path
CFLAGS	+= -I$(SRCBASE_ROUTER)/nas/nas_fbt

LDFLAGS += -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS += -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared
LDFLAGS += -L$(TOP)/libbcmcrypto -L$(INSTALLDIR)/libbcmcrypto/usr/lib -lbcmcrypto
LDFLAGS += $(EXTRA_LDFLAGS)

vpath %.c $(SRCBASE_ROUTER)/nas $(SRCBASE_ROUTER)/nas/nas_fbt $(SRCBASE_SYS)/shared $(SRCBASE_ROUTER)/shared $(SRCBASE_SYS)/bcmcrypto

# build target
NASOBJS = mppe.o wpa.o nas.o nas_linux.o nas_wl.o nas_wksp.o
NASOBJS += nas_radius.o nas_wksp_radius.o
ifeq ($(CONFIG_FBT),y)
NASOBJS += wpa_common.o wpa_auth_ft.o eloop.o l2_packet.o
endif
UTLOBJS = bcmwpa.o bcmutils.o sha256.o hmac_sha256.o

all: nas

nas: $(NASOBJS) $(UTLOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# install to target
install: all
	install -D nas $(INSTALLDIR)/usr/sbin/nas
	$(STRIP) $(INSTALLDIR)/usr/sbin/nas

# clean up mess
clean:
	rm -f *.o *~ nas

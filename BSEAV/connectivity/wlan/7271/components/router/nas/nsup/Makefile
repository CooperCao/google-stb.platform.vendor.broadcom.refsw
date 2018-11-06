#
# 802.1x Supplicant Embedded Linux Build Makefile
#
# $Copyright (C) 2003 Broadcom Corporation$
#
# $Id: Makefile 403251 2013-05-20 05:15:55Z xwei $
#

CFLAGS	+= -I.. -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/../components/shared -I$(SRCBASE)/common/include -I$(SRCBASE_ROUTER)/shared -Wall -Werror

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
CFLAGS  += -DNAS_WKSP_BUILD_NAS_SUPPL
CFLAGS  += -DNAS_WKSP_ON_DEMAND

ifeq ($(CONFIG_BCMWPA2),y)
ADDED_CFLAGS += -DBCMWPA2
endif

CFLAGS	+= -DBCMSUPPL

# EAPD include path
CFLAGS	+= -I$(SRCBASE_ROUTER)/eapd

LDFLAGS	+= -L$(TOP)/nvram -L$(INSTALLDIR)/nvram/usr/lib -lnvram
LDFLAGS	+= -L$(TOP)/shared -L$(INSTALLDIR)/shared/usr/lib -lshared

vpath %.c $(SRCBASE)/router/nas $(SRCBASE)/bcmcrypto

# build target
NASOBJS = mppe.o wpa.o nas.o nas_linux.o nas_wl.o nas_wksp.o
ENCOBJS = aes.o aeskeywrap.o rijndael-alg-fst.o passhash.o prf.o md5.o hmac.o rc4.o sha1.o

all: nsup

nsup: $(NASOBJS) $(ENCOBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# install to target
install: all
	install -D nsup $(INSTALLDIR)/usr/sbin/nsup
	$(STRIP) $(INSTALLDIR)/usr/sbin/nsup

# clean up mess
clean:
	rm -f *.o *~ nsup

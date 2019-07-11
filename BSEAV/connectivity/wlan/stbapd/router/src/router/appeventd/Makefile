#
# Linux appeventd Makefile
#
# Copyright (C) 2018, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
# $Id:  $
#

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include
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
endif
LDFLAGS += $(EXTRA_LDFLAGS)


vpath %.c $(SRCBASE)/shared/

all: appeventd

OBJS = appeventd_main.o

install: all
	install -d $(INSTALLDIR)/usr/sbin/
	install appeventd $(INSTALLDIR)/usr/sbin/
	$(STRIP) $(INSTALLDIR)/usr/sbin/appeventd

clean:
	rm -f *.o *.a *.so appeventd

${OBJS}: appeventd.h

appeventd: bcmutils.o ${OBJS}
	$(CC) $(CFLAGS) $^ -o $@  $(LDFLAGS)

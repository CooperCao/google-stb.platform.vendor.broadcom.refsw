#
# BCM shared library Makefile for Linux router
#
# Copyright (C) 2018, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile 636868 2016-05-11 00:14:09Z $
#

include $(TOP)/.config


CFLAGS	+= -I. -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -I$(SRCBASE)/router/shared
ifneq ($(STBANDROID),1)
CCFLAGS	+= -Wall -Werror
endif
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -fPIC
ifeq ($(CONFIG_RTR_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif
ifeq ($(STBANDROID),1)
LDFLAGS = $(SHARED_LDFLAGS)
endif
LDFLAGS += -L.

LDFLAGS += -L$(TOP)/shared -lshared -L$(TOP)/nvram -lnvram

OBJS := bcmgpio.o


all: libbcm.so

install: all
	install -d $(INSTALLDIR)/usr/lib
	install -m 755 libbcm.so $(INSTALLDIR)/usr/lib
	$(STRIP) $(INSTALLDIR)/usr/lib/libbcm.so

clean:
	rm -f *.o *.so

libbcm.so: $(OBJS)
	$(LD) $(LDFLAGS) -shared -o $@ $^

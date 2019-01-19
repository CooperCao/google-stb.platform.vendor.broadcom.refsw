#
# Binary shared library Makefile for Linux router
#
# Copyright (C) 2018, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile 669585 2016-11-10 07:43:53Z $
#

include $(TOP)/.config


CFLAGS	+= -I. -I../include -I$(SRCBASE)/include ${WLAN_StdIncPathA} -I$(SRCBASE)/common/include -I$(TOP)/shared -Wall -Werror
CFLAGS  += $(if $(WLAN_ComponentIncPath),$(WLAN_ComponentIncPath),$(addprefix -I,$(wildcard $(SRCBASE)/shared/bcmwifi/include)))
#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -fPIC
ifeq ($(CONFIG_RTR_OPTIMIZE_SIZE),y)
CFLAGS  += -Os
else
CFLAGS  += -O2
endif
LDFLAGS += -L.

PREBUILTDIR = ../prebuilt
TARGET = libbshared.so

OBJS := bcm_stamon.o bcm_steering.o


all: $(TARGET)

install: all
	install -m 755 $(TARGET) $(PREBUILTDIR)
	$(STRIP) $(PREBUILTDIR)/$(TARGET)

clean:
	rm -f *.o *.so

$(TARGET): $(OBJS)
	$(LD) -shared -o $@ $^

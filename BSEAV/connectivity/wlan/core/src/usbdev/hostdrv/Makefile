#
# GNUmakefile for Broadcom host drivers
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

# Now try a couple of places for LINUXDIR if not specified
ifeq ($(LINUXDIR),)
ifeq ($(LINUXVER),)
# Neither one is specified, use uname for version
LINUXVER := $(shell uname -r)
endif
ifneq ($(wildcard /lib/modules/$(LINUXVER)/build/include/linux/version.h),)
LINUXDIR := /lib/modules/$(LINUXVER)/build
else
ifneq ($(wildcard /tools/linux/src/linux-$(LINUXVER)/include/linux/version.h),)
LINUXDIR := /tools/linux/src/linux-$(LINUXVER)
else
LINUXDIR := /usr/src/linux
endif
endif
endif

BCM_KVER := $(shell echo $(LINUXVER) | cut -c1-3 | sed 's/2\.[56]/2\.6/')

# driver source base
SRCBASE := $(shell /bin/pwd)/../..

# basic options
CFLAGS := -I. -I$(SRCBASE)/include
CFLAGS += -fshort-wchar
CFLAGS += -DBCMDRIVER

# host options
HOSTCC := $(CC) 
HOSTCFLAGS := $(CFLAGS) $(shell $(MAKE) --no-print-directory -s -C $(LINUXDIR) script 'SCRIPT=@echo $$(CFLAGS) $$(MODFLAGS)')

# Override HOSTCC if "cross-compiling" for another linux version
ifneq ($(LINUXVER),$(shell uname -r))
ifneq ($(CROSS_COMPILE),)
HOSTCC := $(CROSS_COMPILE)gcc
endif
endif

TARGETS := usb-cdc.o

ifneq ($(BCM_KVER),2.4)
all:
	$(MAKE) -C linux-2.6
else
all: $(TARGETS)
endif

usb-%.o: usb-%.c
	$(HOSTCC) $(HOSTCFLAGS) -c -o usb-$*-$(LINUXVER).o $< 

clean:
	rm -rf *.o *~

#
# GNUmakefile for umsd driver on Linux 2.6
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

# Assign LINUXVER and LINUXDIR if not specified. Try a few places for LINUXDIR.
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

SRCBASE := $(M)/../../..

EXTRA_CFLAGS += -I$(M)/.. -I$(SRCBASE)/include
EXTRA_CFLAGS += -Wall -Werror -Wstrict-prototypes -fshort-wchar
EXTRA_CFLAGS += -DBCMSDIO -DPR81999_WAR

obj-m += umsd.o
umsd-objs += umsd_main.o

default:
	$(MAKE) -C $(LINUXDIR) M=$(PWD) V=1 modules

clean:
	$(MAKE) -C $(LINUXDIR) M=$(PWD) V=1 clean

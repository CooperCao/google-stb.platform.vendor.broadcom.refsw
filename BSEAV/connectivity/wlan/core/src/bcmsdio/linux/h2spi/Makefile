#
# GNUmakefile for h2spi driver on Linux 2.6
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

BCM_KVER := $(shell echo $(LINUXVER) | cut -c1-3 | sed 's/2\.[56]/2\.6/')

SRCREL := ../../..

COMMON_CFLAGS += -I$(SRCBASE)/include
COMMON_CFLAGS += -Wall -Werror
COMMON_CFLAGS += -fshort-wchar
COMMON_CFLAGS += -Wstrict-prototypes

ifeq ($(BCM_KVER),2.4)

##############################################################################
# Linux 2.4
##############################################################################

# driver source base
SRCBASE := $(shell /bin/pwd)/$(SRCREL)

# basic options
CFLAGS += $(COMMON_CFLAGS)
CFLAGS += -I.

# host options
HOSTCC := $(CC) 
HOSTCFLAGS := $(CFLAGS) $(shell $(MAKE) --no-print-directory -s -C $(LINUXDIR) \
		script 'SCRIPT=@echo $$(CFLAGS) $$(MODFLAGS)')

# Override HOSTCC if "cross-compiling" for another linux version
ifneq ($(LINUXVER),$(shell uname -r))
  ifneq ($(CROSS_COMPILE),)
    HOSTCC := $(CROSS_COMPILE)gcc
  endif
endif

TARGETS := h2spi.o

all: $(TARGETS)

h2spi.o: h2spi_main.c h2spi.h
	$(HOSTCC) $(HOSTCFLAGS) -c -o h2spi.o $<

clean:
	rm -rf *.o *~

else	# BCM_KVER

##############################################################################
# Linux 2.6
##############################################################################

SRCBASE := $(M)/$(SRCREL)

# basic options
EXTRA_CFLAGS += $(COMMON_CFLAGS)
EXTRA_CFLAGS += -I$(M)/..

obj-m := h2spi.o
h2spi-objs += h2spi_main.o

default:
	$(MAKE) -C $(LINUXDIR) M=$(PWD) V=1 modules

clean:
	$(MAKE) -C $(LINUXDIR) M=$(PWD) V=1 clean

endif	# BCM_KVER

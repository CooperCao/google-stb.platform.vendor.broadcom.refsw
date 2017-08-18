#
# GNU Makefile for Broadcom Dongle Host Driver
#
# $Copyright$
#
# $Id$
#

# Assign LINUXVER and LINUXDIR if not specified.
# Try a couple of places for LINUXDIR.

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

# This Makefile is linked as <objdir>/Makefile, and is executed to build the
# actual module in the style of Linux 2.6.  This Makefile is included from
# Linux's build/scripts/Makefile.build.  Note that the current working directory
# is in the Linux source dir, and $(M) is the original <objdir>.

SRCBASE := $(M)/../../..

# basic options
EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(SRCBASE)/dhd/sys -I$(SRCBASE)/dongle
EXTRA_CFLAGS += -MD -Wall -Wstrict-prototypes -Werror -fshort-wchar
EXTRA_CFLAGS += -DBCMDRIVER
EXTRA_CFLAGS += -DSRCBASE=\"$(SRCBASE)\"

# usb-rndis.c has not been ported to Linux 2.6
#obj-m := usb-cdc.o usb-rndis.o
obj-m := usb-cdc.o

# Note: The source file name becomes the module name.  It must not have extra dots in the
# name (such as from LINUXVER); otherwise it may be impossible to rmmod the module!

default:
	ln -sf ../usb-cdc.c usb-cdc.c
	if [ -f ../usb-rndis.c ]; then ln -sf ../usb-rndis.c usb-rndis.c; fi
	$(MAKE) -C $(LINUXDIR) M=$(shell pwd) V=1 modules
	cp -f usb-cdc.ko usb-cdc-$(LINUXVER).ko
	if [ -f usb-rndis.ko ]; then cp -f usb-rndis.ko usb-rndis-$(LINUXVER).ko; fi

clean:
	rm -rf *.o *.ko *~
	rm -rf usb-cdc*.c usb-rndis*.c .usb-*.cmd .tmp_versions

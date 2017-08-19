#
# Makefile for  4335a0 USB bootloader based on hndrte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: Makefile 261896 2011-05-26 16:25:05Z $
#

SRCBASE = $(shell /bin/pwd)/../../../..

TARGETS = boot-rom run-debug boot-rom-debug-qt boot-rom-qt

CHIP	= 4335

TARGET_ARCH = arm
TARGET_CPU = cr4

NVRAM := 1
TCAM := 1
ZLIB := 0

TOOLSVER	:= 2010.09
TOOLSCCFLAGS	:= -fno-short-enums -Wno-strict-aliasing

# TODO: Update RAM size per chip
RAM_SIZE	:= 0xC0000	# 544KB RAM

# Adjust if needed 
ifneq ($(ZLIB),1)
BOOT_SIZE	?= 0x7800	# Data size needed for boot loader w/o zlib
else
BOOT_SIZE	:= 0xa400	# Data size needed for boot loader w/ zlib
endif

DATA_BASE	:= 0x180000

# DATA_START limits the max size of image to be downloaded.  Use the biggest possible.
#   Reserve 0x88000 - 0x80000 = 0x7800 (30720) bytes without ZLIB
ifeq ($(findstring boot,$(TARGET)),boot)
# Boot loader data start
DATA_START	:= $(shell awk 'BEGIN { \
			printf "0x%x\n", $(DATA_BASE) + ($(RAM_SIZE) - $(BOOT_SIZE)) \
			}')
endif
ifeq ($(findstring flash,$(TARGET)),flash)
# default Makerules assigns 0x1e000000 for ROM, flash is at 0x1c000000
TEXT_START	:= 0x1c000000
endif

# src/include/bcmdevs.h BCM_DNGL_BL_PID_4324
BCM_ID_PRODUCT	:= 0xbd1c

EXTRA_DFLAGS	+= -DBCMUSBDEV_ENABLED
EXTRA_DFLAGS	+= -DBCMUSBDEV_BULKIN_2EP

# This is an AI chip, select to eliminate SB code
EXTRA_DFLAGS	+= -DBCMCHIPTYPE=SOCI_AI -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS	+= -DHND_STACK_SIZE=2048
EXTRA_DFLAGS	+= -DDL_NVRAM=2044
EXTRA_DFLAGS	+= -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		   -DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn

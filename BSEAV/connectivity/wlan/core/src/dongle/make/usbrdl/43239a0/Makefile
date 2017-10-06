#
# Makefile for USB bootloader based on hndrte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$:
#

SRCBASE = $(shell /bin/pwd)/../../../..

TARGETS = boot-rom run-debug boot-rom-debug-qt boot-rom-qt boot-rom-qt-flash boot-rom-flash

CHIP	= 43239

NVRAM := 1
TCAM := 1
ZLIB := 0

DATA_BASE := 0x60000000

#   Reserve 0x70000 - 0x68800 = 0x7800 (30720) bytes without ZLIB
#   Reserve 0x70000 - 0x65c00 = 0xa400 (41984) bytes with ZLIB
ifeq ($(findstring boot,$(TARGET)),boot)
DATA_START	:= 0x60068800
else
TEXT_START	:= 0x00000000
endif
ifeq ($(findstring flash,$(TARGET)),flash)
# default Makerules assigns 0x1e000000 for ROM, flash is at 0x1c000000
TEXT_START	:= 0x1c000000
endif

# src/include/bcmdevs.h BCM_DNGL_BL_PID_43239
BCM_ID_PRODUCT := 0xbd1b

EXTRA_DFLAGS	+= -DBCMUSBDEV_BULKIN_2EP -DBCMUSBSUSPEND
# This is for devices which support only HSIC but not USB
EXTRA_DFLAGS	+= -DHSIC_ONLY

# This is an AI chip, select to eliminate SB code
EXTRA_DFLAGS	+= -DBCMCHIPTYPE=SOCI_AI -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS	+= -DHND_STACK_SIZE=2048
EXTRA_DFLAGS	+= -DDL_NVRAM=2044
EXTRA_DFLAGS	+= -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		   -DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn

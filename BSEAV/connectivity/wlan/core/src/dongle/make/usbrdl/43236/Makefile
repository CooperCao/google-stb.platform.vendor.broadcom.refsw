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
# $Id$
#

#
# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..

# override above to produce more useful targets
TARGETS := boot-rom run-debug boot-rom-flash boot-flash-nonxip run-btldr

CHIP := 43236

NVRAM := 1
TCAM := 1

ROMSIZE := 262144
ROMBANKSIZE := 65536
RAM_SIZE := 458752

# src/include/bcmdevs.h BCM_DNGL_BL_PID_43236
BCM_ID_PRODUCT := 0xbd17

DATA_BASE := 0x60000000
# The DATA_START will limit the max size of image to be downloaded, need to tune to biggest possible
ifeq ($(findstring boot,$(TARGET)),boot)
DATA_START	:= 0x60065C00
ifeq ($(findstring flash,$(TARGET)),flash)
# default Makerules assigns 0x1e000000 for ROM, flash is at 0x1c000000
TEXT_START	:= 0x1c000000
ifeq ($(findstring nonxip,$(TARGET)),nonxip)
DATA_START	:= 0x6006AC00
endif
endif	# flash
else	# !run
ifeq ($(findstring btldr,$(TARGET)),btldr)
TEXT_START	:= 0x00053800
else
TEXT_START	:= 0x00000000
endif
endif	# run

ifeq ($(findstring nonxip, $(TARGET)),nonxip)
EXTRA_DFLAGS += -DSFLASH_BOOTLOADER
EXTRA_DFLAGS += -DSF_BOOTLOADER_RAM_ADDRESS=0x60053800
EXTRA_DFLAGS += -DSF_BOOTLOADER_END_ADDRESS=0x6006a000
endif

ifeq ($(findstring btldr,$(TARGET)),btldr)
SFBOOTLOADER_NONXIP	:= 1
endif

EXTRA_DFLAGS += -DBCMUSBDEV_BULKIN_2EP

# This is AI chip, select to eliminate SB code
EXTRA_DFLAGS += -DBCMCHIPTYPE=SOCI_AI -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS += -DHND_STACK_SIZE=4096
EXTRA_DFLAGS += -DDL_NVRAM=2044
EXTRA_DFLAGS += -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		-DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn

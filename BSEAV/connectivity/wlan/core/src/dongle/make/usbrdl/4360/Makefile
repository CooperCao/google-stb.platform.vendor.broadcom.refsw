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
# $Id: Makefile 251173 2011-04-05 19:28:10Z $
#

#
# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..

# override above to produce more useful targets
TARGETS :=  boot-rom-thumb run-thumb-flash boot-rom-thumb-sdr run-thumb-flash-sdr run-thumb-sdr

CHIP := 4360
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4

NVRAM := 1
TCAM := 0

RAM_SIZE	:= 0xe0000	# 896KB RAM

BCM_ID_PRODUCT := 0xbd1d

DATA_BASE := 0x00000000
# The DATA_START will limit the max size of image to be downloaded, need to tune to biggest possible
ifeq ($(findstring boot,$(TARGET)),boot)
ROMSIZE := 65536
ROMBANKSIZE := 65536

TEXT_START	:= 0x000f0000
DATA_START	:= 0x000d0000

else	# !run
ifeq ($(findstring flash,$(TARGET)),flash)
TEXT_START	:= 0x000c0000
TRX_OFFSET1	:= 0x000c0001
else
TEXT_START	:= 0x00000000
TRX_OFFSET1	:= 0x00000001
endif #!flash
TRX_FLAGS	:= 0x0060
BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0
endif	# run

ZLIB		:= 0

ifeq ($(findstring run-thumb-flash,$(TARGET)),run-thumb-flash)
EXTRAOBJCOPYFLAGS += --qt-len 1
EXTRA_DFLAGS += -DSFLASH_BOOT
else
ifeq ($(findstring boot-rom,$(TARGET)),boot-rom)
EXTRAOBJCOPYFLAGS += --qt-len 8
endif
endif

#for cr4, thumb target should be used if the chip is configure to boot into thumb mode.
#all C code are compiled using thumb mode for all target 
ifeq ($(findstring thumb,$(TARGET)),thumb)
ASFLAGS += -mthumb
endif

ifeq ($(findstring rom,$(TARGET)),rom)
STARTUP := boot
#STARTUP_OBJECTS := $(BOOT_OBJECTS)
endif

ifeq ($(findstring sdr,$(TARGET)),sdr)
SDR := 1
ZLIB := 0
EXTRA_DFLAGS += -DHND_STACK_SIZE=20480
else
EXTRA_DFLAGS += -DHND_STACK_SIZE=4096
EXTRA_DFLAGS += -DDL_NVRAM=2044
endif

# This is AI chip, select to eliminate SB code
EXTRA_DFLAGS += -DBCMCHIPTYPE=SOCI_AI -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS += -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		-DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128
EXTRA_DFLAGS += -DBCMUSBDEV_BULKIN_2EP
EXTRA_DFLAGS += -DUSB_IFTEST

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn

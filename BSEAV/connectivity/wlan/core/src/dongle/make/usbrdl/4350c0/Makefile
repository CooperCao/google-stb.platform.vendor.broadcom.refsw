#
# Makefile for 4350 USB bootloader based on hndrte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: Makefile$
#

#
# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..

# override above to produce more useful targets
TARGETS = boot-rom-thumb run-thumb-flash run-thumb

CHIP := 4350
TARGET_ARCH := arm
TARGET_CPU := cr4

NVRAM := 1
TCAM := 1
ZLIB := 0
TCAM_PCNT := 1
FLOPS_SUPPORT := 1
HBUS_ENAB := usb
BOOTLOADER_PATCH_SIZ := 0

TOOLSVER	:= 2011.09
TOOLSCCFLAGS	:= -fno-short-enums -Wno-strict-aliasing

BCM_ID_PRODUCT	:= 0xbd23

# TODO: Update RAM size per chip
RAM_SIZE	:= 0xC0000	# 768KB RAM

# 48KB for bootloader stack & heap 
BOOT_SIZE	:= 0xC000

DATA_BASE	:= 0x180000

PATCH_TBL_SIZE := 0x800

# DATA_START limits the max size of image to be downloaded.  Use the biggest possible.
#   Reserve 0x88000 - 0x80000 = 0x7800 (30720) bytes without ZLIB
# Boot loader data start
DATA_START	:= $(shell awk 'BEGIN { \
			printf "0x%x\n", $(DATA_BASE) + ($(RAM_SIZE) - $(BOOT_SIZE)) \
			}')
ifeq ($(findstring boot,$(TARGET)),boot)
ROMSIZE := 65536
ROMBANKSIZE := 65536

# TEXT_START for bootloader [ROM]
TEXT_START	:= 0x00000000
else # !boot = run
ifeq ($(findstring flash,$(TARGET)),flash)
# TEXT_START for bootloader [flash]. Note that, this is B1TCM start,-
# - where the flash code is copied and executed.
TEXT_START	:= 0x210000
TRX_OFFSET1	:= 0x210081
else # !flash = ram
# TEXT_START for bootloader [RAM] run imgs
TEXT_START	:= 0x00220000
TRX_OFFSET1	:= 0x00180081
endif # flash
TRX_FLAGS	:= 0x0060
BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0
endif	# run

ifeq ($(findstring run-thumb-flash,$(TARGET)),run-thumb-flash)
EXTRAOBJCOPYFLAGS += --qt-len 1
EXTRA_DFLAGS += -DSFLASH_BOOT
else
ifeq ($(findstring boot-rom,$(TARGET)),boot-rom)
# Don't need this because of a bug in objcopy for swapqt format with the below flag, this issue 
# is now taken care by qtmem_4to8.pl script.  
#EXTRAOBJCOPYFLAGS += --qt-len 8

MEM_WIDTH := 8
endif
endif

#for cr4, thumb target should be used if the chip is configure to boot into thumb mode.
#all C code are compiled using thumb mode for all target 
ifeq ($(findstring thumb,$(TARGET)),thumb)
ASFLAGS += -mthumb
endif

ifeq ($(findstring rom,$(TARGET)),rom)
STARTUP := boot
endif

EXTRA_DFLAGS 	+= -DHND_STACK_SIZE=4096
EXTRA_DFLAGS    += -DBCMTRXV2
EXTRA_DFLAGS	+= -DBCMUSBDEV_ENABLED
EXTRA_DFLAGS	+= -DBCMUSBDEV_BULKIN_2EP
EXTRA_DFLAGS    += -DBCMUSBDEV_COMPOSITE
EXTRA_DFLAGS 	+= -DUSB_IFTEST
EXTRA_DFLAGS	+= -DUSB_XDCI
EXTRA_DFLAGS	+= -DXDC_U1U2_DISABLE

# This is an AI chip, select to eliminate SB code
EXTRA_DFLAGS	+= -DBCMCHIPTYPE=SOCI_AI -DBCMBUSTYPE=SI_BUS
EXTRA_DFLAGS	+= -DUSBBULK_RXBUFS=4 -DUSB_NTXD=8 -DUSBCTL_RXBUFS=4 \
		   -DUSB_NRXD=8 -DUSB_RXBND=-1 -DUSBCTLBUFSZ=128

# default GPIO behavior for this chip's bootloader:
# wait infinitely for CC4350_PIN_GPIO_01 to go high (HOST_READY)
EXTRA_DFLAGS   += -DDEFAULT_USBFLAGS=0x11ff
# DEVICE_READY is on GPIO10, active high
EXTRA_DFLAGS   += -DDEFAULT_USBRDY=0x0a

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/usbrdl/Makefile.cmn

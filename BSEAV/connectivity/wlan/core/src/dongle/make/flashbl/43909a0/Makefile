#
# Makefile for 43909 ACPU and WCPU bootloader based on hndrte
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: Makefile 424563 2013-09-18 11:19:48Z $
#

#
# top level make
#
SRCBASE = $(shell /bin/pwd)/../../../..

# override above to produce more useful targets
TARGETS = boot-rom-thumb-wlan boot-rom-thumb-apps run-thumb-flash-wlan run-thumb-flash-apps-debug run-thumb boot-rom-thumb-qt-wlan boot-rom-thumb-qt-apps-debug

CHIP := 43909
TARGET_ARCH := arm
TARGET_CPU := cr4

NVRAM := 1
TCAM := 1
ZLIB := 0
TCAM_PCNT := 1
FLOPS_SUPPORT := 1
HBUS_ENAB := m2m
BOOTLOADER_PATCH_SIZ := 0

TOOLSVER	:= 2011.09
TOOLSCCFLAGS	:= -fno-short-enums -Wno-strict-aliasing

ifeq ($(findstring apps,$(TARGET)),apps)
     CPU_CORE = apps
endif

ifeq ($(findstring wlan,$(TARGET)),wlan)
     CPU_CORE = wlan
endif

BCM_ID_PRODUCT	:= 0xbd25

# TODO: Update RAM size per chip
ifeq ($(CPU_CORE),apps)
#APPS SOCSRAM
RAM_SIZE	:= 0xC0000	# 2MB SOCSRAM
else
#WLAN TCM
RAM_SIZE	:= 0x90000	# 576KB TCM
endif

# 48KB for bootloader stack & heap 
BOOT_SIZE	:= 0xC000

ifeq ($(CPU_CORE),apps)
DATA_BASE	:= 0x4A0000
else
#WLAN
DATA_BASE	:= 0x1B0000
endif
#DATA_BASE	:= 0x000000

PATCH_TBL_SIZE := 0x800

# DATA_START limits the max size of image to be downloaded.  Use the biggest possible.
#   Reserve 0x88000 - 0x80000 = 0x7800 (30720) bytes without ZLIB
# Boot loader data start
DATA_START	:= $(shell awk 'BEGIN { \
			printf "0x%x\n", $(DATA_BASE) + ($(RAM_SIZE) - $(BOOT_SIZE)) \
			}')

ifeq ($(findstring apps,$(TARGET)),apps)
EXTRA_DFLAGS += -DSOCRAM -DSECOND_ARM
EXTRA_DFLAGS += -DSOCRAM_BASE=0x4A0000
endif

ifeq ($(findstring boot,$(TARGET)),boot)
ifeq ($(CPU_CORE),apps)
#APPS ROM SIZE = 640KB
ROMSIZE := 655360 
#APPS BANK SIZE = 128KB
ROMBANKSIZE := 131072 

#APPS TEXT_START for bootloader [ROM]
TEXT_START	:= 0x400000
FLOPS_START	:= 0x400000
else
#WLAN ROM SIZE = 448KB
ROMSIZE := 0x70000
#WLAN BANK SIZE = 64KB
ROMBANKSIZE := 0x10000 

# TEXT_START for bootloader [ROM]
TEXT_START	:= 0x000000
FLOPS_START	:= 0x000000
endif
else # !boot = run
ifeq ($(findstring flash,$(TARGET)),flash)
# TEXT_START for bootloader [flash]. Note that, this is B1TCM start,-
# - where the flash code is copied and executed.
ifeq ($(CPU_CORE),apps)
#Set the TEX_START address as the start of SOCSRAM
TEXT_START	:= 0x4A0000
TRX_OFFSET1	:= 0x4A0081
else
#Set the TEX_START address as the start of ATCM
TEXT_START	:= 0x1B0000
TRX_OFFSET1	:= 0x1B0081
endif# CPU_CORE != apps
else # !flash = ram
# TEXT_START for bootloader [RAM] run imgs
TEXT_START	:= 0x4A0000
TRX_OFFSET1	:= 0x4A0081
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

# This is an AI chip, select to eliminate SB code
EXTRA_DFLAGS	+= -DBCMCHIPTYPE=SOCI_AI -DBCMBUSTYPE=SI_BUS

# include common rule after all configuration is setup
include $(SRCBASE)/dongle/make/flashbl/Makefile.cmn

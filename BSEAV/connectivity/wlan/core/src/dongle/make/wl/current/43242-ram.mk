#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 43242-ram.mk 368192 2012-11-12 17:56:01Z $:

# chip specification
CHIP	:= 43242
#REVID	:= 0

# default targets
TARGETS := \
	usb-ag \
	usb-ag-assert \
	usb-ag-assert-err \
	usb-ag-p2p-mchan

TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
TARGET_HBUS	:= usb
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 0

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43242_ram
WLTUNEFILE	:= wltunable_rte_43242.h

SMALL		:= 1
MEMBASE		:= 0
MEMSIZE		:= 589824	# Hardcoding it saves ~112 bytes from startarm.S
FLASH		:= 0
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
BCMPKTPOOL	:= 1
DMATXRC		:= 0
WLRXOV		:= 0
PROP_TXSTATUS	:= 0
NODIS		:= 0
#NO_BCMINTERNAL	:= 1

# p2p dongle code support
VDEV		:= 0
SDTEST		:= 0
POOL_LEN	:= 30
POOL_LEN_MAX	:= 60
HND_STACK_SIZE	:= 4608
WL11D		:= 0

SKIP_IMAGES     := rtecdc.trx

# if MAXASSOC_LIMIT is set, use it for maxassoc; otherwise use MAXSCB
MAXASSOC_LIMIT	:= 10

EXTRA_DFLAGS	+= -DHND_STACK_SIZE=$(HND_STACK_SIZE)

#radio specific defines
BCMRADIOREV     := 0x1
BCMRADIOVER     := 0x1
BCMRADIOID      := 0x022e

EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)

# Enable compiler option for inlining of simple functions for targets that run almost entirely from
# RAM. Inlining actually saves memory due to the elimination of function call overhead and more
# efficient register usage.
NOINLINE	:=

# Use the 2013.11 version of the toolchain. It provides significant memory savings
# relative to the older toolchain when building full-RAM or mostly-RAM images.
TOOLSVER	= 2013.11

# Enable compiler optimizations that might rename functions in order to save memory.
NOFNRENAME	:= 0

# Use functions instead of macros for isset/isclr/setbit/clrbit to save memory.
EXTRA_DFLAGS	+= -DBCMUTILS_BIT_MACROS_USE_FUNCS

# Disabling RADAR, ACI and SROM8 code for 43242
EXTRA_DFLAGS    += -DWLC_DISABLE_DFS_RADAR_SUPPORT
EXTRA_DFLAGS    += -DWLC_DISABLE_ACI
EXTRA_DFLAGS    += -DWLC_DISABLE_SROM8

# Making PHY_IPA compile time option
EXTRA_DFLAGS    += -DWLPHY_IPA_ONLY

#wowl gpio pin4 polarity at logic low is 1
WOWL_GPIOPIN	= 0x4
WOWL_GPIO_POLARITY = 0x1
EXTRA_DFLAGS    += -DWOWL_GPIO=$(WOWL_GPIOPIN)  -DWOWL_GPIO_POLARITY=$(WOWL_GPIO_POLARITY)

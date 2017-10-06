#
# Makefile to generate romtable.S for 4365c0 ROM builds
#
# This makefile expects romctl.txt to exist in the roml directory
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4365c0.mk$
#

####################################################################################################
# This makefile is used when building a ROM, specifically when generating a symbol table that
# represents the ROM contents.  It is not used when building a ROM offload image. The roml makefile
# (src/make/roml/<chipid>/Makefile) inherits the settings in this romlsym file.
#
# Settings are defined in the 'wlconfig', settings in there should not be redefined in this file.
#
# The makefile generates "romtable_full.S", which is renamed to "romtable.S" when it is copied into
# the src/dongle/make/roml/"chipidchiprev" directory.
####################################################################################################

# chip specification
CHIP		:= 4365
REV		:= c0
REVID		:= 4

TARGETS		:= pcie

# common target attributes
TARGET_HBUS	:= pcie
TARGET_ARCH	:= arm
TARGET_CPU	:= ca7
THUMB		:= 1
HBUS_PROTO	:= msgbuf
BAND		:= ag

# ROMCTL needed for location of romctl.txt
ROMCTL		:= $(TOPDIR)/../roml/$(CHIP)$(REV)/romctl.txt

# wlconfig
WLCONFFILE	:= wlconfig_rte_4365c0_dev

# wltunable
ifeq ($(DEVSIM_BUILD),1)
#	WLTUNEFILE	:= wltunable_rte_$(DEVSIM_CHIP)$(DEVSIM_CHIP_REV).h
	ROMCTL		:= $(TOPDIR)/../roml/$(DEVSIM_CHIP)sim-$(CHIP)$(REV)/romctl.txt
	CHIP		:= $(DEVSIM_CHIP)
	DBG_ASSERT      := 1
else
	WLTUNEFILE	:= wltunable_rte_4365c0.h
	DBG_ASSERT      := 0
endif

# features (sync with make/wl/current/4365c0-roml.mk)
#By the default, CA7 owns 0x1c0000 (1,835,008) bytes
#for FLOPS first 64bytes
MEMBASE		:= 0x00000040
MEMSIZE		:= 2228224
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ERROR	:= 1
PROP_TXSTATUS	:= 1
VDEV		:= 1

HNDLBUFCOMPACT  := 0
DLL_USE_MACROS  := 1
HNDLBUF_USE_MACROS := 1

# To limit PHY core checks
EXTRA_DFLAGS	+= -DBCMPHYCORENUM=$(BCMPHYCORENUM)

# CLM info
CLM_TYPE	:= min

# extra flags
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=64
EXTRA_DFLAGS	+= -DIBSS_PEER_GROUP_KEY_DISABLED
EXTRA_DFLAGS	+= -DIBSS_PEER_DISCOVERY_EVENT_DISABLED
EXTRA_DFLAGS	+= -DIBSS_PEER_MGMT_DISABLED

ifneq ($(ROMLIB),1)
ROMBUILD	:= 1
EXTRA_DFLAGS	+= -DBCMROMSYMGEN_BUILD
endif

#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR

TOOLSVER	:= 2013.11
NOFNRENAME	:= 1

BCMPKTPOOL	:= 1
BCMFRAGPOOL	:= 1
BCMRXFRAGPOOL	:= 1
BCMLFRAG	:= 1
POOL_LEN_MAX	:= 60
#FRAG_POOL_LEN should be less than POOL_LEN_MAX
FRAG_POOL_LEN   := 60
RXFRAG_POOL_LEN := 192
BCMPKTIDMAP     := 1
BCMSPLITRX	:= 1
#Microsoft Extensible STA for Dongle
NDISFW=1
EXTSTA=1

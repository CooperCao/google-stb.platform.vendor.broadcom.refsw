# Makefile for hndrte based 4347 standalone programs,
#	to generate romtable.S for 4347a0 ROM builds
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4347a0.mk 453855 2014-02-06 15:27:00Z $:

# chip specification
CHIP		:= 4347
REV		:= a0
REVID		:= 1
PCIEREVID	:= 19
CCREV		:= 60
PMUREV		:= 32
GCIREV		:= 11
AOBENAB		:= 1
OTPWRTYPE	:= 2

TARGETS		:= threadx-sdio-pcie

# common target attributes
TARGET_HBUS	:= sdio pcie
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
THUMB		:= 1
HBUS_PROTO	:= cdc msgbuf
BAND		:= ag

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_$(CHIP)$(REV)_dev

# ROMCTL needed for location of romctl.txt
ROMCTL		:= $(TOPDIR)/../roml/$(CHIP)$(REV)/romctl.txt

WLTUNEFILE	:= wltunable_rte_$(CHIP)$(REV).h
DBG_ASSERT      := 0

PCIE_DMACHANNUM	:= 3

# features (sync with 4347a0.mk, 4347a0-roml.mk)
MEMSIZE		:= 1376256 #1344KB
MFGTEST		:= 1
WLTINYDUMP	:= 0
DBG_ERROR	:= 0
POOL_LEN_MAX	:= 60
SDTEST		:= 0
DMATXRC	:= 1
PROP_TXSTATUS := 1
DNG_DBGDMP := 1

BCMSPLITRX	:= 1
BCMFRAGPOOL	:= 1
BCMLFRAG	:= 1
BCMPKTIDMAP	:= 1
THREADX		:= 1
HNDRTE		:= 0

MAX_MPU_REGION	:= 12
BCMPCIE_IFRM	:= 1

ifeq ($(findstring pcie,$(TARGET)),pcie)
POOL_LEN_MAX    := 200
POOL_LEN        := 40
MFGTESTPOOL_LEN := 20
WL_POST		:= 40
endif
FRAG_POOL_LEN   := 200
RXFRAG_POOL_LEN   := 120

WL_POST_CLASSIFIED_FIFO := 4
WL_CLASSIFY_FIFO := 2
COPY_CNT_BYTES  := 64

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	:= min

#these are features used for olympic
SRSCAN		:= 1
BCM_LOGTRACE	:= 1
#BCM_EVENT_LOG	:= 1

# extra flags
EXTRA_DFLAGS	+= -DSHARE_RIJNDAEL_SBOX	# Save 1400 bytes; wep & tkhash slightly slower
EXTRA_DFLAGS	+= -DROBUST_DISASSOC_TX
#Include non-WLTEST version for fun's having conditional WLTEST logic in ROM (to prevent production image invalidation).
#functions exclusively used by WLTEST are included in ROM
EXTRA_DFLAGS	+= -DWLTEST_DISABLED
ifneq ($(ROMLIB),1)
ROMBUILD	:= 1
EXTRA_DFLAGS	+= -DBCMROMSYMGEN_BUILD
endif

TOOLSVER := 2013.11

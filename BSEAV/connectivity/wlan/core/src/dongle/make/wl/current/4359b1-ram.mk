# Makefile for hndrte based 4359b1 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4359b1-ram.mk 523451 2014-12-30 09:59:09Z $

# chip specification

# ?= assignment required to handle 4359 case too
CHIP	?= 4359
ROMREV	?= b1
REV	:= b1
REVID	?= 5
PCIEREVID := 14

# default targets
TARGETS := \
	threadx-sdio-ag \
	threadx-sdio-ag-p2p-mchan-assert-err \
	threadx-pcie-ag-msgbuf-splitrx

TEXT_START := 0x180000
# normally 0x00180900 when bootloader is present. 2K for bootloader patch table entries
DATA_START := 0x180100


# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio pcie
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 1

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4359b1_rsdb_bu
WLTUNEFILE	:= wltunable_rte_4359b1.h

# use different CONFIG file for norsdb builds to add -assert-err
ifeq ($(call opt,norsdb),1)
WLCONFFILE	:= wlconfig_rte_4359b1_bu
endif

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))
# features (sync with 4359b1.mk, 4359b1-roml.mk)
MEMBASE		:= 0x180000
MEMSIZE		:= 786432	# Hardcoding it saves ~112 bytes from startarm.S
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT_TRAP	:= 0

POOL_LEN_MAX	:= 60
POOL_LEN	:= 30
WL_POST		:= 6
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 64

SDPCMD_RXBUFS	:= 16
SDPCMD_NRXD	:= 32
SDPMCD_NTXD	:= 128
SDPCMD_RXBND	:= 10
SDPCMD_TXGLOM	:= 8

BUS_RXBUFS_MFGC := 18
BUS_NRXD_MFGC	:= 32
BUS_NTXD_MFGC	:= 64



ifeq ($(call opt,pcie),1)
POOL_LEN_MAX		:= 60
FRAG_POOL_LEN		:= 30
RXFRAG_POOL_LEN		:= 30
POOL_LEN		:= 20
MFGTESTPOOL_LEN		:= 30
WL_POST			:= 8
WL_NRXD			:= 128
WL_NTXD			:= 128
PKT_MAXIMUM_ID          := 640
WL_RXBND                := 40
WL_RXBND_SMALL		:= 12
ifeq ($(call opt,splitrx),1)
WL_POST			:= 6
WL_NRXD			:= 128
POOL_LEN		:= 12
WL_POST_CLASSIFIED_FIFO := 4
WL_SPLITRX_MODE	:= 3
WL_CLASSIFY_FIFO := 2
COPY_CNT_BYTES	:= 32
ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS    += -DFORCE_RX_FIFO1
endif
endif
EXTRA_DFLAGS    += -DWL_RXBND=$(WL_RXBND)
EXTRA_DFLAGS    += -DWL_RXBND_SMALL=$(WL_RXBND_SMALL)
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=16
EXTRA_DFLAGS    += -DPCIEDEV_MAX_LOCALBUF_COUNT=6
endif

ifeq ($(call opt,mfgtest),1)
POOL_LEN_MAX    := 40
endif

# p2p dongle code support
VDEV		:= 1
# Default RSDB chip
RSDB		:= 1
# CLM info
CLM_TYPE	:= 4359b1
# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE		:= 8
AMPDU_SCB_MAX_RELEASE_AQM	:= 16

# Makeconf defines BCM4324 and BCM4324B0 based on $(CHIP)$(ROMREV), but we also need B0
EXTRA_DFLAGS	+= -DBCM$(CHIP) -DBCM$(CHIP)$(ROMREV)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=$(AMPDU_RX_BA_DEF_WSIZE)
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=$(AMPDU_SCB_MAX_RELEASE_AQM)
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

BCMPKTPOOL	:= 1
FLOPS_SUPPORT	:= 1
BCMPKTIDMAP	:= 1

# Default RSDB chip
RSDB		:=1

# PHY related Fixed for 4359b1
# common between RAM, ROM and ROM-Offload
BCMSROMREV	:=0x0b
BCMRADIOID	:=0x03eb
BCMRADIOMAJORREV:=0x2
BCMPHYACMINORREV:=0x2



# DMA Complete WAR - To handle the situation where the
# Doorbell interrupt from Dongle to Host arrives before
# the DMA is complete. If BCMCHKD2HDMA is set by default
# the WAR used is "Modulo-253 SeqNum marker". Enabling
# this method as default for Dingo FW (4359A2)
BCMCHKD2HDMA := 1

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2013.11

ifeq ($(call opt,die0),1)
BCMRADIOREV  := 0x13
BCMRADIOVER  := 0x13
endif
ifeq ($(call opt,die5),1)
BCMRADIOREV  := 0x15
BCMRADIOVER  := 0x15
endif

BCMRADIOREV  := 0x14
BCMRADIOVER  := 0x14

EXTRA_DFLAGS	+= -DWLPHY_IPA_ONLY
EXTRA_DFLAGS	+= -DASDB_DBG
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# Makefile for hndrte based 4369a0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4369a0-ram.mk $:

# chip specification

# ?= assignment required to handle 4355 case too
CHIP	?= 4369
CHIPROM	?= 4369
ROMREV	?= a0
REV	:= a0
REVID	:= 0
PCIEREVID	:= 24
CCREV		:= 61
PMUREV		:= 33
GCIREV		:= 12
AOBENAB		:= 1
OTPWRTYPE	:= 2

# default targets
TARGETS := \
	threadx-pcie-ag \
	threadx-pcie-ag-p2p-mchan-assert-err \
	threadx-pcie-ag-msgbuf-splitrx

TEXT_START := 0x170000
DATA_START := 0x170100


# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= pcie
THUMB		:= 1
HBUS_PROTO	:= msgbuf
# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4369a0_bu
WLTUNEFILE	:= wltunable_rte_4369a0.h

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))
PCIE_DMACHANNUM	:= 3
# features (sync with 4369a0.mk, 4369a0-roml.mk)
MEMBASE		:= 0x170000
MEMSIZE		:= 1638400
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
SDTEST		:= 0
PROP_TXSTATUS	:= 1
POOL_LEN_MAX	:= 100
POOL_LEN	:= 10
WL_POST		:= 8

BUS_NRXD	:= 32
BUS_NTXD	:= 64

BUS_RXBUFS_MFGC := 18
BUS_NRXD_MFGC	:= 32
BUS_NTXD_MFGC	:= 64
MFGTESTPOOL_LEN	:= 50

# MPU region
MAX_MPU_REGION	:= 12

# Implicit Flow Ring Manager
BCMPCIE_IFRM := 1

SRSCAN		:= 1
BCM_LOGTRACE	:= 1
BCM_ERR_USE_EVENT_LOG := 1
#BCM_EVENT_LOG	:= 1

#PKTIDMAP feature
BCMPKTIDMAP		:= 1

BCMSPLITRX	:= 1
BCMLFRAG	:= 1
BCMFRAGPOOL	:= 1
FRAG_POOL_LEN	:= 32
RXFRAG_POOL_LEN	:= 32

# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 24
AMPDU_SCB_MAX_RELEASE_AQM       := 16

POOL_LEN_MAX		:= 400
FRAG_POOL_LEN		:= 200
RXFRAG_POOL_LEN		:= 200
POOL_LEN		:= 10
MFGTESTPOOL_LEN		:= 10
WL_POST			:= 64
WL_POST_FIFO1		:= 4
WL_POST_SMALL		:= 3
WL_NRXD			:= 512
WL_NTXD			:= 64
WL_NTXD_LFRAG		:= 512
PKT_MAXIMUM_ID		:= 640
WL_RXBND                := 64
WL_RXBND_SMALL		:= 16
WL_POST_CLASSIFIED_FIFO := 4

WL_SPLITRX_MODE	:= 2
WL_CLASSIFY_FIFO := 2
COPY_CNT_BYTES	:= 20
ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS    += -DFORCE_RX_FIFO1
endif

H2D_DMAQ_LEN		:= 256
D2H_DMAQ_LEN		:= 256
MAX_HOST_RXBUFS		:= 256


EXTRA_DFLAGS	+= -DWL_RXBND=$(WL_RXBND)
EXTRA_DFLAGS    += -DWL_RXBND_SMALL=$(WL_RXBND_SMALL)
EXTRA_DFLAGS	+= -DMAX_TX_STATUS_COMBINED=96 -DPD_NBUF_D2H_TXCPL=4
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=8

EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=40
EXTRA_DFLAGS    += -DPCIEDEV_MAX_LOCALBUF_COUNT=6

# max fetch count at once

PCIEDEV_MAX_PACKETFETCH_COUNT	:= 64


# Aux tunables
WL_RSDB_AUX_BUFPOST	:= 1
WL_NRXD_AUX		:= 256
WL_POST_AUX		:= 16
WL_RXBND_AUX		:= 16
WL_NTXD_LFRAG_AUX	:= 256

ifeq ($(call opt,mfgtest),1)
POOL_LEN_MAX    := 40
endif

MAXSZ_NVRAM	:= 13312
EXTRA_DFLAGS += -DMAXSZ_NVRAM_VARS=$(MAXSZ_NVRAM)
EXTRA_DFLAGS += -DDL_NVRAM=9000

ifeq ($(call opt,assert),1)
FRAG_POOL_LEN           := 100
RXFRAG_POOL_LEN         := 100
POOL_LEN                := 8
endif # end assert

# p2p dongle code support
VDEV		:= 1
# CLM info
CLM_TYPE	:= 4369a0


#Enable FW Support for DMA'ing r/w
#indice by default if xorcsum is not enabled
ifeq ($(call opt,xorcsum),0)
EXTRA_DFLAGS += -DPCIE_DMA_INDEX
endif

# Makeconf defines BCM4324 and BCM4324B0 based on $(CHIP)$(ROMREV), but we also need B0
EXTRA_DFLAGS	+= -DBCM$(CHIP) -DBCM$(CHIP)$(ROMREV)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=$(AMPDU_RX_BA_DEF_WSIZE)
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=$(AMPDU_SCB_MAX_RELEASE_AQM)
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

BCMPKTPOOL	:= 1
FLOPS_SUPPORT	:= 1

# Default RSDB chip
RSDB		:=1

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2013.11

BCMSROMREV	:= 0x0b

BCMRADIOREV	:= 0x6
BCMRADIOID	:= 0x5FB
EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# Use Macro definitions for LBUF function
HNDLBUF_USE_MACROS	:= 1
DLL_USE_MACROS:= 1

# PCIe rev16 error attention feature
PCIE_ERR_ATTN_CHECK := 1

# Feature flag to support PKT engine TX request caching
ifeq ($(call opt,mfgtest),1)
EXTRA_DFLAGS	+= -DPKTENG_TXREQ_CACHE
endif

# Supporting dump rsdb iovar
EXTRA_DFLAGS    += -DDNG_DBGDUMP -DBCMDBG_RSDB


# Enable Txpwr compensation parameters
EXTRA_DFLAGS += -DPOWPERCHANNL

LOG_BUF_LEN		:= 4096
REDUX			:= 1

#Enable GCI Mailbox interrupt
EXTRA_DFLAGS  += -DWLGCIMBHLR

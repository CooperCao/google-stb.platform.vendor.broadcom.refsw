# Makefile for hndrte based 4349a0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4364a0-ram.mk 498584 2014-08-25 07:59:03Z $:

# chip specification

# ?= assignment required to handle 4355 case too
CHIP	?= 4364
ROMREV	?= a0
REV	:= a0
REVID	?= 0
PCIEREVID := 16
OTPWRTYPE := 1

# default targets
TARGETS := \
	threadx-sdio-ag \
	threadx-sdio-ag-p2p-mchan-assert-err \
	threadx-pcie-ag-msgbuf-splitrx

TEXT_START := 0x160000
DATA_START := 0x160100


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
ifeq ($(call opt,norsdb),1)
WLCONFFILE	:= wlconfig_rte_4364a0_bu
else
WLCONFFILE	:= wlconfig_rte_4364a0_rsdb_bu
endif
WLTUNEFILE	:= wltunable_rte_4364a0.h

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))
# features (sync with 4364a0.mk, 4364a0-roml.mk)
MEMBASE		:= 0x160000
MEMSIZE		:= 1310720
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT_TRAP	:= 0

POOL_LEN_MAX	:= 60
POOL_LEN	:= 30
WL_POST		:= 6
MFGTESTPOOL_LEN	:= 30
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
WL_POST			:= 8
WL_NRXD			:= 128
WL_NTXD			:= 128
ifeq ($(call opt,splitrx),1)
WL_POST			:= 6
WL_NRXD			:= 128
POOL_LEN		:= 12
WL_POST_CLASSIFIED_FIFO := 4
WL_SPLITRX_MODE	:= 2
WL_CLASSIFY_FIFO := 2
COPY_CNT_BYTES	:= 32
ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS    += -DFORCE_RX_FIFO1
endif
endif
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=16
EXTRA_DFLAGS    += -DPCIEDEV_MAX_LOCALBUF_COUNT=6
endif

ifeq ($(call opt,mfgtest),1)
POOL_LEN_MAX    := 40
endif

ifeq ($(call opt,norsdb),1)
BUS_POST		:= 6
WL_RXBND		:= 24
else
WL_POST			:= 50
WL_POST_AUX		:= 20
WL_RXBND		:= 12
WL_RXBND_AUX	:= 12
WL_NRXD_AUX		:= 64
WL_RSDB_AUX_BUFPOST	:= 1
WL_NTXD_LFRAG_AUX	:= 512
endif

#Supporting nvram size upto 13K
MAXSZ_NVRAM	:= 13312
EXTRA_DFLAGS += -DMAXSZ_NVRAM_VARS=$(MAXSZ_NVRAM)

# p2p dongle code support
VDEV		:= 1
# CLM info
CLM_TYPE	:= 4364a0
# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE		:= 8
AMPDU_SCB_MAX_RELEASE_AQM	:= 16

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

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

#Enable dongle pkt chaining
EXTRA_DFLAGS    += -DPKTC_DONGLE

BCMPKTPOOL	:= 1
FLOPS_SUPPORT	:= 1

# Default RSDB chip
RSDB		:=1

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2013.11


ifeq ($(call opt,norsdb),1)
BCMRADIOREV	:= 0x40
BCMRADIOID	:= 0x2069
EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)
else
BCMRADIO2069REV  := 0x40
BCMMULTIRADIO0    := 0x2069

BCMRADIO20691REV  := 0x81
BCMMULTIRADIO1 := 0x30B

EXTRA_DFLAGS    += -DBCMRADIO2069REV=$(BCMRADIO2069REV)
EXTRA_DFLAGS	+= -DBCMMULTIRADIO0=$(BCMMULTIRADIO0)
EXTRA_DFLAGS    += -DBCMRADIO20691REV=$(BCMRADIO20691REV)
EXTRA_DFLAGS	+= -DBCMMULTIRADIO1=$(BCMMULTIRADIO1)
endif
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# Use Macro definitions for LBUF function
HNDLBUF_USE_MACROS	:= 1
BCMPKTIDMAP := 1
DLL_USE_MACROS:= 1
# PCIe rev 16 rx wait for completion
PCIDMA_WAIT_CMPLT_ON := 0
# PCIe rev16 error attention feature
PCIE_ERR_ATTN_CHECK := 1
EXTRA_DFLAGS += -DWAR_HW_RXFIFO_0

# Feature flag to support PKT engine TX request caching
ifeq ($(call opt,mfgtest),1)
EXTRA_DFLAGS	+= -DPKTENG_TXREQ_CACHE
endif
RSDB_PM_MODESW:= 1

# Supporting dump rsdb iovar
EXTRA_DFLAGS    += -DDNG_DBGDUMP -DBCMDBG_RSDB

BCMOTPSROM	:= 1

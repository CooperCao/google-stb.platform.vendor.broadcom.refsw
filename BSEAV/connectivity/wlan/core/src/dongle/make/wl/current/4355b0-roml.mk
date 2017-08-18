# Makefile for hndrte based 4355a0 ROM Offload image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4355b0-roml.mk 510052 2014-10-22 23:03:25Z $

# chip specification
CHIP	?= 4355
CHIPROM	?= 4355
ROMREV	:= b0
REV	:= b0
REVID	:= 4
PCIEREVID := 14
# default targets
TARGETS := \
	threadx-sdio-ag \
	threadx-sdio-ag-pool-idsup-idauth-p2p-pno-aoe-toe-keepalive \
	threadx-sdio-ag-ccx-p2p-idsup-idauth-pno \
	threadx-sdio-ag-mfgtest-seqcmds \
	threadx-sdio-ag-mfgtest-seqcmds-ndis \
	threadx-sdio-ag-pool-ndis-reclaim \
	threadx-sdio-ag-cdc-ndis-reclaim \
	threadx-sdio-ag-pool-idsup	\

TEXT_START := 0x180000
# normally 0x00180900 when bootloader is present. 2K for bootloader patch table entries
DATA_START := 0x180100

ROM_LOW	 ?= 0x00000000
ROM_HIGH ?= 0x000C0000

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio pcie
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 0

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4355b0_dev
WLTUNEFILE	:= wltunable_rte_4355b0.h


# ROM image info
ROMOFFLOAD	:= 1
ROMLDIR		:= $(TOPDIR)/../../images/roml/$(CHIPROM)$(ROMREV)
ROMLLIB         := roml.exe

JMPTBL_FULL	:= 1
JMPTBL_TCAM	:= 1
GLOBALIZE	:= 1
# Do not use LBUF static inline functions
HND_LBUF_NO_STATIC_INLINE	:= 1

# features (sync with 4355b0.mk, 4355b0-roml.mk)
#MEMBASE		:= 0
MEMSIZE		:= 786432
MEMBASE		:= 0x180000
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ERROR	:= 1
SDTEST		:= 0
PROP_TXSTATUS	:= 1
POOL_LEN_MAX	:= 100
POOL_LEN	:= 10
WL_POST		:= 8
BUS_NRXD	:= 32
BUS_NTXD	:= 64

SDPCMD_RXBUFS	:= 16
SDPCMD_NRXD	:= 32
SDPMCD_NTXD	:= 128
SDPCMD_RXBND	:= 10
SDPCMD_TXGLOM	:= 8

BUS_RXBUFS_MFGC	:= 18
BUS_NRXD_MFGC	:= 32
BUS_NTXD_MFGC	:= 64
MFGTESTPOOL_LEN	:= 50

# TCAM feature
# If changing these values, clean all and rebuild
TCAM		:= 1
TCAM_PCNT	:= 1
TCAM_SIZE	:= 256

#these are features used for olympic
SRSCAN		:= 1
BCM_LOGTRACE	:= 1
#BCM_EVENT_LOG	:= 1

#PKTIDMAP feature
BCMPKTIDMAP		:= 1

BCMSPLITRX	:= 1
BCMLFRAG	:= 1
BCMFRAGPOOL	:= 1

# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 24
AMPDU_SCB_MAX_RELEASE_AQM       := 16

ifeq ($(call opt,assert),1)
DBG_ASSERT_TRAP := 1
endif

# Change for 1ant on RSDB
ifeq ($(call opt,1ant),1)
WLRSDB_1ANT	:= 1
endif

ifeq ($(call opt,pcie),1)
POOL_LEN_MAX		:= 400
FRAG_POOL_LEN		:= 100
RXFRAG_POOL_LEN		:= 100
POOL_LEN		:= 50
MFGTESTPOOL_LEN		:= 10
WL_POST			:= 40
WL_POST_SMALL		:= 32
WL_NRXD			:= 64
WL_NTXD			:= 64
WL_NTXD_LFRAG		:= 64
PKT_MAXIMUM_ID		:= 640
WL_RXBND                := 40
WL_RXBND_SMALL		:= 16
# Making splitrx default for PCIEbuilds
#ifeq ($(call opt,splitrx),1)
WL_POST			:= 32
WL_NRXD			:= 64
POOL_LEN		:= 10
WL_POST_CLASSIFIED_FIFO := 4
WL_SPLITRX_MODE	:= 2
WL_CLASSIFY_FIFO := 2
COPY_CNT_BYTES	:= 20
ifeq ($(WL_SPLITRX_MODE),4)
COPY_CNT_BYTES	:= 16
endif
#endif splitrx
ifeq ($(call opt,ampduhostreorder),1)
# Core-0 or MIMO - WL_POST =40 + 16(chain depth), Core-1 or SISO- WL_POST= 20+16
RXFRAG_POOL_LEN	:= 140
WL_POST_SMALL	:= 25
WL_POST			:= 75
WL_NRXD			:= 128
ifeq ($(WL_SPLITRX_MODE),2)
WL_NRXD			:= 512
endif
WL_RXBND_SMALL	:= 12
WL_RXBND		:= 30
endif #end ampduhostreorder

EXTRA_DFLAGS	+= -DWL_RXBND=$(WL_RXBND)
EXTRA_DFLAGS    += -DWL_RXBND_SMALL=$(WL_RXBND_SMALL)

ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS	+= -DFORCE_RX_FIFO1
endif
EXTRA_DFLAGS	+= -DFORCE_AMSDURX
EXTRA_DFLAGS	+= -DMAX_TX_STATUS_COMBINED=64 -DPD_NBUF_D2H_TXCPL=4
EXTRA_DFLAGS   += -DMAX_HOST_RXBUFS=256
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=8
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=16
EXTRA_DFLAGS	+= -DDNG_DBGDUMP
# max fetch count at once

PCIEDEV_MAX_PACKETFETCH_COUNT	:= 64
PCIEDEV_MAX_LOCALITEM_COUNT	:= 64
PCIEDEV_MAX_LOCALBUF_PKT_COUNT	:= 200
EXTRA_DFLAGS    += -DPCIEDEV_MAX_PACKETFETCH_COUNT=$(PCIEDEV_MAX_PACKETFETCH_COUNT)
EXTRA_DFLAGS	+= -DPCIEDEV_MAX_LOCALITEM_COUNT=$(PCIEDEV_MAX_LOCALITEM_COUNT)
# buffers in the tx fetch pool
EXTRA_DFLAGS    += -DPCIEDEV_MAX_LOCALBUF_PKT_COUNT=$(PCIEDEV_MAX_LOCALBUF_PKT_COUNT)

# pcie ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 64
AMPDU_SCB_MAX_RELEASE_AQM       := 24

#
ifeq ($(call opt,sstput),1)
FRAG_POOL_LEN		:= 256
WL_NTXD_LFRAG		:= 512
#enable UCODE DBG logs for catching Mac Suspend / MQ Errors
#in sstput build. will need to be removed after the issue is resolved
EXTRA_DFLAGS	+= -DWLUCODE_DBG
endif

endif # end pcie
#

ifeq ($(call opt,mfgtest),1)
ifeq ($(call opt,ate),1)
WL_POST         := 10
WL_NRXD         := 16
WL_NTXD         := 16
POOL_LEN        := 5
MFGTESTPOOL_LEN := 5
endif
WL_POST         := 32
WL_NRXD         := 64
WL_NTXD         := 64
endif

# p2p dongle code support
VDEV		:= 1
# Default RSDB chip
RSDB		:= 1
# CLM info
CLM_TYPE	?= 4355b0

# Makeconf defines BCM4324 and BCM4324B0 based on $(CHIP)$(ROMREV), but we also need B0
EXTRA_DFLAGS	+= -DBCM$(CHIP) -DBCM$(CHIP)$(ROMREV)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=$(AMPDU_RX_BA_DEF_WSIZE)
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=$(AMPDU_SCB_MAX_RELEASE_AQM)
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

BCMPKTPOOL := 1
FLOPS_SUPPORT := 1

# Default RSDB chip
RSDB		:= 1

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR
EXTRA_DFLAGS	+= -DROBUST_DISASSOC_TX

ifeq ($(call opt,die0),1)
BCMRADIOREV  := 0x0e
BCMRADIOVER  := 0x0e
endif
ifeq ($(call opt,die5),1)
BCMRADIOREV  := 0x12
BCMRADIOVER  := 0x12
endif
ifeq ($(call opt,die3),1)
BCMRADIOREV  := 0x0f
BCMRADIOVER  := 0x0f
EXTRA_DFLAGS	+= -DXTALFREQ_37P4
endif

BCMRADIOREV  ?= 0x12
BCMRADIOVER  ?= 0x12
BCMSROMREV	?= 0x0b
BCMRADIOID	?= 0x03eb

EXTRA_DFLAGS	+= -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMSROMREV=$(BCMSROMREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)
BCMRADIOMAJORREV  := 0x2
EXTRA_DFLAGS      += -DBCMRADIOMAJORREV=$(BCMRADIOMAJORREV)

# PhyFw memory optimization
BCMPHYACMINORREV  := 0x0
EXTRA_DFLAGS      += -DBCMPHYACMINORREV=$(BCMPHYACMINORREV)

ifeq ($(call opt,epaonly),1)
ifndef WLPHY_EPA_ONLY
WLPHY_EPA_ONLY ?= 1
endif
ifndef WLPHY_IPA_ONLY
WLPHY_IPA_ONLY ?= 0
endif
endif

ifndef WLPHY_IPA_ONLY
WLPHY_IPA_ONLY ?= 1
endif
ifeq ($(WLPHY_IPA_ONLY),1)
EXTRA_DFLAGS	+= -DWLPHY_IPA_ONLY
endif

ifeq ($(WLPHY_EPA_ONLY),1)
EXTRA_DFLAGS	+= -DWLPHY_EPA_ONLY
endif

# default params : bcm94359fcpagbss.txt
FEMCTRL		?= 14
BOARD_FLAGS	?= 0x10081201
BOARD_FLAGS2	?= 0x0
BOARD_FLAGS3	?= 0x08500186
ifeq ($(call opt,nvramadj),0)
EXTRA_DFLAGS	+= -DFEMCTRL=$(FEMCTRL) -DBOARD_FLAGS=$(BOARD_FLAGS)
EXTRA_DFLAGS	+= -DBOARD_FLAGS2=$(BOARD_FLAGS2) -DBOARD_FLAGS3=$(BOARD_FLAGS3)
endif

ifeq ($(call opt,dcthrot),1)
EXTRA_DFLAGS += -DDUTY_CYCLE_THROTTLING
endif


# DMA Complete WAR - To handle the situation where the
# Doorbell interrupt from Dongle to Host arrives before
# the DMA is complete. If BCMCHKD2HDMA is set by default
# the WAR used is "Modulo-253 SeqNum marker". Enabling
# this method as default for Dingo FW (4359A2)
BCMCHKD2HDMA := 1

#Manual Patching Option
#WLPATCHFILE    := wlc_patch_4355b0.c
#EXAMPLE_PATCH  := 1

TOOLSVER := 2011.09
#this will be enabled for mfgtest
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

# Enable MEMLPLDO
EXTRA_DFLAGS	+= -DUSE_MEMLPLDO
EXTRA_DFLAGS    += -DWLC_11ACHDRS_ROM_COMPAT
EXTRA_DFLAGS += -DBCMPHYCAL_CACHING
EXTRA_DFLAGS += -DWAR_HW_RXFIFO_0

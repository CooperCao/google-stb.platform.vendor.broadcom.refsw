# Makefile for hndrte based 4355a0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4355b0-ram.mk 491449 2014-07-16 11:50:08Z $

# chip specification

# ?= assignment required to handle 4355 case too
CHIP	?= 4355
ROMREV	?= b0
REV	:= b0
REVID	:= 4
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
WLCONFFILE	:= wlconfig_rte_4355b0_rsdb_bu
WLTUNEFILE	:= wltunable_rte_4355b0.h

# use different CONFIG file for norsdb builds to add -assert-err
ifeq ($(call opt,norsdb),1)
WLCONFFILE	:= wlconfig_rte_4355b0_bu
endif

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))
# features (sync with 4355b0.mk, 4355b0-roml.mk)
MEMSIZE		:= 786432	# Hardcoding it saves ~112 bytes from startarm.S
MEMBASE		:= 0x180000


MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT_TRAP	:= 0


PROP_TXSTATUS	:= 1
POOL_LEN_MAX	:= 100
POOL_LEN	:= 10
BUS_POST	:= 6
WL_POST		:= 6
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

#PKTIDMAP feature
BCMPKTIDMAP := 1

BCMSPLITRX	:= 1
BCMLFRAG	:= 1
BCMFRAGPOOL	:= 1

# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 24
AMPDU_SCB_MAX_RELEASE_AQM       := 16


ifeq ($(call opt,pcie),1)
POOL_LEN_MAX		:= 60
FRAG_POOL_LEN		:= 30
RXFRAG_POOL_LEN		:= 30
POOL_LEN		:= 20
MFGTESTPOOL_LEN		:= 30
WL_POST			:= 8
WL_NRXD			:= 64
WL_NTXD			:= 64
WL_RXBND := 30
WL_RXBND_SMALL  := 12
ifeq ($(call opt,splitrx),1)
WL_POST			:= 6
WL_NRXD			:= 512
POOL_LEN		:= 12
WL_POST_CLASSIFIED_FIFO := 4
WL_SPLITRX_MODE	:= 2
WL_CLASSIFY_FIFO := 2
COPY_CNT_BYTES	:= 20
ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS    += -DFORCE_RX_FIFO1
endif

EXTRA_DFLAGS	+= -DMAX_TX_STATUS_COMBINED=64 -DPD_NBUF_D2H_TXCPL=4
EXTRA_DFLAGS   += -DMAX_HOST_RXBUFS=256
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=8
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=16

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
CLM_TYPE	:= 4355b0
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

# Default RSDB chip
RSDB		:=1

# DMA Complete WAR - To handle the situation where the
# Doorbell interrupt from Dongle to Host arrives before
# the DMA is complete. If BCMCHKD2HDMA is set by default
# the WAR used is "Modulo-253 SeqNum marker". Enabling
# this method as default for Dingo FW (4359A2)
BCMCHKD2HDMA := 1

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2013.11

#EXTRA_DFLAGS	+= -DWLGPIOHLR

ifeq ($(call opt,die0),1)
BCMRADIOREV  := 0x0e
BCMRADIOVER  := 0x0e
endif
ifeq ($(call opt,die5),1)
BCMRADIOREV  := 0x12
BCMRADIOVER  := 0x12
endif

BCMRADIOREV  ?= 0x12
BCMRADIOVER  ?= 0x12
BCMSROMREV	?= 0x0b
BCMRADIOID	?= 0x03eb

EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)
BCMRADIOMAJORREV  := 0x2
EXTRA_DFLAGS      += -DBCMRADIOMAJORREV=$(BCMRADIOMAJORREV)

# PhyFw memory optimization
BCMPHYACMINORREV:=0x0
EXTRA_DFLAGS      += -DBCMPHYACMINORREV=$(BCMPHYACMINORREV)

EXTRA_DFLAGS	+= -DWLPHY_IPA_ONLY
# default params : bcm943550fcpagb.txt, sLNA configuration
FEMCTRL		?= 14
BOARD_FLAGS	?= 0x10481201
BOARD_FLAGS2	?= 0x0
BOARD_FLAGS3	?= 0x48500106
ifeq ($(call opt,nvramadj),0)
EXTRA_DFLAGS	+= -DFEMCTRL=$(FEMCTRL) -DBOARD_FLAGS=$(BOARD_FLAGS)
EXTRA_DFLAGS	+= -DBOARD_FLAGS2=$(BOARD_FLAGS2) -DBOARD_FLAGS3=$(BOARD_FLAGS3)
endif



PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

EXTRA_DFLAGS	+= -DUSE_MEMLPLDO
EXTRA_DFLAGS += -DBCMPHYCAL_CACHING
# Enabling Analytic PAPD by default
EXTRA_DFLAGS += -DWL_APAPD

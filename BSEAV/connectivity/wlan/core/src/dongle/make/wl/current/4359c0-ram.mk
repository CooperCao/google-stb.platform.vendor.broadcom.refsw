# Makefile for hndrte based 4359c0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4359c0-ram.mk 584449 2015-09-07 06:11:20Z $

# chip specification

# ?= assignment required to handle 4359 case too
CHIP	?= 4359
ROMREV	?= c0
REV	:= c0
REVID	?= 9
PCIEREVID := 21

# default targets
TARGETS := \
	threadx-sdio-ag \
	threadx-sdio-ag-p2p-mchan-assert-err \
	threadx-pcie-ag-msgbuf-splitrx

TEXT_START := 0x160000
# normally 0x00160900 when bootloader is present. 2K for bootloader patch table entries
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
WLCONFFILE	:= wlconfig_rte_4359c0_rsdb_bu
WLTUNEFILE	:= wltunable_rte_4359c0.h

# use different CONFIG file for norsdb builds to add -assert-err
ifeq ($(call opt,norsdb),1)
WLCONFFILE	:= wlconfig_rte_4359c0_bu
endif

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))
# features (sync with 4359c0.mk, 4359c0-roml.mk)
MEMBASE		:= 0x160000
MEMSIZE		:= 917504	# Hardcoding it saves ~112 bytes from startarm.S
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT_TRAP	:= 0
DBG_ERROR	:= 0
SDTEST		:= 0
PROP_TXSTATUS	:= 0
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
MFGTESTPOOL_LEN	:= 50



#PKTIDMAP feature
BCMPKTIDMAP		:= 1

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

ifeq ($(call opt,hthrot),1)
EXTRA_DFLAGS += -DHIRSSI_TEMP_THROTTLING
endif

ifeq ($(call opt,pcie),1)
#PCIE BUS debug
PCIE_BUS_DBG := 0
POOL_LEN_MAX		:= 60
FRAG_POOL_LEN		:= 30
# Enabled RESV Pool for sharing with SCB
BCMRESVFRAGPOOL		:= 1
RESV_FRAG_POOL_LEN	:= 60
RXFRAG_POOL_LEN		:= 30
MFGTESTPOOL_LEN		:= 30
WL_POST			:= 8
WL_NTXD			:= 64
WL_NTXD_LFRAG		:= 512
PKT_MAXIMUM_ID          := 640
WL_RXBND                := 40
WL_RXBND_SMALL		:= 16
BCMSPLITRX		:= 1
BCMLFRAG		:= 1
BCMFRAGPOOL		:= 1
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

BUFTUNE := 0
ifeq ($(call opt,ampduhostreorder),1)
BUFTUNE := 1
endif #end ampduhostreorder

ifeq ($(BUFTUNE),1)
RXFRAG_POOL_LEN	:= 140
WL_POST_SMALL	:= 25
WL_POST			:= 60
WL_NRXD			:= 128
WL_NRXD_SMALL		:= 64
WL_RXBND_SMALL	:= 12
WL_RXBND		:= 30
endif #end buftune

EXTRA_DFLAGS    += -DWL_RXBND=$(WL_RXBND)
EXTRA_DFLAGS    += -DWL_RXBND_SMALL=$(WL_RXBND_SMALL)

ifeq ($(WL_SPLITRX_MODE),2)
EXTRA_DFLAGS	+= -DFORCE_RX_FIFO1
endif
EXTRA_DFLAGS	+= -DFORCE_AMSDURX
EXTRA_DFLAGS	+= -DMAX_TX_STATUS_BUF_LEN=128
EXTRA_DFLAGS	+= -DMAX_RXCPL_BUF_LEN=128
EXTRA_DFLAGS	+= -DMAX_HOST_RXBUFS=512
EXTRA_DFLAGS	+= -DPD_NBUF_H2D_RXPOST=8
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=40
# max fetch count at once

PCIEDEV_MAX_PACKETFETCH_COUNT	:= 64
PCIEDEV_MAX_LOCALITEM_COUNT	:= 64
PCIEDEV_MAX_LOCALBUF_COUNT	:= 6
EXTRA_DFLAGS    += -DPCIEDEV_MAX_PACKETFETCH_COUNT=$(PCIEDEV_MAX_PACKETFETCH_COUNT)
EXTRA_DFLAGS	+= -DPCIEDEV_MAX_LOCALITEM_COUNT=$(PCIEDEV_MAX_LOCALITEM_COUNT)
# buffers in the tx fetch pool
EXTRA_DFLAGS    += -DPCIEDEV_MAX_LOCALBUF_COUNT=$(PCIEDEV_MAX_LOCALBUF_COUNT)

# pcie ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 64
AMPDU_SCB_MAX_RELEASE_AQM       := 24

#Enable DFS AP move
ifeq ($(call opt,rsdbdfs),1)
EXTRA_DFLAGS += -DRSDB_DFS_SCAN
endif

endif # end pcie
ifeq ($(call opt,mfgtest),1)
ifeq ($(call opt,ate),1)
WL_POST         := 10
WL_NRXD         := 64
WL_NTXD         := 16
POOL_LEN        := 5
MFGTESTPOOL_LEN := 5
endif
WL_POST         := 32
WL_NRXD         := 256
WL_NTXD         := 64
endif

# p2p dongle code support
VDEV		:= 1
# Default RSDB chip
RSDB		:= 1
# CLM info
CLM_TYPE	:= 4359c0

# Makeconf defines BCM4324 and BCM4324B0 based on $(CHIP)$(ROMREV), but we also need B0
EXTRA_DFLAGS	+= -DBCM$(CHIP) -DBCM$(CHIP)$(ROMREV)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=$(AMPDU_RX_BA_DEF_WSIZE)
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=$(AMPDU_SCB_MAX_RELEASE_AQM)
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Implicit Txbf
ifeq ($(call opt,imptxbf),1)
EXTRA_DFLAGS += -DIMPLICIT_TXBF
endif
#EXTRA_DFLAGS	+= -DWLASSOC_DBG
# Disable Downgraded P2P scan.
#EXTRA_DFLAGS += -DRSDB_P2P_DISC_DISABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

BCMPKTPOOL := 1
FLOPS_SUPPORT := 1

# Default RSDB chip
RSDB		:=1

# PHY related Fixed for 4359c0
# common between RAM, ROM and ROM-Offload
BCMSROMREV	:=0x0b
BCMRADIOID	:=0x03eb
BCMRADIOMAJORREV:=0x2
BCMPHYACMINORREV:=0x6



# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR
EXTRA_DFLAGS	+= -DROBUST_DISASSOC_TX

#ifeq ($(call opt,die0),1)
#BCMRADIOREV  := 0x13
#BCMRADIOVER  := 0x13
#WLPHY_IPA_ONLY := 0
#WLPHY_EPA_ONLY := 1
#endif
ifeq ($(call opt,die3),1)
BCMRADIOREV  := 0x19
BCMRADIOVER  := 0x19
WLPHY_IPA_ONLY := 0
WLPHY_EPA_ONLY := 0
endif
#ifeq ($(call opt,die5),1)
#BCMRADIOREV  := 0x19
#BCMRADIOVER  := 0x19
#WLPHY_IPA_ONLY := 0
#WLPHY_EPA_ONLY := 0
#endif

# DMA Complete WAR - To handle the situation where the
# Doorbell interrupt from Dongle to Host arrives before
# the DMA is complete. If BCMXORCSUM is set, it will
# calcualte xor checksum + Modulo-253 SeqNum marker for
# every d2h messages ensuring DMA is done.
# Enabling this method by default.
BCMXORCSUM := 1


TOOLSVER := 2011.09
#this will be enabled for mfgtest
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192
# Enable MEMLPLDO
EXTRA_DFLAGS	+= -DUSE_MEMLPLDO
EXTRA_DFLAGS += -DBCMPHYCAL_CACHING

ifeq ($(call opt,norsdb),0)
EXTRA_DFLAGS += -DRSDB_SWITCH
endif

BCMRADIOREV  := 0x14
BCMRADIOVER  := 0x14

EXTRA_DFLAGS	+= -DWLPHY_IPA_ONLY

# Makefile for hndrte based 4347a0 ROM Offload image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4347a0-roml.mk 523451 2014-12-30 09:59:09Z $

# chip specification
CHIP		?= 4347
CHIPROM		?= 4347
ROMREV		:= a0
REV		:= a0
REVID		:= 1
PCIEREVID	:= 19
CCREV		:= 60
PMUREV		:= 32
GCIREV		:= 11
AOBENAB		:= 1
OTPWRTYPE	:= 2

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

TEXT_START := 0x170000
DATA_START := 0x170100

ROM_LOW	 ?= 0x00000000
ROM_HIGH ?= 0x00170000

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio pcie
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 0

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4347a0_dev
WLTUNEFILE	:= wltunable_rte_4347a0.h


# ROM image info
ROMOFFLOAD	:= 1
ROMLDIR		:= $(TOPDIR)/../../../../components/chips/images/roml/$(CHIPROM)$(ROMREV)
ROMLLIB         := roml.exe

JMPTBL_FULL	:= 1
JMPTBL_TCAM	:= 1
GLOBALIZE	:= 1
# Use Macro definitions for LBUF function
#HNDLBUF_USE_MACROS     := 1
# Use macros instead of bit manipulation functions like setbit, clrbit etc
#BCMUTILS_BIT_MACROS_USE_FUNCS  := 0

PCIE_DMACHANNUM	:= 3

# features (sync with 4347a0.mk, 4347a0-roml.mk)
MEMSIZE		:= 1376256 # 1344KB
MEMBASE		:= 0x170000
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP := 0
DBG_ERROR	:= 0
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
TCAM_PCNT	:= 2
TCAM_SIZE	:= 512

# MPU region
MAX_MPU_REGION	:= 12

# Implicit Flow Ring Manager
BCMPCIE_IFRM := 1

SRSCAN		:= 1
BCM_LOGTRACE	:= 1
BCM_ERR_USE_EVENT_LOG := 0
#BCM_EVENT_LOG	:= 1

#PKTIDMAP feature
BCMPKTIDMAP		:= 1

BCMSPLITRX	:= 1
BCMLFRAG	:= 1
BCMFRAGPOOL	:= 1

# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 24
AMPDU_SCB_MAX_RELEASE_AQM       := 16

ifeq ($(call opt,pcie),1)
POOL_LEN_MAX		:= 400
FRAG_POOL_LEN		:= 256
RXFRAG_POOL_LEN		:= 256
POOL_LEN		:= 6
MFGTESTPOOL_LEN		:= 6
WL_POST			:= 3
WL_POST_SMALL		:= 3
WL_NRXD			:= 512
WL_NTXD			:= 512
WL_NTXD_LFRAG		:= 512
PKT_MAXIMUM_ID		:= 640
WL_RXBND                := 40
WL_RXBND_SMALL		:= 16
WL_POST_CLASSIFIED_FIFO := 4
WL_CLASSIFY_FIFO 	:= 2
COPY_CNT_BYTES		:= 20

EXTRA_DFLAGS	+= -DWL_RXBND=$(WL_RXBND)
EXTRA_DFLAGS    += -DWL_RXBND_SMALL=$(WL_RXBND_SMALL)

EXTRA_DFLAGS	+= -DMAX_TX_STATUS_COMBINED=64 -DPD_NBUF_D2H_TXCPL=4
EXTRA_DFLAGS    += -DMAX_HOST_RXBUFS=256
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=8
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=40
EXTRA_DFLAGS	+= -DDNG_DBGDUMP
# max fetch count at once

PCIEDEV_MAX_PACKETFETCH_COUNT	:= 64

# pcie ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 64
AMPDU_SCB_MAX_RELEASE_AQM       := 24

#perf file
CFILES_PERF += pciedev_data.c

endif # end pcie

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	?= 4347a0

EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

BCMPKTPOOL := 1
FLOPS_SUPPORT := 1

RSDB		:= 1

# PHY related Fixed for 4359b1, should put right value for 4347a0
# common between ROM and ROM-Offload
BCMSROMREV	:= 0x0b
BCMRADIOID	:= 0x45a
BCMRADIOVER	:= 0x0
#BCMRADIOMAJORREV := 0x2
#BCMPHYACMINORREV := 0x3

#
# For bringup, support both radiorevs 4 and 5 in same FW (~9K)
#
ifeq ($(call opt,ipa),1)
	# iPA radio rev for Aux on 2G only
	# Rest is ePA
	BCMRADIOREV     := 0x4
else ifeq ($(call opt,epa),1)
	# ePA radio rev for both Main and Aux
	BCMRADIOREV     := 0x5
endif

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR
EXTRA_DFLAGS	+= -DROBUST_DISASSOC_TX

# DMA Complete WAR - To handle the situation where the
# Doorbell interrupt from Dongle to Host arrives before
# the DMA is complete. If BCMCHKD2HDMA is set by default
# the WAR used is "Modulo-253 SeqNum marker". Enabling
# this method as default for Dingo FW (4359A2)
BCMCHKD2HDMA := 1

TOOLSVER := 2013.11
#this will be enabled for mfgtest
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

EXTRA_DFLAGS += -DBCMPHYCAL_CACHING
EXTRA_DFLAGS += -DDL_NVRAM=9000

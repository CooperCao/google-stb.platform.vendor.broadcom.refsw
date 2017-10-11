# Makefile for hndrte based 4369a0 ROM Offload image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $

# chip specification
CHIP		?= 4369
CHIPROM		?= 4369
ROMREV		:= a0
REV			:= a0
REVID		:= 0
PCIEREVID	:= 24
CCREV		:= 62
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

ROM_LOW	 ?= 0x00000000
ROM_HIGH ?= 0x00170000

# common target attributes
TARGET_HBUS	:= pcie
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
THUMB		:= 1
HBUS_PROTO	:= msgbuf
NODIS		:= 0

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4369a0_dev
WLTUNEFILE	:= wltunable_rte_4369a0.h

# ROM image info
ROMOFFLOAD	:= 1
ROMLDIR		:= $(TOPDIR)/../../../../components/chips/images/roml/$(CHIPROM)$(ROMREV)
ROMLLIB		:= roml.exe

JMPTBL_FULL	:= 1
JMPTBL_TCAM	:= 1
GLOBALIZE	:= 1

# Use Macro definitions for LBUF function
#HNDLBUF_USE_MACROS     := 1
# Use macros instead of bit manipulation functions like setbit, clrbit etc
#BCMUTILS_BIT_MACROS_USE_FUNCS  := 0

PCIE_DMACHANNUM	:= 3

# features (sync with 4369a0.mk, 4369a0-roml.mk)
MEMSIZE		:= 1638400 # 1600KB
MEMBASE		:= 0x170000
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP := 0
DBG_ERROR	:= 1
BCM_ERR_USE_EVENT_LOG := 1
DBG_DFS		:= 1
DBG_CSA		:= 1
DBG_REG		:= 1
DBG_NAN         := 1
DBG_NATOE	:= 1
DBG_WBTEXT	:= 1

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

ASSOC	:= 1
PS	:= 1
ROAM	:= 1
MPC	:= 1
SCAN	:= 1
WSEC	:= 1
MSGMCNX := 1
DBG_AMPDU	:= 1
DBG_AMPDU_NODUMP	:= 1
DBG_AMSDU	:= 1
BCM_HEALTH_CHECK := 1

# TCAM feature
# If changing these values, clean all and rebuild
TCAM		:= 1
TCAM_PCNT	:= 2
TCAM_SIZE	:= 512

# MPU region
MAX_MPU_REGION	:= 12

SRSCAN		:= 1
BCM_LOGTRACE	:= 1
#BCM_LOGTRACE_PCIE := 1
#BCM_EVENT_LOG	:= 1

#PKTIDMAP feature
BCMPKTIDMAP		:= 1

BCMSPLITRX	:= 1
BCMLFRAG	:= 1
BCMFRAGPOOL	:= 1
FRAG_POOL_LEN	:= 32
RXFRAG_POOL_LEN	:= 32

D11TXSTATUS := 1

# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 24
AMPDU_SCB_MAX_RELEASE_AQM       := 16

ifeq ($(call opt,pcie),1)
POOL_LEN_MAX		:= 400
FRAG_POOL_LEN		:= 400
RXFRAG_POOL_LEN		:= 200
POOL_LEN		:= 16
MFGTESTPOOL_LEN		:= 10
WL_POST			:= 128
WL_NRXD			:= 512
WL_NTXD			:= 64
WL_NTXD_LFRAG		:= 1024
PKT_MAXIMUM_ID		:= 640
WL_RXBND                := 64
WL_POST_CLASSIFIED_FIFO := 4
WL_CLASSIFY_FIFO		:= 2
COPY_CNT_BYTES		:= 20
H2D_DMAQ_LEN		:= 512
D2H_DMAQ_LEN		:= 512
PCIE_H2D_NTXD		:= 512
PCIE_H2D_NRXD		:= 512
PCIE_D2H_NTXD		:= 512
PCIE_D2H_NRXD		:= 512
D11TXSTATUS		:= 1
D11RXSTATUS		:= 1

EXTRA_DFLAGS	+= -DWL_RXBND=$(WL_RXBND)

EXTRA_DFLAGS	+= -DMAX_TX_STATUS_COMBINED=128 -DPD_NBUF_D2H_TXCPL=4
EXTRA_DFLAGS    += -DMAX_HOST_RXBUFS=512
EXTRA_DFLAGS    += -DPD_NBUF_H2D_RXPOST=8
EXTRA_DFLAGS	+= -DBCMPCIE_MAX_TX_FLOWS=40
EXTRA_DFLAGS	+= -DPCIE_DEEP_SLEEP
# max fetch count at once

PCIEDEV_MAX_PACKETFETCH_COUNT	:= 64
PCIEDEV_MAX_LOCALBUF_PKT_COUNT	:= 512

# pcie ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE           := 64
AMPDU_SCB_MAX_RELEASE_AQM       := 32

#perf file
CFILES_PERF += pciedev_data.c

endif # end pcie

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	?= 4369a0
CLM_BLOBS	+= 4369a0
WLCLMINC	:= 0

EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

BCMPKTPOOL := 1
FLOPS_SUPPORT := 1

RSDB		:= 1

# Aux tunables
WL_RSDB_AUX_BUFPOST	:= 1
WL_NRXD_AUX		:= 256
WL_POST_AUX		:= 32
WL_RXBND_AUX		:= 16
WL_NTXD_LFRAG_AUX	:= 256

# PHY related Fixed for 4359b1, should put right value for 4347b0
# common between ROM and ROM-Offload
BCMSROMREV	:= 0x0b
BCMRADIOID	:= 0x5FB
BCMRADIOREV     := 0x6
BCMRADIOVER	:= 0x0
#BCMRADIOMAJORREV := 0x2
#BCMPHYACMINORREV := 0x3

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

# Supporting dump rsdb iovar
EXTRA_DFLAGS    += -DDNG_DBGDUMP -DBCMDBG_RSDB

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

LOG_BUF_LEN	:= 8192

BCM_ROM_AUTO_IOCTL_PATCH := 1

# Automatically send Neighbor Report Request after association
WLASSOC_NBR_REQ  := 1
DMATXRC		:= 0
DNG_DBGDMP := 1
PROP_TXSTATUS := 1
RSDB_PM_MODESW := 1
BCM_ECOUNTERS := 0
WL_RSSIREFINE := 1
LPAS := 0
BCM_SPLITBUF := 1
DLL_USE_MACROS := 1

# Enable ROM auto IOCTL/IOVAR patching.
BCM_ROM_AUTO_IOCTL_PATCH := 1

#Manual patching
#WLPATCHFILE    := wlc_patch_4369a0.c
#PATCHSIGN_ENABLED  := 1

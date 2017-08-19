# Makefile for threadx based 43012b0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 43012b0-ram.mk 596263 2015-10-30 09:48:18Z $:

# chip specification
CHIP	:= 43012
ROMREV	:= b0
REV	:= b0
REVID	:= 0
CCREV		:= 56
PMUREV		:= 30
GCIREV		:= 9
AOBENAB		:= 1
OTPWRTYPE	:= 2
BUSCORETYPE	:= SDIOD_CORE_ID


# default targets
TARGETS := \
	threadx-sdio-g

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
TARGET_HBUS	:= sdio
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 1

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43012b0_bu
WLTUNEFILE	:= wltunable_rte_43012b0.h

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

# features (sync with 430120.mk, 43012a0-roml.mk)
MEMBASE		:= 0
MEMSIZE		:= 655360	# Hardcoding it saves ~112 bytes from startarm.S
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT_TRAP := 0
DBG_ERROR	:= 0
WLRXOV		:= 0

PROP_TXSTATUS	:= 0

ifeq ($(call opt,noap),1)
POOL_LEN_MAX	:= 44
POOL_LEN	:= 44
else
POOL_LEN_MAX	:= 35
POOL_LEN	:= 35
endif

WL_POST		:= 8
BUS_POST	:= 18
BUS_NRXD	:= 32
BUS_NTXD	:= 64
BUS_RXBND	:= 18

SDPCMD_RXBUFS	:= 16
SDPCMD_NRXD	:= 32
SDPMCD_NTXD	:= 128
SDPCMD_RXBND	:= 10
SDPCMD_TXGLOM	:= 8

BUS_RXBUFS_MFGC	:= 18
BUS_NRXD_MFGC	:= 32
BUS_NTXD_MFGC	:= 64
MFGTESTPOOL_LEN	:= 50

NO_BCMINTERNAL := 1

# p2p dongle code support
VDEV		:= 1
SDTEST		:= 0

# CLM info
CLM_TYPE	?= 43012b0
CLM_BLOBS	?= 43012b0

# default ampdu rxba wsize
AMPDU_RX_BA_DEF_WSIZE		:= 16
AMPDU_SCB_MAX_RELEASE_AQM	:= 16

EXTRA_DFLAGS	+= -DBCM$(CHIP) -DBCM$(CHIP)$(ROMREV)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=$(AMPDU_RX_BA_DEF_WSIZE)
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=$(AMPDU_SCB_MAX_RELEASE_AQM)
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED
EXTRA_DFLAGS	+= -DUSE_MEMLPLDO

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX
#11n CERT Support
EXTRA_DFLAGS	+= -DWL11N_STBC_RX_ENABLED

BCMPKTPOOL := 1
TINY_PKTJOIN := 1
WL11N_SINGLESTREAM :=1

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

EXTRA_DFLAGS	+= -DBCMMAXSTREAMS=1

# Debug features enable Flag
WL_STATS := 1

# Radio and Phy parameters
BCMRADIOID		?= 0x53a
BCMRADIOREV		?= 38
BCMRADIOVER		?= 0
BCMPHYCORENUM		?= 1
BCMPHYCOREMASK	?= 1
BCMRADIOMAJORREV	?= 2
BCMRADIOMINORREV	?= 0
BCMPHYACMINORREV	?= 1

ifeq ($(call opt,ipa),1)
	# iPA radio rev
	BCMRADIOREV     := 39
	EXTRA_DFLAGS    += -DUSE_5G_PLL_FOR_2G
else
	EXTRA_DFLAGS    += -DWLPHY_EPA_ONLY
endif

EXTRA_DFLAGS += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS += -DBCMRADIOID=$(BCMRADIOID)
EXTRA_DFLAGS += -DBCMPHYCOREMASK=$(BCMPHYCOREMASK)
EXTRA_DFLAGS += -DBCMPHYCORENUM=$(BCMPHYCORENUM)
EXTRA_DFLAGS      += -DBCMRADIOMAJORREV=$(BCMRADIOMAJORREV)
EXTRA_DFLAGS += -DBCMRADIOMINORREV=$(BCMRADIOMINORREV)
EXTRA_DFLAGS += -DBCMPHYACMINORREV=$(BCMPHYACMINORREV)

EXTRA_DFLAGS    += -DMAXSZ_NVRAM_VARS=2048

TOOLSVER = 2013.11

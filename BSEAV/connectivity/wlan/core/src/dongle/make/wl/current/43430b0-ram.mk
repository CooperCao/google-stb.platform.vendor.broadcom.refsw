# Makefile for hndrte based 43430b0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 43430b0-ram.mk eugenep $:

# chip specification
CHIP		:= 43430
ROMREV		:= b0
REV		:= b0
REVID		:= 2
CCREV		:= 49
PMUREV		:= 24
GCIREV		:= 4
AOBENAB		:= 0
OTPWRTYPE	:= 1
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

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43430b0_bu
WLTUNEFILE	:= wltunable_rte_43430b0.h

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $$(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

# features (sync with 4345b0.mk, 4345b0-roml.mk, 4334b0-romlsim-4345b0.mk)
MEMBASE		:= 0
MEMSIZE		:= 524288
SR_MEMSIZE	:= 65536
SR_ASMSIZE	:= 2048
SR_MEM_START    := 458752
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 0
DBG_ERROR	:= 0
WLRXOV		:= 0
SDTEST		:= 0

# Max size of downloaded nvram variables. This is used to ensure that the ATTACH symbols that
# are relocated to the save-restore region will not collide with the host downloaded nvram file.
MAXSZ_NVRAM_VARS	:= 6144

PROP_TXSTATUS := 0

# Memory reduction features:
# - HNDLBUFCOMPACT: Compacts head/end pointers in lbuf to single word
#   To disable set HNDLBUFCOMPACT = 0
# - BCMPKTIDMAP: Suppresses pkt pointers to Ids in lbuf<next,link>, pktpool, etc
#   Must specify max number of packets (various pools + heap)

HNDLBUFCOMPACT := 1
BCMPKTIDMAP     := 1

POOL_LEN_MAX	:= 16
POOL_LEN	:= 16
MFGTESTPOOL_LEN := 16

WL_POST		:= 10
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 32
BUS_RXBND	:= 6
WL_NTXD		:= 32
WL_NRXD		:= 32

# CLM info
CLM_TYPE	:= 43430b0

LCN20PHY	:= 1

# Radio specific defines
#BCMRADIOREV     := 0x3
BCMRADIOVER     := 0x1
BCMRADIOID      := 0x03da

ifeq ($(call opt,mfgtest),1)
	CLM_TYPE	:= 43430b0mfgtest
	DBG_ASSERT_TRAP	:= 0
	WLRXOV		:= 0
endif

#EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)

EXTRA_DFLAGS    += -DWLPHY_IPA_ONLY

# Reduce stack size to increase free heap
HND_STACK_SIZE	:= 3584
EXTRA_DFLAGS	+= -DHND_STACK_SIZE=$(HND_STACK_SIZE)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16 -DBCMPKTPOOL_ENABLED

# Do not enable the Event pool for 43430 as there is no space
EXTRA_DFLAGS	+= -DEVPOOL_SIZE=0

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

# To limit PHY core checks
#EXTRA_DFLAGS	+= -DBCMPHYCOREMASK=1
#EXTRA_DFLAGS	+= -DTINY_PKTJOIN
#EXTRA_DFLAGS	+= -DWL_RXEARLYRC
#EXTRA_DFLAGS	+= -DWLRXOVERTHRUSTER

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR
#GPIO based TX inhibit
EXTRA_DFLAGS    += -DGPIO_TXINHIBIT

#WLOVERTHRUSTER := 1
#WLRXOVERTHRUSTER := 1
#WL_RXEARLYRC := 1
#TINY_PKTJOIN := 1

TOOLSVER := 2013.11
NOFNRENAME := 1

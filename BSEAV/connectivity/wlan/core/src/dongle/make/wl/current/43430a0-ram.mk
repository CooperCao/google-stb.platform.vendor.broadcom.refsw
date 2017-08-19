# Makefile for hndrte based 43430a0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 43430a0-ram.mk eugenep $:

# chip specification
CHIP	:= 43430
ROMREV	:= a0
REV	:= a
REVID	:= 0

# default targets
TARGETS := \
	sdio-g

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
TARGET_HBUS	:= sdio
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 1

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43430a0_bu
WLTUNEFILE	:= wltunable_rte_43430a0.h

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $$(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

# features (sync with 4345b0.mk, 4345b0-roml.mk, 4334a0-romlsim-4345b0.mk)
MEMBASE		:= 0
MEMSIZE		:= 524288
SR_MEMSIZE	:= 65536
SR_ASMSIZE	:= 2048
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
WLRXOV		:= 1
SDTEST		:= 0

PROP_TXSTATUS := 1

# Memory reduction features:
# - HNDLBUFCOMPACT: Compacts head/end pointers in lbuf to single word
#   To disable set HNDLBUFCOMPACT = 0
# - BCMPKTIDMAP: Suppresses pkt pointers to Ids in lbuf<next,link>, pktpool, etc
#   Must specify max number of packets (various pools + heap)

HNDLBUFCOMPACT := 1
BCMPKTIDMAP     := 1

POOL_LEN_MAX	:= 20
POOL_LEN	:= 20
MFGTESTPOOL_LEN := 20

WL_POST		:= 10
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 32
BUS_RXBND	:= 6
WL_NTXD		:= 64
WL_NRXD		:= 32

# CLM info
CLM_TYPE	:= 43430a0

LCN20PHY	:= 1

# Radio specific defines
BCMRADIOREV     := 0x2
BCMRADIOVER     := 0x1
BCMRADIOID      := 0x03da

EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)

EXTRA_DFLAGS    += -DWLPHY_IPA_ONLY


EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16 -DBCMPKTPOOL_ENABLED

# Save memory; wep & tkhash slightly slower
EXTRA_DFLAGS    += -DSHARE_RIJNDAEL_SBOX

# To limit PHY core checks
EXTRA_DFLAGS	+= -DBCMPHYCOREMASK=1

EXTRA_DFLAGS	+= -DTINY_PKTJOIN
EXTRA_DFLAGS	+= -DWL_RXEARLYRC
EXTRA_DFLAGS	+= -DWLRXOVERTHRUSTER

# WAR for beacon lost issue
#EXTRA_DFLAGS	+= -DBCN_LOST_NDP_WAR_43430

#FLOPS_SUPPORT := 1

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
#Enable GPIO
EXTRA_DFLAGS	+= -DWLGPIOHLR
WLOVERTHRUSTER := 1
WLRXOVERTHRUSTER := 1
WL_RXEARLYRC := 1
TINY_PKTJOIN := 1

TOOLSVER := 2011.09
NOFNRENAME := 1

# LDPC WAR, set "jtag_lcn20_clk320_byp_clken_sel" bit
# in JTAG User Register 1 via Jtag master.
EXTRA_DFLAGS	+= -DBCM_43430A0_LDPC_WAR

# This is for ROM compatibility. 11H was excluded from the ROM.
EXTRA_DFLAGS    += -DROM_11H_MISMATCH_43430A0

# Makefile for hndrte based 4354a1 full ram Image
#
# Copyright (C) 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: 4354a1-ram.mk 436982 2014-11-15 21:44:52Z $

# chip specification
CHIP	:= 4354
ROMREV	:= a1
REVID	:= 1

# To build sdio-reclaim (save ~8-10K):
# 1) Abandon these
#   wl_up
#   wl_down
#   wl_init
#   _wl_init
#   wlc_radio_disable
#   wlc_radio_enable
#   wlc_radio_active
#   wlc_mpc_nodown
#   wlc_ioctl_handle_up
# 2) Comment out #error in wlc_plt.c (Need fix)
#

# default targets
TARGETS := \
	sdio-ag-mfgtest-seqcmds-ndis	\
	usb-ag

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio usb pcie
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 0
BCM_DNGL_BL_PID := 0xbd23

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4354a1_bu
WLTUNEFILE	:= wltunable_rte_4354a1.h

TEXT_START := 0x180000
#DATA_START := 0x180900

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

ifeq (usb,$(findstring usb, $(REMAIN)))
BOOTLOADER_PATCH_SIZE := 0x800
TEXT_START := $(shell awk 'BEGIN { printf "0x%x\n", $(TEXT_START) + $(BOOTLOADER_PATCH_SIZE) }')
endif

# features
MEMSIZE		:= 786432	# Hardcoding it saves ~112 bytes from startarm.S
MEMBASE		:= 0x180000
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
WLRXOV		:= 0
SDTEST		:= 0

# Memory reduction features:
# - HNDLBUFCOMPACT: Compacts head/end pointers in lbuf to single word
#   To disable set HNDLBUFCOMPACT = 0
# - BCMPKTIDMAP: Suppresses pkt pointers to Ids in lbuf<next,link>, pktpool, etc
#   Must specify max number of packets (various pools + heap)

#HNDLBUFCOMPACT := 0
#BCMPKTIDMAP     := 1
#PKT_MAXIMUM_ID  := 400

POOL_LEN_MAX	:= 50
POOL_LEN	:= 30
WL_POST		:= 12
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 32
BUS_RXBND	:= 6

MFGTESTPOOL_LEN	:= 30
BUS_RXBUFS_MFGC	:= 6
BUS_NRXD_MFGC	:= 32
BUS_NTXD_MFGC	:= 32

SDPCMD_RXBUFS	:= 10
SDPCMD_NRXD	:= 32
SDPCMD_NTXD	:= 32
SDPCMD_RXBND	:= 10
SDPCMD_TXGLOM	:= 8

ifeq ($(call opt,mfgtest),1)
POOL_LEN_MAX    := 50
endif

#NO_BCMINTERNAL := 1

# p2p dongle code support
VDEV		:= 1

ifeq ($(call opt,pcie),1)
BCMPKTPOOL	:= 1
POOL_LEN_MAX    := 200
POOL_LEN        := 6
MFGTESTPOOL_LEN := 6
WL_POST		:= 40

FRAG_POOL_LEN   := 200
RXFRAG_POOL_LEN   := 120

#dongle builds need BCMPKDITDMAP
HNDLBUFCOMPACT := 1
BCMPKTIDMAP     := 1
PKT_MAXIMUM_ID	:= 640

BCMPKTIDMAP_ROM_COMPAT := 1
AMPDU_RX_BA_DEF_WSIZE := 48

ifeq ($(call opt,splitrx),1)
POOL_LEN       := 6
RXFRAG_POOL_LEN := 120
WL_NRXD        := 256
WL_NTXD        := 128
WL_POST        := 40
WL_POST_FIFO1  := 2
endif

# For perf at cost of ~7K
BCMUTILS_BIT_MACROS_USE_FUNCS	:= 0

# perf files
CFILES_PERF    += pciedev.c pciedev_rte.c bcmmsgbuf.c

ifeq ($(call opt,threadx),1)
# perf files
CFILES_PERF    += threadx.c
ifeq ($(call opt,assert),1)
POOL_LEN_MAX    := 160
WL_NRXD         := 128
WL_NTXD         := 64
endif #assert
endif #threadx
ifeq ($(call opt,mfgtest),1)
EXTRA_DFLAGS    += -DDELAYED_PCIE_REPROGRAMMING -DPCIE_DMAXFER_LOOPBACK
endif
endif #pcie

# CLM info
CLM_TYPE	:= 4354a1

BCMRADIOREV	:= 0x27
BCMRADIOVER	:= 0x0
BCMRADIOID	:= 0x2069
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)
EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)

# Makeconf defines BCM43xx based on $(CHIP)$(ROMREV), but we also need B1
EXTRA_DFLAGS	+= -DBCM4354 -DBCM4354A1


ifeq ($(call opt,dbgtput),1)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=24
else
ifeq ($(call opt,pcie),1)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=64
else
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16
endif
endif

EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=16
EXTRA_DFLAGS	+= -DPKTC_DONGLE

ifeq ($(call opt,usb),1)
EXTRA_DFLAGS    += -DUSB_XDCI
endif

ifeq ($(call opt,sdio),1)
ifeq ($(call opt,sr),1)
	# Default -srfast for -sr for sdio only
	# Need to verify pcie is stable with overnight test
	# making it default
	SRFAST	:= 1
endif
endif

ifeq ($(call opt,mfgtest),1)
EXTRA_DFLAGS    += -DBCMNVRAMW
endif

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0
FLOPS_SUPPORT := 1

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2011.09


ifeq ($(call opt,dbgtput),1)
POOL_LEN_MAX    := 120
POOL_LEN        := 100
WL_POST		:= 24
WL_NRXD		:= 128
WL_NTXD		:= 128
SDPCMD_RXBUFS	:= 40
SDPCMD_NRXD	:= 128
SDPCMD_NTXD	:= 128
SDPCMD_RXBND	:= 24

EXTRA_DFLAGS	+= -DDEFMAXTXPKTGLOM=32
EXTRA_DFLAGS	+= -DDEFTXLAZYDELAY=2
EXTRA_DFLAGS	+= -DDEFMAXTXPDU=16
EXTRA_DFLAGS	+= -DBCMDBG_TPUT
endif

# PM single core beacon receive
WLPM_BCNRX	:= 1
PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

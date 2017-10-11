# Makefile for hndrte based 4350c0 full ram Image
#
# Copyright (C) 2012, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id$

# chip specification
CHIP	:= 4350
ROMREV	:= c0
REVID	:= 3

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
	usb-ag

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio usb pcie
THUMB		:= 1
HBUS_PROTO 	:= cdc
NODIS		:= 0
BCM_DNGL_BL_PID := 0xbd23

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4350c0_bu
WLTUNEFILE	:= wltunable_rte_4350c0.h

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

# features (sync with 4350a0.mk, 4350a0-roml.mk, 4334a0-romlsim-4350a0.mk)
MEMBASE     := 0x180000
MEMSIZE		:= 786432	# Hardcoding it saves ~112 bytes from startarm.S
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

HNDLBUFCOMPACT := 1
BCMPKTIDMAP     := 1

POOL_LEN_MAX	:= 50
POOL_LEN	:= 30
WL_POST		:= 12
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 32
BUS_RXBND	:= 6

ifeq ($(call opt,pcie),1)
POOL_LEN_MAX    := 80
FRAG_POOL_LEN   := 100
POOL_LEN        := 6
WL_POST_FIFO1   := 2
MFGTESTPOOL_LEN := 20
WLCONFFILE      := wlconfig_rte_4350b1_pcie
RXFRAG_POOL_LEN	:= 60
WL_POST		:= 16
WL_NRXD		:= 128
WL_NTXD		:= 256
ifeq ($(call opt,splitrx),1)
WL_POST		:= 40
WL_NRXD		:= 256
WL_SPLITRX_MODE	:= 2
WL_CLASSIFY_FIFO := 1
endif
endif
ifeq ($(call opt,mfgtest),1)
POOL_LEN_MAX    := 50
endif
#NO_BCMINTERNAL := 1

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	:= 4350c0

BCMRADIOID      := 0x2069
BCMRADIOREV     := 0x24
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)
EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)

# Makeconf defines BCM43xx based on $(CHIP)$(ROMREV), but we also need B1
EXTRA_DFLAGS	+= -DBCM4350 -DBCM4350c0


ifeq ($(call opt,pcie),1)
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=64
else
EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16
endif

EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=16
EXTRA_DFLAGS    += -DWLOVERTHRUSTER
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
EXTRA_DFLAGS += -DPCIE_DELAYED_HOSTWAKE -DPCIE_DMAXFER_LOOPBACK
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
POOL_LEN        := 85
WL_POST		:= 24
WL_NRXD		:= 128
WL_NTXD		:= 128
SDPCMD_RXBUFS	:= 40
SDPCMD_NRXD	:= 128
SDPCMD_NTXD	:= 128
SDPCMD_RXBND	:= 24

EXTRA_DFLAGS	+= -DDEFMAXTXPKTGLOM=24

EXTRA_DFLAGS	+= -DBCMDBG_TPUT
endif

# PM single core beacon receive
WLPM_BCNRX	:= 1

AMPDU_HOSTREORDER := 0
AMPDU_HOSTREORDER_DISABLED := 1

EXTRA_DFLAGS += -DPCIE_DMAXFER_LOOPBACK

# Offsets for the PCIE mem2mem DMA
EXTRA_DFLAGS    += -DH2D_PD_RX_OFFSET=8
EXTRA_DFLAGS    += -DD2H_PD_RX_OFFSET=8

PCIEDEV_MAX_IOCTLRSP_BUF_SIZE := 8192

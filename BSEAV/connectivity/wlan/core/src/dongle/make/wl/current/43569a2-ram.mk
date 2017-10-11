# Makefile for hndrte based 43569a0 full ram Image
#
# Copyright (C) 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

# chip specification
CHIP	:= 43569
ROMREV	:= a2
REVID	:= 2


# default targets
TARGETS := \
	usb-ag \
	usb-ag-assert-pool \
        usb-ag-assert-pool-tdls \
	usb-ag-assert-pool-p2p-mchan

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= usb
THUMB		:= 1
HBUS_PROTO 	:= cdc
NODIS		:= 0
BCM_DNGL_BL_PID := 0xbd27

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43569a2_bu
WLTUNEFILE	:= wltunable_rte_43569a2.h

TEXT_START := 0x180800
DATA_START := 0x180900
BOOTLOADER_PATCH_SIZE := 0x800

# features (sync with 43569a2.mk, 43569a2-roml.mk)
MEMBASE		:= 0x180000
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

HNDLBUFCOMPACT	:= 1
BCMPKTIDMAP     := 1

POOL_LEN_MAX	:= 50
POOL_LEN	:= 30
HND_STACK_SIZE	:= 4608
WL_POST		:= 12
BUS_POST	:= 6
BUS_NRXD	:= 32
BUS_NTXD	:= 32
BUS_RXBND	:= 6


# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	:= 43569a2
STA_KEEP_ALIVE  := 1

BCMRADIOID	:= 0x2069
BCMRADIOREV	:= 0x2C
BCMRADIOVER	:= 0x0
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)
EXTRA_DFLAGS	+= -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCM_BMAC_VARS_APPEND

EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=16
EXTRA_DFLAGS	+= -DWLOVERTHRUSTER
EXTRA_DFLAGS	+= -DPKTC_DONGLE
EXTRA_DFLAGS	+= -DUSB_XDCI

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0
FLOPS_SUPPORT	:= 1

# if MAXASSOC_LIMIT is set, use it for maxassoc; otherwise use MAXSCB
MAXASSOC_LIMIT	:= 10

EXTRA_DFLAGS	+= -DHND_STACK_SIZE=$(HND_STACK_SIZE)

# Enable compiler option for inlining of simple functions for targets that run almost entirely from
# RAM. Inlining actually saves memory due to the elimination of function call overhead and more
# efficient register usage.
NOINLINE	:=

# Use the 2013.11 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER	= 2013.11

# Enable compiler optimizations that might rename functions in order to save memory.
NOFNRENAME	:= 0

# Use functions instead of macros for isset/isclr/setbit/clrbit to save memory.
EXTRA_DFLAGS	+= -DBCMUTILS_BIT_MACROS_USE_FUNCS

# Disabling RADAR and SROM8 code for 43569
EXTRA_DFLAGS	+= -DWLC_DISABLE_DFS_RADAR_SUPPORT
#EXTRA_DFLAGS	+= -DWLC_DISABLE_ACI #doesn't work because of spur war (BFL2_2G_SPUR_WAR)
EXTRA_DFLAGS	+= -DWLC_DISABLE_SROM8

# Making PHY_IPA compile time option
EXTRA_DFLAGS	+= -DWLPHY_IPA_ONLY

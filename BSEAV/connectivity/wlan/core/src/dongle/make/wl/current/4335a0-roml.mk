# Makefile for hndrte based 4335a0 ROM Offload image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4335a0-roml.mk 279687 2011-08-25 07:08:08Z $:

# chip specification
CHIP	:= 4335
ROMREV	:= a0
REV	:= a0
REVID	:= 0
# default targets
TARGETS := \
	sdio-ag-autoabn \
	sdio-ag-pool-idsup-idauth-p2p-pno-aoe-toe-keepalive-autoabn \
	sdio-ag-ccx-p2p-idsup-idauth-pno-autoabn \
	sdio-ag-mfgtest-seqcmds-autoabn \
	sdio-ag-mfgtest-seqcmds-ndis-autoabn \
	sdio-ag-pool-ndis-reclaim-autoabn \
	sdio-ag-cdc-ndis-reclaim-autoabn \
	sdio-ag-pool-idsup-autoabn \
	usb-ag-autoabn \
	usb-ag-pool-idsup-idauth-p2p-pno-aoe-toe-keepalive-autoabn \
	usb-ag-ccx-p2p-idsup-idauth-pno-autoabn \
	usb-ag-idsup-nodis-autoabn \
	usb-ag-mfgtest-seqcmds-autoabn \
	usb-ag-mfgtest-autoabn


TEXT_START := 0x00180000
DATA_START := 0x00180900
BOOTLOADER_PATCH_SIZE := 0x800

ROM_LOW	 ?= 0x00000000
ROM_HIGH ?= 0x00090000

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb)
ifeq ($(REMAIN),)
$(error $(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

ifeq (usb,$(findstring usb, $(REMAIN)))
TEXT_START := $(shell awk 'BEGIN { printf "0x%x\n", $(TEXT_START) + $(BOOTLOADER_PATCH_SIZE) }')
endif

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio usb
THUMB		:= 1
HBUS_PROTO	:= cdc
NODIS		:= 1
BCM_DNGL_BL_PID := 0xbd20

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4335a0_dev
WLTUNEFILE	:= wltunable_rte_4335a0.h

# ROM image info
ROMOFFLOAD	:= 1
ROMLLIB         := roml.exe
JMPTBL_FULL	:= 1
GLOBALIZE	:= 1
#WLPATCHFILE	:= wlc_patch_4335a0.c

# features (sync with 4324a0.mk, 4324a0-roml.mk, 4334a0-romlsim-4324a0.mk)
MEMBASE		:= 0x00180000
MEMSIZE		:= 786432
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP := 1
DBG_ERROR	:= 0
WLRXOV		:= 1
SDTEST		:= 0

PROP_TXSTATUS := 1
TINY_PKTJOIN := 1
WL_RXEARLYRC := 1

POOL_LEN_MAX	:= 75
POOL_LEN	:= 75
WL_POST		:= 12
BUS_POST	:= 18
BUS_NRXD 	:= 32
BUS_NTXD 	:= 64
BUS_RXBND 	:= 18

SDPCMD_RXBUFS	:= 16
SDPCMD_NRXD	:= 32
SDPCMD_NTXD	:= 128
SDPCMD_RXBND	:= 10
SDPCMD_TXGLOM	:= 8

BUS_RXBUFS_MFGC := 18
BUS_NRXD_MFGC  := 32
BUS_NTXD_MFGC := 64

MFGTESTPOOL_LEN := 50

# mini packet Pool
ifeq ($(call opt,minpktpool),1)
	POOL_LEN_MAX	:= 28
	POOL_LEN	:= 28
	WL_POST		:= 6
	BUS_POST	:= 8
	BUS_NRXD	:= 16
	BUS_NTXD	:= 32
	BUS_RXBND	:= 8

	SDPCMD_RXBUFS	:= 8
	SDPCMD_NRXD	:= 16
	SDPMCD_NTXD	:= 32
	SDPCMD_RXBND	:= 8

	EXTRA_DFLAGS += -DMINPKTPOOL
endif

# TCAM feature
# If changing these values, clean all and rebuild
#TCAM		:= 1
#TCAM_PCNT	:= 1
#TCAM_SIZE	:= 256

# CLM info
CLM_TYPE	:= 4335a0


EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=32 -DBCMPKTPOOL_ENABLED
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=8
EXTRA_DFLAGS	+= -DDEFTXLAZYDELAY=1
FLOPS_SUPPORT := 1

EXTRA_DFLAGS += -DWLOVERTHRUSTER

EXTRA_DFLAGS +=  -DWLFCHOST_TRANSACTION_ID

EXTRA_DFLAGS += -DPKTC_DONGLE

#enable MCHAN preclose
ifeq ($(call opt,pclose),1)
EXTRA_DFLAGS += -DWLMCHANPRECLOSE
endif
# Enable HW_WAPI

EXTRA_DFLAGS += -DHW_WAPI

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT
#Enable GPIO
#EXTRA_DFLAGS	+= -DWLGPIOHLR

# Share lookup table.
#EXTRA_DFLAGS	+= -DSHARE_RIJNDAEL_SBOX	# Saves memory; wep & tkhash slightly slower

EXTRA_DFLAGS	+= -DBCM4335 -DBCM4335a0
EXTRA_DFLAGS	+= -DBCMUSBDEV_BULKIN_2EP

EXTRA_DFLAGS	+= -DACPHY_1X1_37P4
EXTRA_DFLAGS	+= -DWLPHY_EPA_ONLY

EXTRA_DFLAGS   += -DBCMPHYCOREMASK=1

# For ROM compatibility
EXTRA_DFLAGS   += -DOLD_TDLS_INFO_TYPE
EXTRA_DFLAGS   += -DOLD_RCVEC_SIZE  -DOLD_ROM_WNM_INFO

# robust disassoc tx to VSDB GO
#EXTRA_DFLAGS += -DROBUST_DISASSOC_TX

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2011.09
NOFNRENAME := 1
VDEV	:= 1

BCMRADIOREV     := 0x10
EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)

# Work-arounds for ROM compatibility to handle ROM that excludes MFP support
WLC_MFP_ROM_COMPAT := 1

# Disable ROM auto IOCTL/IOVAR patching.
BCM_ROM_AUTO_IOCTL_PATCH := 0

# Debug features enable Flag
WL_STATS := 1

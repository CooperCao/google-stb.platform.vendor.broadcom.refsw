# Makefile for hndrte based 4345a0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4335a0-ram.mk 266614 2011-06-14 22:16:03Z $:

# chip specification
CHIP	:= 4345
ROMREV	:= b1
REV	:= b
REVID	:= 5

# default targets
TARGETS := \
	sdio-ag-pool \
	pcie-ag-msgbuf-pool \

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
TARGET_HBUS	:= sdio usb pcie
THUMB		:= 1
HBUS_PROTO 	:= cdc
NODIS		:= 1
BCM_DNGL_BL_PID := 0xbd24

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4345b1_bu
WLTUNEFILE	:= wltunable_rte_4345b1.h

TEXT_START := 0x001b0000
#DATA_START := 0x001b0900

# REMAIN is exported from rte/wl/Makefile. this contains the string specifying the bus (sdio/usb/pcie)
ifeq ($(REMAIN),)
$(error $$(REMAIN) is undefined)
endif

REMAIN := $(subst /,,$(REMAIN))

ifeq (usb,$(findstring usb, $(REMAIN)))
BOOTLOADER_PATCH_SIZE := 0x800
TEXT_START := $(shell awk 'BEGIN { printf "0x%x\n", $(TEXT_START) + $(BOOTLOADER_PATCH_SIZE) }')
endif

MEMBASE		:= 0x001b0000
MEMSIZE		:= 589824	# Hardcoding it saves ~112 bytes from startarm.S
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT_TRAP := 1
WLRXOV		:= 1
TINY_PKTJOIN := 1
WL_RXEARLYRC := 1

POOL_LEN_MAX	:= 60
#POOL_LEN	:= 40
POOL_LEN	:= 20
ifeq ($(call opt,pcie),1)
FRAG_POOL_LEN   := 60
POOL_LEN_MAX	:= 120
POOL_LEN        := 6
WL_POST_FIFO1   := 2
RXFRAG_POOL_LEN   := 30
endif
MFGTESTPOOL_LEN := 50
WL_POST		:= 12
BUS_POST	:= 18
BUS_NRXD 	:= 32
BUS_NTXD 	:= 40
BUS_RXBND 	:= 18

SDPCMD_RXBUFS	:= 16
SDPCMD_NRXD	:= 32
SDPCMD_NTXD	:= 128
SDPCMD_RXBND	:= 10
SDPCMD_TXGLOM	:= 8

BUS_RXBUFS_MFGC := 18
BUS_NRXD_MFGC  := 32
BUS_NTXD_MFGC := 64

#NO_BCMINTERNAL := 1

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	:= 4345b1

EXTRA_DFLAGS	+= -DBCM4345 -DBCM4345B1
EXTRA_DFLAGS	+= -DBCMUSBDEV_BULKIN_2EP


#EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=32 -DBCMPKTPOOL_ENABLED
EXTRA_DFLAGS	+= -DBCMPKTPOOL_ENABLED
EXTRA_DFLAGS	+= -DAMPDU_SCB_MAX_RELEASE_AQM=8

BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0
FLOPS_SUPPORT := 1
EXTRA_DFLAGS	+= -DBCMTRXV2
EXTRA_DFLAGS	+= -DACPHY_1X1_37P4

EXTRA_DFLAGS   += -DBCMPHYCORENUM=1
EXTRA_DFLAGS   += -DBCMPHYCOREMASK=1

# Use the 2011.09 version of the toolchain. It provides significant memory savings
# relative to older toolchains.
TOOLSVER = 2011.09
ifeq ($(call opt,wlbga),1)
BCMRADIOREV     := 71
else	# fcbga
BCMRADIOREV     := 70
endif	# wlgba
BCMRADIOVER     := 0x1
BCMRADIOID      := 0x030b

EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS	+= -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS	+= -DBCMRADIOID=$(BCMRADIOID)

# RxOffsets for the PCIE mem2mem DMA
EXTRA_DFLAGS    += -DH2D_PD_RX_OFFSET=8
EXTRA_DFLAGS    += -DD2H_PD_RX_OFFSET=8

BCMRADIOMAJORREV  := 3
EXTRA_DFLAGS      += -DBCMRADIOMAJORREV=$(BCMRADIOMAJORREV)

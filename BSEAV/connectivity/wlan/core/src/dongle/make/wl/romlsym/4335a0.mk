# Makefile for hndrte based 4335 standalone programs,
#	to generate romtable.S for 4335a0 ROM builds
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4335a0.mk 266614 2011-06-14 22:16:03Z $:

# chip specification
CHIP		:= 4335
REV		:= a0
REVID		:= 0

DEVSIM_CHIP	:= 4334
DEVSIM_CHIP_REV	:= a0

TARGETS		:= sdio-usb

# common target attributes
TARGET_HBUS	:= sdio usb
TARGET_ARCH	:= arm
TARGET_CPU	:= cr4
THUMB		:= 1
HBUS_PROTO 	:= cdc
BAND		:= ag

# wlconfig & wltunable
WLCONFFILE	:= wlconfig_rte_4335a0_dev


# ROMCTL needed for location of romctl.txt
ROMCTL		:= $(TOPDIR)/../roml/$(CHIP)$(REV)/romctl.txt
ifeq ($(DEVSIM_BUILD),1)
#	WLTUNEFILE	:= wltunable_rte_$(DEVSIM_CHIP)$(DEVSIM_CHIP_REV).h
	ROMCTL		:= $(TOPDIR)/../roml/$(DEVSIM_CHIP)sim-$(CHIP)$(REV)/romctl.txt
	CHIP		:= $(DEVSIM_CHIP)
	DBG_ASSERT      := 1
else

WLTUNEFILE	:= wltunable_rte_4335a0.h
DBG_ASSERT      := 0
endif

# features (sync with 4335a0.mk, 4335a0-roml.mk)
MEMSIZE		:= 786432
MFGTEST		:= 1
WLTINYDUMP	:= 0

DBG_ERROR	:= 1
WLRXOV		:= 1
POOL_LEN_MAX	:= 60
SDTEST		:= 0

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	:= 4335

# extra flags
EXTRA_DFLAGS	+= -DSHARE_RIJNDAEL_SBOX	# Save 1400 bytes; wep & tkhash slightly slower
EXTRA_DFLAGS	+= -DWLAMSDU_TX -DAMPDU_RX_BA_DEF_WSIZE=16
EXTRA_DFLAGS	+= -DBCMUSBDEV_BULKIN_2EP	# Support for 2 Bulk-IN endpoints

ifneq ($(ROMLIB),1)
ROMBUILD	:= 1
EXTRA_DFLAGS	+= -DBCMROMSYMGEN_BUILD
endif

# Makefile for hndrte based 4350a0 full ram Image
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4350a0-ram.mk 266614 2011-06-14 22:16:03Z $:

# chip specification
CHIP	:= 4350
ROMREV	:= a0
REVID	:= 0

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
TARGET_HBUS	:= sdio usb
THUMB		:= 1
HBUS_PROTO 	:= cdc
NODIS		:= 0
BCM_DNGL_BL_PID := 0xbd23

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4350a0_bu
WLTUNEFILE	:= wltunable_rte_4350a0.h

TOOLSVER	:= 2011.09
NOFNRENAME	:= 1

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

# features (sync with 4324b0.mk, 4324b0-roml.mk, 4334a0-romlsim-4324b0.mk)
MEMBASE		:= 0x180000
MEMSIZE		:= 786432	# Hardcoding it saves ~112 bytes from startarm.S
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT	:= 0
DBG_ASSERT_TRAP	:= 1
DBG_ERROR	:= 0
WLRXOV		:= 0

POOL_LEN_MAX	:= 60
POOL_LEN	:= 30
MFGTESTPOOL_LEN := 30
WL_POST		:= 6
BUS_POST	:= 6

#NO_BCMINTERNAL := 1

# p2p dongle code support
VDEV		:= 1

# CLM info
CLM_TYPE	:= 4350a0

# Makeconf defines BCM4324 and BCM4324B0 based on $(CHIP)$(ROMREV), but we also need B0
EXTRA_DFLAGS	+= -DBCM4350 -DBCM4350a0


EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

BIN_TRX_OPTIONS_SUFFIX := -x 0x0 -x 0x0 -x 0x0
FLOPS_SUPPORT := 1

BCMRADIOREV     := 0x22
BCMRADIOVER     := 0x0
BCMRADIOID      := 0x2069

EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)

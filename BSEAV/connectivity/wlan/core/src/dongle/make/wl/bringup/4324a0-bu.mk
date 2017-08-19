# Makefile for hndrte based 4324 BU image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 4324a0-bu.mk 251854 2011-04-07 20:21:20Z $:

# chip specification
CHIP	:= 4324
ROMREV	:= a0
REVID	:= 0

TOOLSVER := 2010.09
TOOLSCCFLAGS	:= -fno-short-enums -Wno-strict-aliasing

# default targets
TARGETS := \
	sdio-ag-pool \
	sdio-ag-qt-pool \
	sdio-ag-mfgtest-seqcmds \
	sdio-ag-pool-ccx-p2p-proptxstatus \
	usb-ag-pool \
	usb-ag-qt-pool \
	usb-ag-mfgtest-seqcmds \
	usb-ag-pool-ccx-p2p-proptxstatus

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
TARGET_HBUS	:= sdio usb
THUMB		:= 1
HBUS_PROTO	:= cdc

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_4324a0_bu
WLTUNEFILE	:= wltunable_rte_4324a0.h

# ROM image info
ROMOFFLOAD	:= 0
#ROMLDIR		:= $(TOPDIR)/../../images/roml/$(CHIP)$(ROMREV)
#ROMESC		:= $(ROMLDIR)/romesc.txt
#ROMLLIB         := roml.exe
#ROMLMAP         := roml.map
JMPTBL_TCAM	:= 1
#JMPTBL_FULL	:= 1
ifeq ($(JMPTBL_FULL),1)
JMPTBL		:= 1
ROMDATBL	:= 1
endif
GLOBALIZE	:= 1
# WLPATCHFILE	:= wlc_patch_4324a0.c

# features (sync with 4324a0.mk, 4324a0-roml.mk, 4330a0-romlsim-4324a0.mk)
SMALL		:= 0
MEMSIZE		:= 557056	# Hardcoding it saves ~112 bytes from startarm.S
FLASH		:= 0
MFGTEST		:= 0
WLTINYDUMP	:= 0
DBG_ASSERT	:= 1
DBG_ERROR	:= 1
BCMPKTPOOL	:= 1
DMATXRC		:= 1
WLRXOV		:= 1
PROP_TXSTATUS	:= 1
NO_BCMINTERNAL	:= 1
POOL_LEN_MAX	:= 60
POOL_LEN	:= 30

# TCAM feature
# If changing these values, clean all and rebuild
TCAM		:= 0
TCAM_PCNT	:= 1
TCAM_SIZE	:= 256

# CLM info
CLM_TYPE	:= 4324a0

# extra flags
EXTRA_DFLAGS += -I$(TOPDIR)/../../images/roml/$(CHIP)$(ROMREV)

EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

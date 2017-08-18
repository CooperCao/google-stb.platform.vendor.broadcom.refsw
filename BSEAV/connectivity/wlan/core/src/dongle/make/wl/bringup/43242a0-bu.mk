# Makefile for hndrte based 43242 BU image building
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: 43242a0-bu.mk 251854 2011-04-07 20:21:20Z $:

# chip specification
CHIP	:= 43242
ROMREV	:= a0
REVID	:= 0

# default targets
TARGETS := \
	usb-ag \
	usb-ag-ccx-p2p \
	usb-ag-mfgtest-seqcmds \
	usb-ag-qt \

# common target attributes
TARGET_ARCH	:= arm
TARGET_CPU	:= cm3
TARGET_HBUS	:= usb
THUMB		:= 1
HBUS_PROTO	:= cdc

CONFFILE = wlconfig
TUNEFILE = wltunable

# wlconfig & wltunable for rest of the build targets
WLCONFFILE	:= wlconfig_rte_43242a0_dev
WLTUNEFILE	:= wltunable_rte_43242a0.h

BINDL = $(BIN_TRX)

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
# WLPATCHFILE	:= wlc_patch_43242a0.c

# features (sync with 43242a0.mk, 43242a0-roml.mk, 4334a0-romlsim-43242a0.mk)
MEMSIZE		:= 589824	# Hardcoding it saves ~112 bytes from startarm.S
WLTINYDUMP	:= 0
DBG_ASSERT	:= 1
DBG_ERROR	:= 1
WLRXOV		:= 1

POOL_LEN_MAX	:= 60
POOL_LEN	:= 30

# TCAM feature
# If changing these values, clean all and rebuild
TCAM		:= 0
TCAM_PCNT	:= 0
TCAM_SIZE	:= 256

# CLM info
CLM_TYPE	:= 43242a0

# extra flags
EXTRA_DFLAGS += -I$(TOPDIR)/../../images/roml/$(CHIP)$(ROMREV)

EXTRA_DFLAGS	+= -DAMPDU_RX_BA_DEF_WSIZE=16

# Enable load average printing (CM3)
#EXTRA_DFLAGS	+= -DBCMDBG_LOADAVG -DBCMDBG_FORCEHT

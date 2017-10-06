# Makefile for hndrte based 4335b0 ROM Offload image building
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
#

include $(TOPDIR)/current/4335a0-roml.mk

REVID		:= 1
WLTUNEFILE	:= wltunable_rte_4335b0.h
BCMRADIOREV     := 0x17
BCMRADIOVER     := 0x0
BCMRADIOID      := 0x2069

# For ROM compatibility
EXTRA_DFLAGS    += -DBCMRADIOREV=$(BCMRADIOREV)
EXTRA_DFLAGS    += -DBCMRADIOVER=$(BCMRADIOVER)
EXTRA_DFLAGS    += -DBCMRADIOID=$(BCMRADIOID)
EXTRA_DFLAGS    += -DLOW_TX_CURRENT_SETTINGS_2G
WL_DUMPCCA	:= 1
# For CPU cycle count
#EXTRA_DFLAGS    += -DBCMDBG_FORCEHT -DBCMDBG_CPU

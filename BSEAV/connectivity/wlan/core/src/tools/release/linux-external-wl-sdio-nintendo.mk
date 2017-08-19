#
# Linux External wl build/release Makefile
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

PARENT_MAKEFILE := linux-wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND         ?= linux-external-wl-sdio-nintendo

SELECTED_SRC_FEATURES :=

#
# list and then removes the source features not desired.  This is exactly opposite of the desired
# behavior of only including features if they are explicitly named (so source features won't creep
# in if the master list is up to date.)  Somebody needs to identify the individual or group/class of
# customers that would receive this brand so the features can be picked appropriately.  I could not
# get a concrete answer to this question so for this brand I left it doing what it did previously but
# using the new feature selection framework.
#
include src-features-master-list.mk
COMPILE_TIME_SRC_FEATURES :=
COMPILE_TIME_SRC_FEATURES += $(filter-out $(SELECTED_SRC_FEATURES),$(SRC_FEATURES_MASTER_LIST))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMINTERNAL,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMCCX,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMEXTCCX,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out POCKET_PC,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLAMPDU,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLAMSDU,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLAMSDU_SWDEAGG,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMMIMO,$(COMPILE_TIME_SRC_FEATURES))

COMPILE_TIME_SRC_FEATURES := $(filter-out vxworks __ECOS DOS PCBIOS NDIS _CFE_ _RTE_ _MINOSL_ MACOSX __NetBSD__ EFI,$(COMPILE_TIME_SRC_FEATURES))

SRCFILES :=
SRCFILES += src/include/Makefile
SRCFILES += src/include/epivers.sh
SRCFILES += src/wl/linux/Makefile
SRCFILES += src/wl/linux/makefile.26
SRCFILES += src/wl/config/diffupdate.sh
SRCFILES += src/wl/config/wl.mk
SRCFILES += src/shared/README
SRCFILES += src/wl/sys/README
SRCFILES += src/wl/config/wltunable_nintendo.h
SRCFILES += src/wl/config/wlconfig_lx_wl_nintendo
SRCFILES += src/wl/config/wlconfig_lx_wl_sdio
SRCFILES += src/wl/config/wl_default
SRCFILES += src/wl/config/wl_hnd
SRCFILES += src/wl/config/wlconfig_apdef
SRCFILES += src/wl/config/wlconfig_nomimo

#export BCMSDIO      := true
#export BCMSDIOH_STD := true
export WLNINTENDO   := true

export WLTUNEFILE   := wltunable_nintendo.h
export RELEASENOTES := releasenotesnintendo.html

DRIVERS := fc3iose fc4
DRIVERS_64     :=
# Add only the lowest common for each arch from DRIVERS above to APP_DRVIERS
APP_DRIVERS    := fc4
APP_DRIVERS_64 :=

fc3iose-DEFBASICS    := nintendo nintendo-sdio
fc4-DEFBASICS        := nintendo nintendo-sdio

include $(PARENT_MAKEFILE)

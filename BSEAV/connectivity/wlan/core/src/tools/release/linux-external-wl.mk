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

export BRAND         ?= linux-external-wl

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
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMSDIO,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMCCX,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMEXTCCX,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out POCKET_PC,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLNINTENDO,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLNINTENDO2,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WME_PER_AC_TUNING,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLNOKIA,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out WLPLT,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out BCMWAPI_WPI,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out ROUTER,$(COMPILE_TIME_SRC_FEATURES))
COMPILE_TIME_SRC_FEATURES := $(filter-out LINUX_CRYPTO,$(COMPILE_TIME_SRC_FEATURES))
ifndef BCM_MFGTEST
COMPILE_TIME_SRC_FEATURES := $(filter-out WLTEST,$(COMPILE_TIME_SRC_FEATURES))
endif

COMPILE_TIME_SRC_FEATURES := $(filter-out vxworks __ECOS DOS PCBIOS NDIS _CFE_ _RTE_ _MINOSL_ MACOSX __NetBSD__ EFI,$(COMPILE_TIME_SRC_FEATURES))

# Additional sources outside of linux-wl-filelist.txt
SRCFILES =
SRCFILES += src/include/Makefile
SRCFILES += src/include/epivers.sh
SRCFILES += src/wl/linux/Makefile
SRCFILES += src/wl/linux/makefile.26
SRCFILES += src/linuxdev/Makefile
SRCFILES += src/wl/config/diffupdate.sh
SRCFILES += src/wl/config/wl.mk
SRCFILES += src/shared/README
SRCFILES += src/wl/sys/README
SRCFILES += src/wl/config/wltunable_sample.h
SRCFILES += src/wl/config/wlconfig_lx_wl_apdef
SRCFILES += src/wl/config/wl_default
SRCFILES += src/wl/config/wl_hnd
SRCFILES += src/wl/config/wlconfig_apdef
SRCFILES += src/wl/config/wlconfig_lx_wl_stadef
SRCFILES += src/wl/config/wlconfig_lx_wl_comp
SRCFILES += src/wl/config/wlconfig_lx_wl_extnvm
SRCFILES += src/wl/config/wlconfig_lx_wl_media
SRCFILES += src/wl/config/wlconfig_lx_wl_mips
SRCFILES += src/wl/config/wlconfig_lx_wl_apsta
SRCFILES += src/wl/config/wlconfig_lx_wl_p2p
SRCFILES += src/wl/config/wlconfig_lx_wl_tdls
SRCFILES += src/wl/config/wlconfig_lx_wl_mchan
SRCFILES += src/makefiles/WLAN_Common.mk
SRCFILES += src/makefiles/RelPath.mk
SRCFILES += src/makefiles/d11shm.mk
SRCFILES += src/tools/release/WLAN.usf
ifneq ($(findstring mfgtest,$(BRAND)),)
SRCFILES += src/wl/config/wlconfig_lx_wl_mfgtest
endif

ifeq ($(BRAND),linux-external-wl)
	RELEASE_WLEXESRC?=1
endif

DRIVERS       ?= fc15 fc19
DRIVERS_64    ?= fc15_64 fc19_64 fc22_64


# Add only the lowest common for each arch from DRIVERS above to APP_DRVIERS
APP_DRIVERS    ?= fc15
APP_DRIVERS_64 ?= fc15_64

fc15-DEFBASICS?= apdef-stadef apdef-stadef-extnvm
fc19-DEFBASICS?= apdef-stadef p2p-mchan apdef-stadef-extnvm

fc15_64-DEFBASICS?= apdef-stadef apdef-stadef-extnvm
fc19_64-DEFBASICS?= apdef-stadef apdef-stadef-extnvm

fc22_64-DEFBASICS?= apdef-stadef

export ALL-DEFBASICS = $(foreach driver,$(DRIVERS),$($(driver)-DEFBASICS))

# -----------------------------------------------
# usb chiprevs  = 43236*
# sdio chiprevs = 43237*
# -----------------------------------------------

include $(PARENT_MAKEFILE)

#
# Linux Internal wl build/release Makefile
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

export BRAND    ?= linux-internal-wl

ifdef DAILY_BUILD
  DRIVERS := fc15 fc19
else
  DRIVERS ?= fc15 fc19
endif
DRIVERS_64 ?= fc15_64 fc19_64 fc22_64

# Add only the lowest common for each arch from DRIVERS above to APP_DRVIERS
APP_DRIVERS    ?= fc15
APP_DRIVERS_64 ?= fc15_64

ifndef ALL-DEFBASICS
DEFBASICS_LITTLE_LIST := apdef-stadef

DEFBASICS_BIG_LIST    :=
DEFBASICS_BIG_LIST    += stadef
DEFBASICS_BIG_LIST    += apdef
DEFBASICS_BIG_LIST    += apdef-stadef-p2p
DEFBASICS_BIG_LIST    += stadef-wldiag
DEFBASICS_BIG_LIST    += p2p
DEFBASICS_BIG_LIST    += p2p-mchan

fc15-DEFBASICS        := $(DEFBASICS_LITTLE_LIST)
fc15-DEFBASICS        += apdef-stadef-p2p
fc15-DEFBASICS        += stadef
fc15-DEFBASICS        += stadef-cfg80211
fc15-DEFBASICS        += p2p-mchan
fc15-DEFBASICS        += apdef-stadef-extnvm
fc15-DEFBASICS        += apdef

#fc19
fc19-DEFBASICS        := $(DEFBASICS_LITTLE_LIST)
fc19-DEFBASICS        += apdef-stadef-p2p
fc19-DEFBASICS        += apdef-stadef-p2p-mchan-tdls
fc19-DEFBASICS        += apdef-stadef-extnvm
fc19-DEFBASICS        += apdef

fc19_64-DEFBASICS     := $(DEFBASICS_LITTLE_LIST)
fc19_64-DEFBASICS     += apdef-stadef-p2p
fc19_64-DEFBASICS     += apdef-stadef-p2p-mchan-tdls
fc19_64-DEFBASICS     += apdef-stadef-extnvm
fc19_64-DEFBASICS     += apdef

#fc22_64
fc22_64-DEFBASICS     := $(DEFBASICS_LITTLE_LIST)
fc22_64-DEFBASICS     += apdef-stadef-p2p-mchan-tdls

export ALL-DEFBASICS   = $(foreach driver,$(DRIVERS),$($(driver)-DEFBASICS))
endif # ALL-DEFBASICS

# -----------------------------------------------
# usb chiprevs  = 43236*
# sdio chiprevs = 43237*
# -----------------------------------------------


# Additional embeddable images are appended to ALL_DNGL_IMAGES from
# src/wl/linux/Makefile show_dongle_images target

include $(PARENT_MAKEFILE)

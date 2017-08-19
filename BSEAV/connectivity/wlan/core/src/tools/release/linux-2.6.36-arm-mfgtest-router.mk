#
# BCM947xx Linux 2.6.36  Mfgtest Router build/release Makefile
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id: $
#

PARENT_MAKEFILE := linux-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

# Force this release brand to be gclient only brand
export COTOOL   := gclient

$(DEFAULT_TARGET): $(ALL_TARGET)

LINUX_VERSION = 2_6_36
ROUTER_OS     = linux-2.6.36
HNDSVN_BOM   ?= linux2636-router.sparse
HNDGC_BOM    ?= linux-2.6.36-router
KERNELCFG    ?= defconfig-2.6-bcm947xx-mfgrouter
ROUTERCFG    ?= defconfig-2.6-mfgtest-router

export WLTEST := 1
export RWL    := 1

SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

CONFIG_GLIBC = false

DEFS += ETSRC ENVRAM BCMWPS BCMWFI BCM_MFGTEST
UNDEFS += BCMINTERNAL ROUTERSRC ILSRC WLSRC USBDEVSRC NASSRC EZCSRC SESSRC BCMCCX BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA CTFSRC
UNDEFS := $(filter-out BCMWPS BCMWFI BCM_MFGTEST,$(UNDEFS))

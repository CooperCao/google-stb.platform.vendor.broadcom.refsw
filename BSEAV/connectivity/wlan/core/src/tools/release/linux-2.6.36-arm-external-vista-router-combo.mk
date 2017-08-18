#
# BCM947xx Linux 2.6.36 External Router Full Src build/release Makefile
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

PARENT_MAKEFILE := linux-external-vista-router-combo.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

# Force this release brand to be gclient only brand
export COTOOL   := gclient

export DLNA     := 1

$(DEFAULT_TARGET): $(ALL_TARGET)

LINUX_VERSION = 2_6_36
ROUTER_OS     = linux-2.6.36
HNDSVN_BOM   ?= linux2636-router.sparse
HNDGC_BOM    ?= linux-2.6.36-router
KERNELCFG    ?= defconfig-2.6-bcm947xx-router-ipv6
ROUTERCFG    ?= defconfig-2.6-vista-router

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

CONFIG_GLIBC = false

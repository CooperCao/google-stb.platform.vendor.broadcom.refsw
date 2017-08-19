#
# BCM947xx Linux Internal Router build/release Makefile
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

PARENT_MAKEFILE := linux-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

# Default make target to run
$(DEFAULT_TARGET): $(ALL_TARGET)

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

DEFS   := BCMINTERNAL ETSRC WLSRC BCMDBG_PKT BCMDBG_MEM
UNDEFS := ROUTERSRC ILSRC USBDEVSRC NASSRC EZCSRC SESSRC BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA
UNDEFS += BCMVISTAROUTER BCMWPS BCMWFI
export RWL := 0

# Use NFS router configuration in internal builds
KERNELCFG ?= defconfig-bcm947xx-nfsrouter

ifndef DAILY_BUILD
# For internal builds, test build router with glibc in addition to uclibc
CONFIG_GLIBC := true
endif # DAILY_BUILD

HNDSVN_BOM := linux-router-phydev.sparse

include $(PARENT_MAKEFILE)

UNDEFS       := $(filter-out BCM_APSDSTD BCMSDIO,$(UNDEFS))

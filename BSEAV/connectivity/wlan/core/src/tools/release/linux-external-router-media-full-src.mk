#
# BCM947xx Linux External Router Media Full Source build/release Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id$
#

PARENT_MAKEFILE := linux-external-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

ROUTERSRC := true
ROUTERCFG := defconfig-media-router
KERNELCFG := defconfig-bcm947xx-router-media

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

DEFS += FULL ROUTERSRC WLSRC NASSRC EZCSRC SESSRC BCMWPS BCMWFI BCMVISTAROUTER
UNDEFS := $(filter-out FULL ROUTERSRC WLSRC NASSRC EZCSRC SESSRC BCMWPS BCMWFI BCMVISTAROUTER BCMDBG BCMDBG_ERR,$(UNDEFS))
SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

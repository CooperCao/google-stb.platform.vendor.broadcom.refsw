#
# BCM947xx Linux Internal Router build/release Makefile
# (for performance profiling)
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

$(DEFAULT_TARGET): $(ALL_TARGET)

DEFS := BCMINTERNAL \
	ETSRC WLSRC PROFSRC
UNDEFS := BCMDBG BCMDBG_ERR \
	ROUTERSRC NASSRC EZCSRC SESSRC BSPSRC ILSRC USBDEVSRC \
	BCMCCX BCMEXTCCX BCMSDIO BCMPCMCIA \
	WL_PCMCIA \
	POCKET_PC

KERNELCFG := defconfig-bcm947xx-perfprof-router
ROUTERCFG := defconfig-perfprof-router

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

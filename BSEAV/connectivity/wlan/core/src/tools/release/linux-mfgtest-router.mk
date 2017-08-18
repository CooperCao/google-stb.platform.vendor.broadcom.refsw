#
# BCM947xx Linux External Router Manufacturing Test build/release Makefile
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

PARENT_MAKEFILE := linux-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

KERNELCFG := defconfig-bcm947xx-mfgrouter
ROUTERCFG := defconfig-mfgtest-router

DEFS := ETSRC ENVRAM
UNDEFS := BCMINTERNAL ROUTERSRC ILSRC WLSRC USBDEVSRC NASSRC EZCSRC SESSRC BCMCCX BCMEXTCCX BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA CTFSRC DPSTASRC

export WLTEST := 1
export RWL    := 1

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

#
# BCM947xx Linux External Router with WPS build/release Makefile
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

ROUTERCFG := defconfig-vista-router
KERNELCFG := defconfig-bcm947xx-router-ipv6
DEFS   := ETSRC BCMWPS BCMVISTAROUTER
UNDEFS := BCMINTERNAL BCMDBG BCMDBG_ERR ROUTERSRC ILSRC WLSRC USBDEVSRC NASSRC EZCSRC SESSRC BCMCCX BCMEXTCCX BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA ENVRAM
UNDEFS += BCMWFI CTFSRC DPSTASRC

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)
SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

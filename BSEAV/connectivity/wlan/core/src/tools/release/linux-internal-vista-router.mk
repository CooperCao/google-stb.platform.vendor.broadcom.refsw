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

ROUTERCFG := defconfig-vista-router
DEFS := BCMINTERNAL ETSRC WLSRC BCMWPS BCMWFI
UNDEFS := ROUTERSRC ILSRC USBDEVSRC NASSRC EZCSRC SESSRC BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA

# Use NFS router configuration in internal builds
KERNELCFG ?= defconfig-bcm947xx-nfsrouter

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

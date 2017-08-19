#
# BCM947xx Linux External Router Mini build/release Makefile
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

ROUTERCFG ?= defconfig-mini-router
KERNELCFG ?= defconfig-bcm947xx-router-mini

DEFS   := ETSRC BCMWPS
UNDEFS := BCMINTERNAL BCMDBG BCMDBG_ERR ROUTERSRC ILSRC WLSRC USBDEVSRC NASSRC EZCSRC SESSRC
UNDEFS += BCMCCX BCMEXTCCX BSPSRC BCMSDIO BCMPCMCIA POCKET_PC WL_PCMCIA ENVRAM BCMVISTAROUTER
UNDEFS += BCMWFI

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

# Do not support SES and EZC on Router Mini builds
DEFS   := $(filter-out __CONFIG_SES__ __CONFIG_SES_CL__ __CONFIG_EZC__,$(DEFS))
UNDEFS += __CONFIG_SES__ __CONFIG_SES_CL__ __CONFIG_EZC__
SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

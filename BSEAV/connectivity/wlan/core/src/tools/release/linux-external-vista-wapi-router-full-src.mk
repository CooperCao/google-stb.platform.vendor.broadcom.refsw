#
# BCM947xx Linux External Router Full Source build/release Makefile
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

PARENT_MAKEFILE := linux-external-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

ROUTERSRC := true
ROUTERCFG := defconfig-vista-wapi-router
KERNELCFG ?= defconfig-bcm947xx-router-wapi

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

DEFS += FULL ROUTERSRC WLSRC NASSRC EZCSRC SESSRC BCMWPS BCMVISTAROUTER
UNDEFS := $(filter-out FULL ROUTERSRC WLSRC NASSRC EZCSRC SESSRC BCMWPS BCMVISTAROUTER BCMDBG BCMDBG_ERR,$(UNDEFS))

# Support WAPI, remove them from UNDEFS list which defined in linux-router.mk
UNDEFS := $(filter-out BCMWAPI_WPI BCMWAPI_WAI,$(UNDEFS))
SRCFILELISTS_COMPONENTS += src/tools/release/components/wps-filelist.txt

# Do not specify BCMWAPI_WPI BCMWAPI_WAI in DEFS list, because mogrify remove
# them from source code and the linux wl driver variants build (wl_sta) will have
# problem.

# For full-src brand release, we have to use other directive(BCMWAPISRC) for WAPI source file release.
# Now we use BCMWAPISRC to indicate we need to release both WAI and WPI source code, if someday
# someone want to release them individually please use different directive.
DEFS   += BCMWAPISRC

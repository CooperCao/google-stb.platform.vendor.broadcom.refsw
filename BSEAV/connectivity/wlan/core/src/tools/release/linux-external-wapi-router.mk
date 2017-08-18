#
# BCM947xx Linux External WAPI Router build/release Makefile
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

# Default make target to run
$(DEFAULT_TARGET): $(ALL_TARGET)

KERNELCFG ?= defconfig-bcm947xx-router-wapi
ROUTERCFG ?= defconfig-wapi-router

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

# Over-write HNDCVS_BOM (Remove it after HNDCVS_BOM changed in linux-router.mk)
HNDCVS_BOM := linux-wapirouter-bom

# Do not support SES and EZC on Router WAPI builds
DEFS   := $(filter-out __CONFIG_SES__ __CONFIG_SES_CL__ __CONFIG_EZC__,$(DEFS))
UNDEFS += __CONFIG_SES__ __CONFIG_SES_CL__ __CONFIG_EZC__

# Support WAPI, remove them from UNDEFS list which defined in linux-router.mk
UNDEFS := $(filter-out BCMWAPI_WPI BCMWAPI_WAI,$(UNDEFS))

# Do not specify BCMWAPI_WPI BCMWAPI_WAI in DEFS list, because mogrify remove
# them from source code and the linux wl driver variants build (wl_sta) will have
# problem.

# For full-src brand release, we have to use other directive(BCMWAPISRC) for WAPI source file release.
# Now we use BCMWAPISRC to indicate we need to release both WAI and WPI source code, if someday
# someone want to release them individually please use different directive.
DEFS   += BCMWAPISRC

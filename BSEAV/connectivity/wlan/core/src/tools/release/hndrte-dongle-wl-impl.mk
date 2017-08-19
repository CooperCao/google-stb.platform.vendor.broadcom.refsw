# WLAN dongle build/release Makefile
#
# This Makefile is intended to run in a Linux environment
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# Contact: hnd-software-scm-list
#
# $Id$

include vinfo.mk

start-phase         = @date +"START: $@, %D %T on $$(hostname)"
end-phase           = @date +"END: $@, %D %T on $$(hostname)"

DNGL_TOPDIR := src/dongle/make/wl

# If this makefile was invoked using lsmake we may need to switch to gnu make at some
# point in order to stay on the same host from here on. The name "$(MAKE)" has special
# properties (see manual) so it must be textually present below. Therefore, instead of
# changing the recipe literally we toggle the value of $(MAKE) between the two programs.
# If the setting HOSTMAKE=make isn't imposed from above then $(MAKE) will never change.
HOSTMAKE ?= $(MAKE)

.PHONY: all
all: build_end

# Build only releaseable firmware images
$(if $(DNGL_MAKE_IMAGES),,$(error missing DNGL_MAKE_IMAGES))
.PHONY: $(DNGL_MAKE_IMAGES)
$(DNGL_MAKE_IMAGES): MAKE := $(HOSTMAKE)
$(DNGL_MAKE_IMAGES):
	@$(start-phase)
	$(MAKE) -C $(DNGL_TOPDIR) -Orecurse -k $(or $(MAKEJOBS),-j8) NOMAXSIZE=1 $@
	@$(end-phase)

.PHONY: build_clean
build_clean: $(DNGL_MAKE_IMAGES)
	@$(start-phase)
	-find src components -name "*.o" -type f -exec rm -f {} +
	@$(start-phase)

.PHONY: build_end
build_end: MAKE := $(HOSTMAKE)
build_end: build_clean
	@$(start-phase)
	$(MAKE) -C $(DNGL_TOPDIR) imgstat
	@$(end-phase)

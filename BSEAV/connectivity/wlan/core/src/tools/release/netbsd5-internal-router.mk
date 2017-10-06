#!/bin/bash
#
# Build Internal NetBSD router flavors
#
# $Id: netbsd-internal-router.mk 241869 2011-02-20 02:33:44Z $
#

PARENT_MAKEFILE := netbsd-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

# Force this release brand to be gclient only brand
export COTOOL   := gclient

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
HNDGC_BOM := netbsd5-router
NETBSD_VERSION:=5
DEFS        :=
UNDEFS      :=
BRAND       ?= netbsd5-internal-router

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

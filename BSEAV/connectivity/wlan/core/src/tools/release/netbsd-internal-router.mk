#!/bin/bash
#
# Build Internal NetBSD router flavors
#
# $Id$
#

PARENT_MAKEFILE := netbsd-router.mk
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

# Force this release brand to be gclient only brand
export COTOOL   := gclient

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
DEFS        :=
UNDEFS      :=
BRAND       ?= netbsd-internal-router

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        /home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        /projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

include $(PARENT_MAKEFILE)

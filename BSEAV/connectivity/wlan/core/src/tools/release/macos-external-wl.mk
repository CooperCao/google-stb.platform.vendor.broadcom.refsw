#
# MacOS External wl build/release Makefile
#
# Copyright (C) 2004 Broadcom Corporation
#
# $Id$
#

PARENT_MAKEFILE := macos-wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := macos-external-wl
export OS_SUFF     := $(shell sw_vers -productVersion | sed 's,\.,_,g')

BUILD_BMAC          := true

include $(PARENT_MAKEFILE)

## Extra external build undefs
UNDEFS += BCMINTERNAL

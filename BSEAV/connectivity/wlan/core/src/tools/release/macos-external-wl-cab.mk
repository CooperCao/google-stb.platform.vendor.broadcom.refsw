#
# MacOS External wl build/release Makefile for Cab (MacOS 10.9 release)
#
# Copyright (C) 2013 Broadcom Corporation
#
#

PARENT_MAKEFILE := macos-wl.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := macos-external-wl-cab
export OS_SUFF      = 10_9
export MFGOS_SUFF   = 10_9
export P2POS_SUFF   = 10_9

BUILD_BMAC          := true

include $(PARENT_MAKEFILE)

# Snow Leopard project configs

## Extra external build undefs
UNDEFS += BCMINTERNAL

#
# Copyright (C) 2012 Broadcom Corporation
#
#

PARENT_MAKEFILE := macos-dhd.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := macos-gala-external-dongle-pcie
export OS_SUFF     := $(shell sw_vers -productVersion | sed 's,\.,_,g')
export MFGOS_SUFF  := ${OS_SUFF}
export P2POS_SUFF  := ${OS_SUFF}

BUILD_BMAC          := true

include $(PARENT_MAKEFILE)

## Extra external build undefs
UNDEFS += BCMINTERNAL

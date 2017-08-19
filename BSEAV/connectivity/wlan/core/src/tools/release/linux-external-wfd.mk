#!/bin/bash
# Build Linux WFD from trunk. Package WFD sources
#
#

PARENT_MAKEFILE := linux-wfd.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

# Force this release brand to use gclient
export COTOOL 	:= gclient

$(DEFAULT_TARGET): $(ALL_TARGET)

# Include defs and undefs
BRAND       ?= linux-external-wfd

include $(PARENT_MAKEFILE)

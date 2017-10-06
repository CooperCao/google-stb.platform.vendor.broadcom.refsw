#
# Generate external package for broadcom wireless connection API
#
# $Id$
#

PARENT_MAKEFILE := linux-cxapi.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
DEFS        :=
UNDEFS      := BCMINTERNAL
BRAND       ?= linux-external-cxapi
OEM_LIST    ?= bcm nobcmccx

include $(PARENT_MAKEFILE)

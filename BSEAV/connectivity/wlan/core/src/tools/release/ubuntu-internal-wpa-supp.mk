#
# Build Linux WPA Supplicant brand Makefile.
#
# $Id$
#

PARENT_MAKEFILE := ubuntu-wpa-supp.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

BRAND       ?= ubuntu-internal-wpa-supp

include $(PARENT_MAKEFILE)

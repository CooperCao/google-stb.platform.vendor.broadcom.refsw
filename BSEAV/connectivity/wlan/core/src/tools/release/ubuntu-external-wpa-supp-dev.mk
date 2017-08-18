#
# Build Ubuntu WPA Supplicant component brand Makefile.
#
# $Id$
#

PARENT_MAKEFILE := ubuntu-wpa-supp-dev.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

BRAND       ?= ubuntu-external-wpa-supp-dev

include $(PARENT_MAKEFILE)

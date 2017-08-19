#
# Build Ubuntu WPA Supplicant component brand Makefile.
#
# $Id$
#

PARENT_MAKEFILE := ubuntu-wpa-supp-test.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

BRAND       ?= ubuntu-external-wpa-supp-test

include $(PARENT_MAKEFILE)

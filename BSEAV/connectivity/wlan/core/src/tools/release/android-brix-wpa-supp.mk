#
# Build Ubuntu WPA Supplicant component brand Makefile.
#
# $Id$
#

PARENT_MAKEFILE := android-brix-wpa-supp-parent.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

BRAND       ?= android-brix-wpa-supp

include $(PARENT_MAKEFILE)

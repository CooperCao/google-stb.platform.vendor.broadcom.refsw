#
# External makefile to build USB DHD drivers, app and package them
#
# $Id$
#

PARENT_MAKEFILE := win_dhd_usb.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := win_external_dongle_usb
export BUILD_TYPES := free

include $(PARENT_MAKEFILE)

UNDEFS += BCMINTERNAL DHD_SPROM

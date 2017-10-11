#
# Internal makefile to build USB DHD drivers, app and package them
#
# $Id$
#

PARENT_MAKEFILE := win_dhd_usb.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

export BRAND       := win_internal_dongle_usb
export BUILD_TYPES := checked

include $(PARENT_MAKEFILE)

DEFS += DHD_SPROM

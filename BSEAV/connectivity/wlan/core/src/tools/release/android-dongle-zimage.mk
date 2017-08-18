#
# Build android CDC DHD and SDIO Dongle Image(on Ubuntu). Package DHD sources
# with all sdio dongle images
#
# $Id$
#

PARENT_MAKEFILE := android-dhd-zimage.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

BRAND       ?= android-dongle-zimage
OEM_LIST    ?= android
OEM_LIST_OPEN_SRC ?=

include $(PARENT_MAKEFILE)

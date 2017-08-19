#
# Build Linux CDC DHD/apps and All Dongle Images. Package DHD sources
# with all dongle images
#
# This brand produce internal dhd+app build and includes sdio dongle
# images
#
# $Id$
#

PARENT_MAKEFILE := linux-dhd-sdio.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
COMMONDEFS      := BCMINTERNAL
COMMONUNDEFS    :=
BRAND           ?= linux-internal-dongle
OEM_LIST        ?= bcm voipbcm

include $(PARENT_MAKEFILE)

## Build additional dhd targets for bcm internal build
BUILD_bcm_DHD_TARGETS += dhd-cdc-sdbcm

# Dongle image that needs to be embedded into host, if BCMEMBEDIMAGE is ON
# List images to be embedded for each oem
EMBED_IMAGE_bcm     ?=
EMBED_IMAGE_voipbcm ?= $(EMBED_IMAGE_bcm)

RLS_IMAGES_bcm += \
	4349a0-ram/sdio-ag \
	4349a0-ram/sdio-ag-norsdb

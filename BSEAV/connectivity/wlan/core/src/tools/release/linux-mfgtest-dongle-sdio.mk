#
# Build Linux CDC DHD/apps and All Dongle Images. Package DHD sources
# with all dongle images for mfgtest build
#
# This brand produce mfgtest dhd+app build and includes both mfgtest sdio
# dongle images
#
# $Id$
#

PARENT_MAKEFILE := linux-dhd-sdio.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

.PHONY: olympic_base_image_43012 olympic_base_image_43430
olympic_base_image_43012:
	@echo $(OLYMPIC_BASE_IMAGE_43012)
olympic_base_image_43430:
	@echo $(OLYMPIC_BASE_IMAGE_43430)

OLYMPIC_BASE_IMAGE_43012 := config_sdio_olympic_mfgtest
OLYMPIC_BASE_IMAGE_43430 := config_sdio_olympic_mfgtest

# Common defs and undefs are in included common makefile
COMMONDEFS      := BCMINTERNAL WLTEST DHD_SPROM
COMMONUNDEFS    := DHD_GPL
COMMONUNDEFS    += SERDOWNLOAD
COMMONUNDEFS    += DHD_AWDL WLAWDL
BRAND           ?= linux-mfgtest-dongle-sdio
OEM_LIST        ?= bcm
BCM_MFGTEST     := true

include $(PARENT_MAKEFILE)

# Dongle image that needs to be embedded into host, if BCMEMBEDIMAGE is ON
# List images to be embedded for each oem
EMBED_IMAGE_bcm     ?= $(strip $(shell egrep "DNGL_IMAGE_NAME.*=" src/dhd/linux/Makefile 2> $(NULL) | awk -F= '{printf("%s",$$2);}'))

# WARN: Do not add anything other than mfgtest images to following list.
# NOTE: List dongle images needed for each <oem>

RLS_IMAGES_bcm  += \
	4339a0-ram/sdio-ag-mfgtest-seqcmds \
	43012a0-ram/config_sdio_olympic_mfgtest \
	43430b0-ram/$(OLYMPIC_BASE_IMAGE_43430)
# Dual band image is too big: put it back after we have the "ucode
# download from host" feature working in TOT.
#                   4325b0/sdio-mfgtest-ag-cdc-seqcmds

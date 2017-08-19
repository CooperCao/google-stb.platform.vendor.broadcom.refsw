#
# Build Linux CDC DHD/apps and All Dongle Images. Package DHD sources
# with all dongle images for mfgtest build
#
# This brand produce mfgtest dhd+app build and includes both mfgtest sdio
# dongle images
#
# $Id: linux-mfgtest-dongle-sdio.mk 279131 2011-08-23 03:12:30Z $
#

PARENT_MAKEFILE := linux-dhd-pcie.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

.PHONY: olympic_base_image_4357 olympic_sr_image_4357 olympic_uart_image_4357
olympic_base_image_4357:
	@echo $(OLYMPIC_BASE_IMAGE_4357)
olympic_sr_image_4357:
	@echo $(OLYMPIC_SR_IMAGE_4357)
olympic_uart_image_4357:
	@echo $(OLYMPIC_UART_IMAGE_4357)

OLYMPIC_BASE_IMAGE_4357 := config_pcie_mfgtest
OLYMPIC_SR_IMAGE_4357 := config_pcie_mfgtest_sr
OLYMPIC_UART_IMAGE_4357 := config_pcie_mfgtest_uart

# Common defs and undefs are in included common makefile
COMMONDEFS      := BCMINTERNAL WLTEST DHD_SPROM
COMMONUNDEFS    := DHD_GPL
COMMONUNDEFS    += SERDOWNLOAD
COMMONUNDEFS    += DHD_AWDL WLAWDL
BRAND           ?= linux-mfgtest-dongle-pcie
OEM_LIST        ?= bcm
BCM_MFGTEST     := true

include $(PARENT_MAKEFILE)

# Dongle image that needs to be embedded into host, if BCMEMBEDIMAGE is ON
# List images to be embedded for each oem
#EMBED_IMAGE_bcm     ?= $(strip $(shell egrep "DNGL_IMAGE_NAME.*=" src/dhd/linux/Makefile 2> $(NULL) | awk -F= '{printf("%s",$$2);}'))

# WARN: Do not add anything other than mfgtest images to following list.
# NOTE: List dongle images needed for each <oem>

RLS_IMAGES_bcm  += \
		43602a1-ram/pcie-ag-mfgtest-seqcmds-splitrx \
		4359b0-ram/pcie-ag-msgbuf-p2p-mchan-splitrx-die3-norsdb-mfgtest \
		4357a0-ram/config_pcie_mfgtest \
		4357a1-ram/config_pcie_mfgtest \
		4357a1-ram/config_pcie_mfgtest_sr \
		4357a1-ram/config_pcie_mfgtest_uart \
		4357a1-ram/config_pcie_mfgtest_mu \
		4357a1-ram/config_pcie_mfgtest_debug \
		4357b0-ram/config_pcie_mfgtest \
		4361a0-ram/config_pcie_mfgtest

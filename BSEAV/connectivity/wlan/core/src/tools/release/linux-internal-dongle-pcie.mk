#
# Build Linux PCIE msgbuf DHD/apps and All Dongle Images. Package DHD sources
# with all dongle images for pcie internal full dongle build
#
# This brand produce pcie internal dhd dhd+app build and includes both
# dongle images
#
# $Id$:
#

PARENT_MAKEFILE := linux-dhd-pcie.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
COMMONDEFS      := BCMINTERNAL
COMMONUNDEFS    :=
BRAND           ?= linux-internal-dongle-pcie
OEM_LIST        ?= bcm android

include $(PARENT_MAKEFILE)

# Dongle image that needs to be embedded into host, if BCMEMBEDIMAGE is ON
# List images to be embedded for each oem
#EMBED_IMAGE_bcm     ?= $(strip $(shell egrep "DNGL_IMAGE_NAME.*=" src/dhd/linux/Makefile 2> $(NULL) | awk -F= '{printf("%s",$$2);}'))

# WARN: Do not add anything other than mfgtest images to following list.
# NOTE: List dongle images needed for each <oem>

RLS_IMAGES_bcm  += \
		43602a1-ram/pcie-ag-splitrx \
		43602a1-ram/pcie-ag-splitrx-clm_min \
		43602a1-ram/pcie-ag-err-assert-splitrx \
		43602a1-ram/pcie-ag-err-assert-splitrx-txqmux \
		43602a1-ram/pcie-ag-err-assert-splitrx-clm_min \
		43602a1-ram/pcie-ag-err-assert-p2p-mchan \
		4349a0-ram/pcie-ag-msgbuf-splitrx \
		4349a0-ram/pcie-ag-msgbuf-splitrx-norsdb-mfgtest \
		4349a0-ram/pcie-ag-msgbuf-splitrx-norsdb \
		4364a0-ram/config_pcie_release \
		4364a0-ram/config_pcie_release_row \
		4364a0-ram/config_pcie_release_norsdb \
		4364a0-ram/config_pcie_release_norsdb_row \
		4364a0-ram/config_pcie_mfgtest \
		4364a0-ram/config_pcie_mfgtest_row \
		4364a0-ram/config_pcie_assert_norsdb_row \
		4364a0-ram/config_pcie_assert_row \
		43602a1-ram/pcie-ag-msgbuf-chkd2hdma-ndis-extsta-p2p-txbf-pktctx-amsdutx-pktfilter-pno-aoe-ndoe-gtkoe-mfp-proptxstatus-keepalive-sr-ampduretry-d0c \
		4366c0-ram/pcie-ag-splitrx-fdap-mbss-mfgtest-seqcmds-phydbg-phydump-txbf-pktctx-amsdutx-ampduretry-chkd2hdma-11nprop-ringer-dmaindex16-dbgam-dbgams-bgdfs-murx \
		4357b0-ram/config_pcie_release \
		4357b0-ram/config_pcie_debug \
		4357b0-ram/config_pcie_tput

#
# Build PCIe Dongle Image(on linux). Package DHD sources
# with all pcie dongle images
#
# $Id: linux-external-dongle-pcie.mk 387033 2013-02-22 21:29:45Z $
#

PARENT_MAKEFILE := linux-dhd-pcie.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
COMMONDEFS      := BCMDONGLEHOST DHD_GPL
COMMONUNDEFS    := BCMINTERNAL BCMDBG _WIN32 _RTE_ EFI __NetBSD__
COMMONUNDEFS    += __ECOS vxworks TARGETOS_nucleus _MSC_VER _WIN64 MFGTEST WL_LOCATOR
COMMONUNDEFS    += BCMPERFSTATS BCMTSTAMPEDLOGS BCMDBG_PKT DSLCPE_DELAY
COMMONUNDEFS    += CONFIG_PCMCIA CONFIG_PCMCIA_MODULE BCMDBG_MEM ILSIM
COMMONUNDEFS    += DONGLEBUILD BCMROMOFFLOAD BCMROMBUILD
COMMONUNDEFS    += DOS PCBIOS NDIS _CFE_ _MINOSL_ UNDER_CE
COMMONUNDEFS    += WIN7 WLLMAC WLLMAC_ENABLED OLYMPIC_RWL
COMMONUNDEFS    += __BOB__ CONFIG_XIP NDIS60
COMMONUNDEFS    += WLTEST DHD_SPROM BCMDBUS
COMMONUNDEFS    += BCMDBG_ERR BCMDBG_ASSERT BCMDBG_DUMP BCMDBG_TRAP
COMMONUNDEFS    += BCMJTAG NOTYET BCMECICOEX BINOSL mips __vxworks
COMMONUNDEFS    += USER_MODE IL_BIGENDIAN BCMHND74K
COMMONUNDEFS    += APSTA_PINGTEST WLC_HIGH WLC_LOW
COMMONUNDEFS    += SI_SPROM_PROBE SI_ENUM_BASE_VARIABLE OSLREGOPS BCMASSERT_SUPPORT
COMMONUNDEFS    += EXT_STA  ATE_BUILD PATCH_SDPCMDEV_STRUCT WLC_PATCH
COMMONUNDEFS    += SERDOWNLOAD BCM_BOOTLOADER
COMMONUNDEFS    += USBAP CTFMAP PKTCTFMAP __mips__ WLC_HIGH_ONLY
COMMONUNDEFS    += __FreeBSD__ DHD_AWDL WLAWDL
COMMONUNDEFS    += CUSTOMER_HW2 CUSTOMER_HW3 CUSTOMER_HW4 CUSTOMER_HW5 CUSTOMER_HW10

BRAND       ?= linux-external-dongle-pcie
OEM_LIST    ?= bcm

include $(PARENT_MAKEFILE)

# WARN: Do not add mfgtest images to following list.
# NOTE: List dongle images needed for each <oem>

RLS_IMAGES_bcm  += \
		43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-nan-assert-redux \
		43602a1-ram/pcie-ag-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux-clm_min \
		43602a1-ram/pcie-ag-p2p-mchan-splitrx-pno-aoe-pktfilter-ndoe-pktctx-norsdb-proptxstatus-ampduhostreorder-nan-assert-redux \
		43602a1-ram/pcie-ag-p2p-mchan-splitrx-norsdb-tdls-assert \
		43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-err-logtrace-redux \
		43602a1-ram/pcie-ag-p2p-mchan-splitrx-pno-aoe-pktfilter-ndoe-pktctx-norsdb-proptxstatus-ampduhostreorder-assert \
		43602a1-ram/pcie-ag-splitrx-pktctx-proptxstatus-ampduhostreorder-aibss-relmcast-norsdb-redux-assert \
		43602a1-ram/pcie-ag-p2p-mchan-splitrx-pktctx-proptxstatus-ampduhostreorder-assert-logtrace-redux-proxd \
		43602a1-ram/pcie-ag-splitrx-pktctx-proptxstatus-ampduhostreorder-aibss-relmcast-btcdyn-norsdb-redux-assert \
		43602a1-ram/config_pcie_awdl \
		4359b0-ram/threadx-pcie-ag-msgbuf-p2p-mchan-splitrx-ampduhostreorder-proptxstatus-sr-pktctx-norsdb-nan \
		4357a0-ram/config_pcie_awdl \
		4357a0-ram/config_pcie_release \
		4357a0-ram/config_pcie_debug \
		4357a0-ram/config_pcie_tput \
		4357a0-ram/config_pcie_nan \
		4357a1-ram/config_pcie_release \
		4357a1-ram/config_pcie_debug \
		4357a1-ram/config_pcie_tput \
		4361a0-ram/config_pcie_release \
		4361a0-ram/config_pcie_debug \
		4357a0-ram/config_pcie_dbgnan \
		43452a3-ram/config_pcie_base \
		43452a3-ram/config_pcie_nan \
		43452a3-ram/config_pcie_dbgnan

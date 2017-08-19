#
# Build Linux CDC DHD and SDIO Dongle Image(on linux). Package DHD sources
# with all sdio dongle images
#
# $Id$
#

PARENT_MAKEFILE := linux-dhd-sdio.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Common defs and undefs are in included common makefile
COMMONDEFS        := BCMDONGLEHOST BCMSDIO DHD_GPL
COMMONUNDEFS      := BCMINTERNAL BCMDBG _WIN32 _RTE_ EFI __NetBSD__
COMMONUNDEFS      += __ECOS vxworks TARGETOS_nucleus _MSC_VER _WIN64 MFGTEST WL_LOCATOR
COMMONUNDEFS      += BCMPERFSTATS BCMTSTAMPEDLOGS BCMDBG_PKT DSLCPE_DELAY
COMMONUNDEFS      += CONFIG_PCMCIA CONFIG_PCMCIA_MODULE BCMDBG_MEM ILSIM
COMMONUNDEFS      += DONGLEBUILD BCMROMOFFLOAD BCMROMBUILD
COMMONUNDEFS      += DOS PCBIOS NDIS _CFE_ _MINOSL_ UNDER_CE
COMMONUNDEFS      += WIN7 WLLMAC WLLMAC_ENABLED OLYMPIC_RWL
COMMONUNDEFS      += __BOB__ CONFIG_XIP NDIS60
COMMONUNDEFS      += WLTEST DHD_SPROM BCMDBUS
COMMONUNDEFS      += BCMDBG_ERR BCMDBG_ASSERT BCMDBG_DUMP BCMDBG_TRAP
COMMONUNDEFS      += BCMJTAG NOTYET BCMECICOEX BINOSL mips __vxworks
COMMONUNDEFS      += USER_MODE IL_BIGENDIAN BCMHND74K
COMMONUNDEFS      += APSTA_PINGTEST WLC_HIGH WLC_LOW
COMMONUNDEFS      += SI_SPROM_PROBE SI_ENUM_BASE_VARIABLE OSLREGOPS BCMASSERT_SUPPORT
COMMONUNDEFS      += EXT_STA  ATE_BUILD PATCH_SDPCMDEV_STRUCT WLC_PATCH
COMMONUNDEFS      += SERDOWNLOAD BCM_BOOTLOADER
COMMONUNDEFS      += USBAP CTFMAP PKTCTFMAP __mips__ WLC_HIGH_ONLY
COMMONUNDEFS      += __FreeBSD__ DHD_AWDL WLAWDL

BRAND       ?= linux-external-dongle-sdio
OEM_LIST    ?= bcm
OEM_LIST_OPEN_SRC ?= bcm

include $(PARENT_MAKEFILE)

# Dongle image that needs to be embedded into host, if BCMEMBEDIMAGE is ON
# List images to be embedded for each oem
EMBED_IMAGE_bcm     ?= $(strip $(shell egrep "DNGL_IMAGE_NAME.*=" src/dhd/linux/Makefile 2> $(NULL) | awk -F= '{printf("%s",$$2);}'))
EMBED_IMAGE_voipbcm ?= $(EMBED_IMAGE_bcm)

# WARN: Do not add mfgtest images to following list.
# NOTE: List dongle images needed for each <oem>

RLS_IMAGES_bcm      += \
	43430b0-ram/sdio-g \
	4339a0-ram/sdio-ag-p2p-assert \
	4339a0-ram/sdio-ag-p2p-mchan-aoe-keepalive-pktfilter-proptxstatus-dmatxrc-pktctx-assert \
	4339a0-ram/sdio-ag-p2p-proptxstatus-dmatxrc-pktctx-assert-clm_4335c0_min \
	4339a0-ram/sdio-ag-assert-clm_4335c0_min \
	43012a0-ram/config_sdio_debug \
	43012a0-ram/config_sdio_noap

RLS_IMAGES_voipbcm  +=

RLS_IMAGES_nokia    +=

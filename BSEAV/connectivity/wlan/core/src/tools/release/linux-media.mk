#
# This makefile is used for the following
#    - Common definitions used for -media- brands
#    - Includes the parent makefile 'linux.mk' for the -media- brands
# This makefile is included from the following brand makefiles
#    - linux-internal-media.mk
#    - linux-external-media.mk
#    - linux-mfgtest-media.mk
#
# $Id$
#

PARENT_MAKEFILE := linux.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

## -----------------------------------------------------------------------
## The filelists and component level central filelists for -media- brands
## -----------------------------------------------------------------------
SRCFILELIST  := \
	src/tools/release/linux-dhd-media-filelist.txt \
	src/tools/release/linux-media-filelist.txt \
	src/tools/release/linux-wlexe-filelist.txt \
	src/tools/release/linux-usbbcmdl-filelist.txt
SRCFILELIST_GPL  := \
	src/tools/release/linux-dhd-gpl-filelist.txt
SRCFILELIST_HSL  := \
	src/tools/release/linux-p2plib-filelist.txt
SRCFILELISTS_COMPONENTS := \
	src/tools/release/components/wlphy-filelist.txt \
	src/tools/release/components/bcmcrypto-filelist.txt
SRCFILELISTS_HSL_COMPONENTS  := \
	src/tools/release/components/wps-filelist.txt \
	src/tools/release/components/secfrw-filelist.txt \
	src/tools/release/components/pre_secfrw-filelist.txt \
	src/tools/release/components/p2p-filelist.txt \
	src/tools/release/components/dhcpd-filelist.txt \
	src/tools/release/components/supp-filelist.txt \
	src/tools/release/components/bwl-filelist.txt \
	src/tools/release/components/bcmcrypto-filelist.txt \
	src/tools/release/components/wfi-filelist.txt

# Final Filelist for Parent makefile
BRAND_SRCFILELIST  := $(SRCFILELIST) $(SRCFILELIST_GPL) $(SRCFILELIST_HSL) \
	$(SRCFILELISTS_COMPONENTS) $(SRCFILELISTS_HSL_COMPONENTS)

## -----------------------------------------------------------------------
## The filelist DEFS and UNDEFS for -media- brands
## -----------------------------------------------------------------------
COMMON_SRCFILELIST_DEFS := \
	WLPHY BCMCRYPTO BCMWPS LINUX BCMWFI P2P BWL WFI DHCPD_APPS BCMDBUS WLLX \
	ESTA_OPENSRC_CLEANUP_LIST

COMMON_SRCFILELIST_UNDEFS := \
	BCMSDIO POCKET_PC WLNINTENDO WLNINTENDO2 WME_PER_AC_TUNING WLNOKIA \
	WLPLT BCMWAPI_WPI ROUTER LINUX_CRYPTO \
	vxworks __ECOS DOS PCBIOS NDIS _CFE_ _HNDRTE_ _MINOSL_ MACOSX __NetBSD__ EFI \
	LINUX_ROUTER ECOS_ROUTER WINCE PRE_SECFRW DHCPD_APPS \
	SUPP OEM_CHROMIUMOS NO_WLEXTLOG
# COMMON_SRCFILELIST_UNDEFS += OEM_ANDROID

# Media Specific DEFS and UNDEFS for filelist (from Brand Makefile)
MEDIA_BRAND_SRCFILELIST_DEFS   ?=
MEDIA_BRAND_SRCFILELIST_UNDEFS ?=

# Final Filelist DEFS & UNDEFS for Parent makefile
BRAND_SRCFILELIST_DEFS   := $(COMMON_SRCFILELIST_DEFS) $(MEDIA_BRAND_SRCFILELIST_DEFS)
BRAND_SRCFILELIST_UNDEFS := $(COMMON_SRCFILELIST_UNDEFS) $(MEDIA_BRAND_SRCFILELIST_UNDEFS)

## -----------------------------------------------------------------------
## The Mogrification DEFS and UNDEFS for -media- brands
## -----------------------------------------------------------------------
COMMON_MOGRIFY_DEFS := BCM47XX_CHOPS

COMMON_MOGRIFY_UNDEFS := BCM33XX_CHOPS BCM42XX_CHOPS BCM4413_CHOPS \
	BCM4710A0 BCM93310 BCM93350 BCM93352 BCM93725B BCM94100 \
	BCMENET BCMP2P BCMQT BCMSDIO BCMSDIODEV BCMSIM BCMSIMUNIFIED BCMSLTGT \
	BCM_USB BCMUSBDEV COMS \
	CONFIG_BCM4710 CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCRM_93725 CONFIG_BRCM_VJ CONFIG_MIPS_BRCM \
	DEBUG_LOST_INTERRUPTS DSLCPE ETSIM INTEL __klsi__ LINUXSIM NETGEAR \
	NICMODE PSOS ROUTER VX_BSD4_3 WLFIPS WLNINTENDO WLNINTENDO2 \
	BCM_ROUTER_DHD BCM_GMAC3

# Media Specific Mogrification DEFS and UNDEFS (from Brand Makefile)
MEDIA_BRAND_MOGRIFY_DEFS      ?=
MEDIA_BRAND_MOGRIFY_UNDEFS    ?=

# Final Mogrification DEFS & UNDEFS for Parent makefile
BRAND_MOGRIFY_DEFS     := $(COMMON_MOGRIFY_DEFS) $(MEDIA_BRAND_MOGRIFY_DEFS)
BRAND_MOGRIFY_UNDEFS   := $(COMMON_MOGRIFY_UNDEFS) $(MEDIA_BRAND_MOGRIFY_UNDEFS)

## -----------------------------------------------------------------------
## The Driver Types and Platform/OS specific definitions for each <oem>
## -----------------------------------------------------------------------
# OEM list
OEM_LIST   ?= bcm

# The Operating System of the Host Driver Builds
HOST_DRV_OS_bcm       ?= fc19x64 fc22x64 \
	stb7425mipslx33 stb7445armlx31419 stb7252armlx314111 stb7252armlx4110

HOST_DRV_OS_android   ?=

# The Driver Types to build for each <oem>
#    dhd --> Full Dongle Host driver
#    wl -->  BMAC / NIC Driver
HOST_DRV_TYPE_bcm     ?= dhd wl
HOST_DRV_TYPE_android ?=

# The Operating System of the Apps Builds
APP_DRV_OS_bcm        ?= fc19x64 stb7252armlx314111 stb7252armlx4110 stb7425mipslx33
APP_DRV_OS_android    ?=

# The Operating System of the HSL Builds
#APP_HSL_OS_bcm        ?= fc15 fc19x64 stbfc15
APP_HSL_OS_bcm        ?= fc19x64 stb7425mipslx33 stb7252armlx314111 stb7252armlx4110
APP_HSL_OS_android    ?=

## -----------------------------------------------------------------------
## Check Validity of Brands
## -----------------------------------------------------------------------
VALID_BRANDS := \
	linux-external-media \
	linux-internal-media \
	linux-mfgtest-media

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
	$(error ERROR This $(BRAND) build is not a standalone build brand. Valid brands are $(VALID_BRANDS))
endif

include $(PARENT_MAKEFILE)

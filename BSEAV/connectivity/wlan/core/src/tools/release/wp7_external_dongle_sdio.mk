#
# Window Mobile 7 (Windows Phone 7) external build brand
# NOTE: This is a temporary build brand until
#
# $Id$
#

## Add only brand specific defs and undefs here, for common ones update
## included ce_external_dongle_sdio.mk
DEFS   := _WIN32 BCMDRIVER BDC BCMSDIO STA NDIS60 NDIS \
          EXT_STA BCMSUP_PSK BCMWPA2 NDIS_WDM NDIS_MINIPORT_DRIVER \
          WDM
# UNDEFS += WL11N SIMPLE_ISCAN
UNDEFS := BCMDHDUSB P2P WLC_HIGH_ONLY PRE_v1_68_HEADERS \
          WOWL ARPOE TOE AP BCMDBUS
# UNDEFS += WL_IW_USE_ISCAN

BRAND               := wp7_external_dongle_sdio
BUILD_WP7            = 1
BUILD_INSTALLER      =

export WMTOOL_VER   := WM700
export WMTOOL_AKU   := 6020
export WPTOOL_VER   := $(WMTOOL_VER)_$(WMTOOL_AKU)

export DEVTTYPES    := retail debug
export DEVNAMES     := bcm
export DEVTYPES     := sddhd
export DEVOSVERS_all:= 700
export DEVPROCS_all := ARMV6 ARMV7

include ce_external_dongle_sdio.mk

# Override default list of firmware images for Windows Phone 7
ifdef WLTEST
     # mfgtest firmware
     EMBED_DONGLE_IMAGE  ?=
else  # non-mfgtest firmware
     EMBED_DONGLE_IMAGE  ?=
endif # BCM_MFGTEST

ALL_DNGL_IMAGES	     = $(EMBED_DONGLE_IMAGE)

ce_external_dongle_sdio.mk: OVFILE=$(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
ce_external_dongle_sdio.mk:
	cvs -q -d${CVSROOT} co $(if $(TAG),-r $(TAG)) -p src/tools/release/$@ > $@
ifneq ($(OVERRIDE),)
	-[ -f "$(OVFILE)" ] && cp $(OVFILE) $@
endif

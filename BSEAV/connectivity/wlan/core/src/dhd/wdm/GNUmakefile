#
# $Id: GNUmakefile,v 1.7 2008-07-02 19:15:29 $
#

## By default USB drivers are built. Refer below for specific targets
ifneq ($(findstring $(WINOS),WIN10 WIN8X WIN7),)
   include win8xdriver.mk
else
   include builddriver.mk
endif

## Build USB XP and Vista drivers (32 and 64bit)
build_usb: build_usb_xp_driver build_usb_vista_driver

## Build SDIO XP drivers (only 32bit supported for now)
build_sdio: build_sdstd_xp_driver

old_build_sdio_xp_driver:
	$(MAKE) -f sdspi.mk

## Build SDSPI XP drivers (only 32bit supported for now)
build_sdspi: build_sdspi_xp_driver

build_sdspi_xp_driver:
	$(MAKE) -f sdspi.mk

## Build gSPI XP drivers (only 32bit supported for now)
build_gspi: build_gspi_xp_driver

build_gspi_xp_driver:
	$(MAKE) -f gspi.mk

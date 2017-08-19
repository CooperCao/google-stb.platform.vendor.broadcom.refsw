#
# $Id$
#
#
NULL               := /dev/null
FIND               := /usr/bin/find
TREECMD		   := /usr/bin/tree
BUILD_BASE         := $(shell pwd)
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release
MOGRIFY            := $(SRCDIR)/tools/build/mogrify.pl
SRCFILTER          := $(SRCDIR)/tools/build/srcfilter.pl
SRCFILELIST        := $(SRCDIR)/tools/release/linux-wfi-filelist.txt
UNAME              := $(shell uname -a)
OEM_LIST           ?= bcm
BRAND		   ?= linux-wfi
CC		   = gcc
empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

#
# NOTE: Since this is a global makefile, include only global DEFS and UNDEFS
# here. Device and Build type specific flags should go to individual (calling)
# makefile
#
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV BCMCCX BCMEXTCCX  \
          BCMSDIODEV WLFIPS BCMWAPI_WPI BCMP2P BCMWAPI_WAI

UNDEFS += DEBUG_LOST_INTERRUPTS BCMQT BCMSLTGT

UNDEFS += UNDER_CE

# These symbols will be DEFINED in the source code by the mogrifier
#
DEFS += BCM47XX BCM47XX_CHOPS

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

include unreleased-chiplist.mk
include swrls.mk

all: extract_src mogrify build release build_end

extract_src:
	cvs co src/tools/release/linux-wps-enrollee.mk
	make -f src/tools/release/linux-wps-enrollee.mk extract_src
	cvs co src/apps/wfi/linux
	cvs co src/tools/build
	cvs co src/doc/BCMLogo.gif
	
# #########################################
# Mogrification targets
# #########################################
mogrify: src/.mogrified

# run mogrifier
src/.mogrified:
	@date +"START: $@, %D %T"
	@echo " -- MARK mogrify --"
	$(FIND) src components $(MOGRIFY_EXCLUDE) -type f -print  |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' | \
		xargs perl $(MOGRIFY) $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
	touch src/.mogrified
	@date +"END:   $@, %D %T"

build: build_prep build_wfi

build_prep:
	@date +"START:   $@, %D %T"
	# Copy over release sources to build folder now for verification
	find src components | perl $(SRCFILTER) -v $(SRCFILELIST) | col -b | \
		cpio -p -d build/bcm
	@date +"END:   $@, %D %T"

build_wfi:
	@date +"START:   $@, %D %T"
	$(MAKE) -C build/bcm/src/apps/wfi/linux/wfi_api BLDTYPE=release
	@date +"END:   $@, %D %T"

release: build_wfi
	@date +"START:   $@, %D %T"
	# Prepare binary package in release/apps
	install -Dp "build/bcm/src/apps/wfi/linux/wfi_api/$(CC)/wfiapitester" release/bcm/apps/wfiapitester
	install -Dp "build/bcm/src/apps/wfi/linux/wfi_api/$(CC)/libwfiapi.a" release/bcm/libs/libwfiapi.a
	install -p "build/bcm/src/wps/linux/enr/$(CC)/libwpsapi.a" release/bcm/libs/libwpsapi.a
	install -p "build/bcm/src/wps/linux/enr/$(CC)/libwpsenr.a" release/bcm/libs/libwpsenr.a
	install -p "build/bcm/src/wps/linux/enr/$(CC)/libwpscom.a" release/bcm/libs/libwpscom.a
	install -p "build/bcm/src/wps/linux/enr/$(CC)/libbcmcrypto.a" release/bcm/libs/libbcmcrypto.a
	# Prepare api sdk package in release/api_sdk
	install -Dp "build/bcm/src/apps/wfi/linux/include/wfi_api.h" release/bcm/include/wfi_api.h
	install -p src/doc/BCMLogo.gif release/bcm/
# ##########	
	#install -p src/doc/BrandReadmes/$(BRAND).txt release/bcm/README.txt
# ##########	
	#install -p src/doc/WPS_ReleaseNotes.html release/bcm/
	# When filelist contents grow, windows utilities complain that
	# command line is too long. So pass filelist via input file
	cd build/bcm && find src components | perl $(SRCFILTER) -v $(SRCFILELIST) | \
		    grep -v "\.o\|\.obj\|\.pdb\|\.map\|\.exe\|/CVS" > pkg_these
	# Copy over validated sources to release/<oem>/src folder now
	tar cpf - -C build/bcm -T build/bcm/pkg_these \
            --exclude=*.o --exclude=*.obj --exclude=*.pdb --exclude=*.map --exclude=*.exe --exclude=*/.svn | \
	    tar xvpf - -C release/bcm
	tar cpf release/bcm/linux-wfi-source.tar -C release/bcm \
		--exclude=*/.svn \
		src apps libs include BCMLogo.gif
#		src apps libs include BCMLogo.gif README.txt
	cd release/bcm; $(TREECMD) src apps libs include > linux-wfi-source-contents.txt
	gzip -9 release/bcm/linux-wfi-source.tar
	@date +"END:   $@, %D %T"

build_clean: release
	rm -rf build
	rm -rf release/bcm/src
	-@find src components -type f -name "*.obj" -o -name "*.OBJ" -o -name "*.o" | \
		xargs rm -f

build_end: build_clean
	        @date +"BUILD_END: $(BRAND) TAG=${TAG} %D %T"

%.mk:
	cvs co -p src/tools/release/$@ > $@
		
phony:

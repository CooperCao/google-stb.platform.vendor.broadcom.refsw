#!/bin/bash
#
# NetBSD Router master makefile
#
# NOTE: This makefile should not be used by itself, it is included into
# other brands like netbsd-external-router.mk
#
# $Id$
#

DATE               := $(shell date -I)
BUILD_BASE         := $(shell pwd)
MAKE_MODE          := unix

NULL               := /dev/null
FIND               := /usr/bin/find
SRCDIR             := $(BUILD_BASE)/src
RELEASEDIR         := $(BUILD_BASE)/release
MOGRIFY            := perl $(SRCDIR)/tools/build/mogrify.pl
SRCFILTER          := perl $(SRCDIR)/tools/build/srcfilter.pl
ifeq ($(NETBSD_VERSION), 5)
                   SRCFILELISTS_COMPONENTS += src/tools/release/components/wlphy-filelist.txt
                   SRCFILELISTS_COMPONENTS += src/tools/release/components/bcmcrypto-filelist.txt
                   SRCFILELIST        := $(SRCDIR)/tools/release/netbsd5-router-filelist.txt
else
  SRCFILELIST        := $(SRCDIR)/tools/release/netbsd-router-filelist.txt
endif
UNAME              := $(shell uname -a)
OEM_LIST           ?= bcm
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE


VALID_BRANDS       := netbsd-external-router netbsd-internal-router netbsd5-external-router netbsd5-internal-router

ifeq ($(NETBSD_VERSION), 5)
  BSD_TOOL_PATH      := /projects/hnd/tools/linux/hndtools-mipseb-netbsd-5.0/bin:/projects/hnd/tools/linux/hndtools-mipsel-netbsd-5.0/bin
else
  BSD_TOOL_PATH      := /projects/hnd/tools/linux/hndtools-mipseb-netbsd-4.0/bin:/projects/hnd/tools/linux/hndtools-mipsel-netbsd-4.0/bin
endif

ifeq ($(filter $(VALID_BRANDS),$(BRAND)),)
        $(warning ERROR: This $(BRAND) build is not a standalone build brand)
        $(warning ERROR: Valid brands are $(VALID_BRANDS))
        $(error Exiting)
endif # BRAND

ifneq ($(VCTOOL),svn)
  export PATH          := /usr/bin:$(BSD_TOOL_PATH):$(PATH)
else
  export PATH          := /tools/bin:/usr/bin:$(BSD_TOOL_PATH):$(PATH)
endif

export BSD_TOOL_PATH

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $1/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $1/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

vinfo:=$(if $(TAG),$(subst _,$(space),$(TAG)),$(shell date '+X REL %Y %m %d'))
maj  := $(word 3,$(vinfo))
min  := $(word 4,$(vinfo))
rcnum:= $(patsubst RC%,%,$(word 5,$(vinfo)))
rcnum:= $(if $(rcnum),$(rcnum),0)
incr := $(word 6,$(vinfo))
incr := $(if $(incr),$(incr),0)

# Effective Build or Release Number
RELNUM := $(maj).$(min).$(rcnum).$(incr)

HNDGC_BOM ?= netbsd-net80211-router

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
# NOTE: Since this is a global makefile, include only common DEFS and UNDEFS
# here. Device and Build type specific flags should go to individual (calling)
# brand makefile
#
UNDEFS += \
CONFIG_BCM93725 CONFIG_BCM93725_VJ CONFIG_BCM93730 CONFIG_BCM933XX \
BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 \
MICROSOFT MSFT vxworks DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO \
_RTE_ __ARM_ARCH_4T_ WLNOKIA BCM_APSDSTD BCMDONGLEHOST \
BCMNODOWN BCMROMOFFLOAD BCMSDIODEV WLPFN_AUTO_CONNECT BCMCCX WL_LOCATOR \
WLFIPS EXT_STA BCMWAPI_WPI

# These symbols will be DEFINED in the source code by the transmogirifier
DEFS += \
CONFIG_MIPS_BRCM BCM47XX_CHOPS BCM4710A0 BCM47XX BCMENET __CONFIG_SES__ \
__CONFIG_SES_CL__ WLSRC BCMCRYPTO_SRC

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
#TODO# netbsd shell scripts are not getting mogrified properly, fix mogrifier
#disabled# MOGRIFY_EXT = [^\/]*\.c [^\/]*\.h [^\/]*\.mk [^\/]*\.inc [^\/]*\.s [^\/]*\.tcl [^\/]*\.cpp [^\/]*\.h\.in \/sources.* [^\/]*filelist\.txt [^\/]*readme.*txt [^\/]*wl\.wlex [^\/]*akefile.* [^\/]*akerule.* [^\/]*\.sh
MOGRIFY_EXT = [^\/]*\.c [^\/]*\.h [^\/]*\.mk [^\/]*\.inc [^\/]*\.s [^\/]*\.tcl [^\/]*\.cpp [^\/]*\.h\.in \/sources.* [^\/]*filelist\.txt [^\/]*readme.*txt [^\/]*wl\.wlex [^\/]*akefile.* [^\/]*akerule.*

MOGRIFY_DIRS =  src/include \
		src/shared \
		src/wl \
		src/bcmcrypto \
		src/tools \
		components/router 	
ifeq ($(NETBSD_VERSION), 5)
   MOGRIFY_DIRS += src/netbsd-5.0.2/router
else
   MOGRIFY_DIRS += src/netbsd/router
endif

# Default build rule
all: profile.log checkout mogrify build_include build_router release build_end

ifneq ($(ALLCHIPS),1)
include unreleased-chiplist.mk
endif # ALLCHIPS
include swrls.mk

profile.log :
	@date +"BUILD_START: $(BRAND) TAG=${TAG} %D %T" | tee profile.log

# check out files
checkout: profile.log $(CHECKOUT_TGT)

## ------------------------------------------------------------------
## Mogrify sources.
## ------------------------------------------------------------------
mogrify: src/.mogrified

src/.mogrified :
	@date +"START: $@, %D %T" | tee -a profile.log
	$(FIND) $(MOGRIFY_DIRS) $(MOGRIFY_EXCLUDE) -type f -print  |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' | \
		xargs $(MOGRIFY) $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
	touch src/.mogrified
	@date +"END: $@, %D %T" | tee -a profile.log

build_include: checkout
	@date +"START: $@, %D %T"  | tee -a profile.log
	$(MAKE) -C src/include
	@date +"END:   $@, %D %T"  | tee -a profile.log

## ------------------------------------------------------------------
## Build netbsd flavors now
## ------------------------------------------------------------------
build_router:
	@date +"START: $@, %D %T"  | tee -a profile.log
	@echo "========== Building with debug ================"
ifeq ("$(NETBSD_VERSION)", "5")
	$(MAKE) -C src/wl/bsd
	$(MAKE) -C src/netbsd-5.0.2 BCMHND74K=1 KERN_CONFIG=BCM47XX-MD-DBG
	install -pv src/netbsd-5.0.2/images/netbsd.trx netbsd_74k.trx
else
	$(MAKE) -C src/netbsd BCMHND74K=1 KERN_CONFIG=BCM47XX-MD-DBG
	install -pv src/netbsd/images/netbsd.trx netbsd_74k.trx
endif

	@echo "========== Building without debug ================"
ifeq ("$(NETBSD_VERSION)", "5")
	$(MAKE) -C src/wl/bsd
	$(MAKE) -C src/netbsd-5.0.2 clean KERN_CONFIG=BCM47XX-MD-DBG
	$(MAKE) -C src/netbsd-5.0.2 BCMHND74K=1 KERN_CONFIG=BCM47XX-MD
	install -pv src/netbsd-5.0.2/images/netbsd.trx netbsd_74k_nodebug.trx
else
	$(MAKE) -C src/netbsd clean KERN_CONFIG=BCM47XX-MD-DBG
	$(MAKE) -C src/netbsd BCMHND74K=1 KERN_CONFIG=BCM47XX-MD
	install -pv src/netbsd/images/netbsd.trx netbsd_74k_nodebug.trx
endif
	@date +"END: $@, %D %T" | tee -a profile.log

## ------------------------------------------------------------------
## Package built binaries now
## ------------------------------------------------------------------
release_bins:
	@date +"START: $@, %D %T"  | tee -a profile.log
	install -d $(RELEASEDIR)
	$(call WARN_PARTIAL_BUILD,release)
	install -d -m 755 release/images
	# touch release/README.txt
	# -install src/doc/BrandReadmes/$(BRAND).txt release/README.txt
	# -install src/doc/BCMLogo.gif release
	mv netbsd_74k_nodebug.trx release/images/netbsd_74k_nodebug.trx
	mv netbsd_74k.trx release/images/netbsd_74k.trx
	$(call REMOVE_WARN_PARTIAL_BUILD,$(reldir))
	@date +"END:   $@, %D %T"  | tee -a profile.log

## ------------------------------------------------------------------
## Create source releases
## ------------------------------------------------------------------
release_src:

release: release_bins

build_clean: release
	-@find src components -type f -name "*.o" -a ! -name "*_dbgsym.o" | xargs rm -f

build_end: build_clean
	@date +"BUILD_END: $(BRAND) TAG=${TAG} %D %T" | tee -a profile.log

.PHONY: FORCE checkout build_include build_end build_router release release_bins release_src

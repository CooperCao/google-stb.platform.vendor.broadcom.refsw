#
# Build Windows WPS SDK and WPS Wizard
#
# $Id$
#

# HOW THIS MAKEFILE WORKS (This makefile can NOT be used by itself)
#
# 1. Checkout source from SVN to src/. The directories and modules to
# check out are in $(HNDSVN_BOM).
#
# 2. Run the mogrifier on all source code to remove some selected
# defines and to force the expression of others. Also some comments
# undefined by the transmogrifier are in $(UNDEFS). The symbols to be
# defined by the transmogrifier are in $(DEFS).
#
# 3. Build binaries include both wps enrolle and wps installer
#
# 4. copy binary files to release folder and zip them
#

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

export VCTOOL   := svn

PARENT_MAKEFILE :=
CHECKOUT_RULES  := checkout-rules.mk
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

# Include custom (if exists) or central checkout rules
include $(strip $(firstword $(wildcard \
        $(CHECKOUT_RULES) \
        Z:/home/hwnbuild/src/tools/release/$(CHECKOUT_RULES) \
        Z:/projects/hnd_software/gallery/src/tools/release/$(CHECKOUT_RULES))))

# include $(PARENT_MAKEFILE)

## Caller makefile defines BRAND, BUILD_TYPES variables
DATE               := $(shell date -I)
BUILD_BASE         := $(shell pwd)
RELEASEDIR         := $(BUILD_BASE)/release
SHELL              := bash.exe
MAKE_MODE          := unix
NULL               := /dev/null
ENR_DIR            := src/wps/wpsapi/win32
WARN_FILE          := _WARNING_PARTIAL_BUILD_DO_NOT_USE

ifneq ($(origin TAG), undefined)
    export TAG
    vinfo := $(subst _,$(space),$(TAG))
else
    vinfo := $(shell date '+D11 REL %Y %m %d')
endif

maj:=$(word 3,$(vinfo))
min:=$(word 4,$(vinfo))
rcnum:=$(word 5,$(vinfo))
rcnum:=$(patsubst RC%,%,$(rcnum))
ifeq ($(rcnum),)
  rcnum:=0
endif
incremental:=$(word 6,$(vinfo))
ifeq ($(incremental),)
  incremental:=0
endif
RELNUM=$(maj).$(min).$(rcnum).$(incremental)
RELDATE=$(shell date '+%m/%d/%Y')

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $1/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $1/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# These are module names and directories that will be checked out of CVS.
HNDSVN_BOM := win-wps-enrollee.sparse

# These are EFFECTIVE only for host-side compilation
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV \
	  BCMCCX BCMEXTCCX BCMSDIODEV BCMSDIO WLFIPS \
	  BCMP2P

UNDEFS += DEBUG_LOST_INTERRUPTS BCMQT BCMSLTGT BCMINTERNAL

include unreleased-chiplist.mk

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS + =

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout mogrify build_enrollee build_install release build_end

include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# check out files
checkout : $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

# run mogrifier
mogrify: checkout
	@$(MARKSTART)
	$(call WARN_PARTIAL_BUILD,$(BUILD_BASE))
	$(MAKE) -f $(MOGRIFY_RULES)
	$(call REMOVE_WARN_PARTIAL_BUILD,$(BUILD_BASE))
	@$(MARKEND)

build_include: checkout filelists  mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

build_enrollee: checkout mogrify
	@$(MARKSTART)
	$(MAKE) -C $(ENR_DIR)
	@$(MARKEND)

build_install: export VS_PRE_BUILD_ENABLED=0
build_install: build_enrollee checkout mogrify
	@$(MARKSTART)
	$(MAKE) -C src/tools/install/WPSWizard
	@$(MARKEND)

clean:
	$(MAKE) -C $(ENR_DIR) clean_all
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll clean

release: release_bins

release_bins:
	@$(MARKSTART)
	# First create wps sdk package
	install -d release/Bcm/
	$(call WARN_PARTIAL_BUILD,release/Bcm)
	install -p $(ENR_DIR)/readme.txt release/Bcm/
	install -p $(ENR_DIR)/vcredist_x86.exe release/Bcm/
	install -p src/8021x/wpdpack/WpdPack_4_1/Installer/WinPcap_setup.exe release/Bcm/
	install -p src/doc/BCMLogo.gif release/Bcm/
	install -p $(ENR_DIR)/WPS_SDK_API_Reference_Manual.doc release/Bcm/
	install -pD src/wps/wpsapi/common/include/wps_sdk.h release/Bcm/common/include/wps_sdk.h
	install -pD src/include/epivers.h release/Bcm/common/include/epivers.h
	install -pD $(ENR_DIR)/bin/debug/wps_api.dll release/Bcm/win32/bin/debug/wps_api.dll
	install -pD $(ENR_DIR)/bin/debug/wps_api.lib release/Bcm/win32/bin/debug/wps_api.lib
	install -pD $(ENR_DIR)/bin/debug/wps_api_test.exe release/Bcm/win32/bin/debug/wps_api_test.exe
	install -pD $(ENR_DIR)/bin/debug/WpsWizard.exe release/Bcm/win32/bin/debug/WpsWizard.exe
	install -pD $(ENR_DIR)/bin/release/wps_api.dll release/Bcm/win32/bin/release/wps_api.dll
	install -pD $(ENR_DIR)/bin/release/wps_api.lib release/Bcm/win32/bin/release/wps_api.lib
	install -pD $(ENR_DIR)/bin/release/wps_api_test.exe release/Bcm/win32/bin/release/wps_api_test.exe
	install -pD $(ENR_DIR)/bin/release/WpsWizard.exe release/Bcm/win32/bin/release/WpsWizard.exe
	install -pD $(ENR_DIR)/bin/debug/x64/wps_api.dll release/Bcm/win32/bin/debug/x64/wps_api.dll
	install -pD $(ENR_DIR)/bin/debug/x64/wps_api.lib release/Bcm/win32/bin/debug/x64/wps_api.lib
	install -pD $(ENR_DIR)/bin/debug/x64/wps_api_test.exe release/Bcm/win32/bin/debug/x64/wps_api_test.exe
	install -pD $(ENR_DIR)/bin/debug/x64/WpsWizard.exe release/Bcm/win32/bin/debug/x64/WpsWizard.exe
	install -pD $(ENR_DIR)/bin/release/x64/wps_api.dll release/Bcm/win32/bin/release/x64/wps_api.dll
	install -pD $(ENR_DIR)/bin/release/x64/wps_api.lib release/Bcm/win32/bin/release/x64/wps_api.lib
	install -pD $(ENR_DIR)/bin/release/x64/wps_api_test.exe release/Bcm/win32/bin/release/x64/wps_api_test.exe
	install -pD $(ENR_DIR)/bin/release/x64/WpsWizard.exe release/Bcm/win32/bin/release/x64/WpsWizard.exe
	tar cpf - $(TAR_SKIPCMD) -C $(ENR_DIR) WpsWizard/ wps_api_test/ | \
		tar xpf - -C release/Bcm/win32
	$(call REMOVE_WARN_PARTIAL_BUILD,release/Bcm)
	zip -r release/wps_sdk-$(subst .,-,$(RELNUM)).zip release/Bcm/
	rm -rf release/Bcm/*
	mv release/wps_sdk-$(subst .,-,$(RELNUM)).zip release/Bcm/
	# Next copy over installshield package
	$(call WARN_PARTIAL_BUILD,release/Bcm)
	install -pD src/tools/install/WPSWizard/is_bin/Bcm/WpsSetup.exe \
		    release/Bcm/Bcm_InstallShield/WpsSetup.exe
	$(call REMOVE_WARN_PARTIAL_BUILD,release/Bcm)
	@$(MARKEND)

build_clean: release
	@$(MARKSTART)
	-@find src components -type f -name "*.obj" -o -name "*.OBJ" -o -name "*.o" | \
		xargs rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)
	

.PHONY: release build_enrollee build_install checkout mogrify build_include build_end FORCE

#
# Common makefile to build USB DHD drivers, app and package them
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
#
# $Id$
#

# HOW THIS MAKEFILE WORKS (This makefile can NOT be used by itself)
#
# 1. Checkout source from SVN to src/. The directories and modules to
# check out are in $(HNDSVN_BOM).
#
# 2. Run the transmogrifier on all source code to remove some selected
# defines and to force the expression of others. Also some comments
# undefined by the transmogrifier are in $(UNDEFS). The symbols to be
# defined by the transmogrifier are in $(DEFS).
#
# 3. Build binaries include both trapapp and driver
#
# 4. copy binary files to release folder
#

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

## Caller makefile defines BRAND, BUILD_TYPES variables
DATE               := $(shell date -I)
BUILD_BASE         := $(shell pwd)
RELEASEDIR         := $(BUILD_BASE)/release

export BRAND_RULES       = brand_common_rules.mk
export MOGRIFY_RULES     = mogrify_common_rules.mk

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

MOGRIFY             = perl src/tools/build/mogrify.pl $(patsubst %,-U%,$(UNDEFS)) $(patsubst %,-D%,$(DEFS))
SHELL              := bash.exe
MAKE_MODE          := unix
NULL               := /dev/null

ifdef BCM_MFGTEST
  export WLTEST    := 1
endif # BCM_MFGTEST

HNDRTE_DEPS           := checkout
HNDRTE_IMGFN          := rtecdc.h
# Dual band image (default)
EMBED_DONGLE_IMAGE     :=
# Single band image (only mfgtest needs this)
EMBED_DONGLE_IMAGE_SB  =
ALL_DNGL_IMAGES        = $(EMBED_DONGLE_IMAGE) $(EMBED_DONGLE_IMAGE_SB)
ALL_DNGL_HEADERS       = $(subst /,,$(foreach image,$(ALL_DNGL_IMAGES),rtecdc_$(word 1,$(subst -,$(space),$(dir $(image)))).h))

# Win dhd drivers
OBJTYP := $(if $(findstring win_internal_dongle_usb,$(BRAND)),objchk,objfre)

VISTA_x86_DHDDRIVER = src/dhd/wdm/buildusbvista/$(OBJTYP)_wlh_x86/i386/bcmusbdhdlh.sys
VISTA_x64_DHDDRIVER = src/dhd/wdm/buildusbvista/$(OBJTYP)_wlh_amd64/amd64/bcmusbdhdlh64.sys
XP_x86_DHDDRIVER    = src/dhd/wdm/buildusbxp/$(OBJTYP)_wxp_x86/i386/bcmusbdhdxp.sys
XP_x64_DHDDRIVER    = src/dhd/wdm/buildusbxp/$(OBJTYP)_wnet_amd64/amd64/bcmusbdhdxp64.sys

PRE_RELEASE_DEPS   :=
PROJECT_CONFIGS_GENERIC := 'Release|win32' 'Release|x64'

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

# Verisign server has occasional network issues
#export SIGN_TIMESTAMP_URL ?=http://timestamp.verisign.com/scripts/timstamp.dll
# geotrush isn't recognized by all instances of signtool.exe
#export SIGN_TIMESTAMP_URL  ?=https://timestamp.geotrust.com/tsa
# This is another one to try as back for verisign server
export SIGN_TIMESTAMP_URL  ?=http://timestamp.globalsign.com/scripts/timstamp.dll

# Arg listing: $1=Driver-folder $2=CPU $3=catalog-file $4=driver-name
# Arg listing: $3 and $4 are optional and are derived from $2
define RELEASE_SIGN_DRIVER
	@echo "#- $0"
	$(MAKE) -C src/dhd/wdm release_sign_driver \
		SIGN_DIR=$1 SIGN_ARCH=$2 WINOS=$3 \
		RELNUM="$(RELNUM)" RELDATE="$(RELDATE)"
endef # RELEASE_SIGN_DRIVER

# These are module names and directories that will be checked out of CVS.
HNDSVN_BOM     := hndrte.sparse

# These are EFFECTIVE only for host-side compilation
UNDEFS += CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
          CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
          CONFIG_BCM93725 BCM93725B BCM94100 NICMODE ETSIM \
          INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS \
          __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 \
          CONFIG_BCM4710 CONFIG_MIPS_BRCM DSLCPE LINUXSIM BCMSIM \
          BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 BCMUSBDEV BCMCCX BCMEXTCCX  \
          BCMSDIODEV BCMSDIO WLFIPS BCMP2P

UNDEFS += DEBUG_LOST_INTERRUPTS BCMQT BCMSLTGT

include unreleased-chiplist.mk

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS + = BCM47XX BCM47XX_CHOPS BCMDHDUSB

# These are extensions of source files that should be transmogrified
# (using the parameters specified in DEFS and UNDEFS above.)
#
MOGRIFY_EXT = $(COMMON_MOGRIFY_FILETYPES)

all: build_start checkout mogrify copy_dongle_images build_dhd build_apps release build_end

include linux-dongle-image-launch.mk
include $(MOGRIFY_RULES)
include $(BRAND_RULES)

# Additionally copy this from dongle image build
# ALL_DNGL_HEADERS list is generated dynamically based on ALL_DNGL_IMAGES
DNGL_IMG_FILES += $(ALL_DNGL_HEADERS)

# check out files
checkout : $(CHECKOUT_TGT)

build_start:
	@$(MARKSTART_BRAND)

define MOGRIFY_LIST
	/usr/bin/find src components $(MOGRIFY_EXCLUDE) -type f -print  |  \
		perl -ne 'print if /($(subst $(space),|,$(foreach ext,$(MOGRIFY_EXT),$(ext))))$$/i' > \
		src/.mogrified
endef

build_include: checkout mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

build_apps:
	@$(MARKSTART)
	@echo " -- MARK build apps --"
	$(MAKE) -C src/wl/exe
ifdef BCM_MFGTEST
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlu_dll WLTEST=1 mfg
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll WLTEST=1
	$(MAKE) -C src/wl/exe/win7 WLTEST=1 \
		PROJECT_CONFIGS="$(PROJECT_CONFIGS_GENERIC)"
else  # BCM_MFGTEST
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll
endif # BCM_MFGTEST

	@$(MARKEND)

# TODO: Convert this dhd build similar to linux-dhd.mk
build_dhd: checkout mogrify build_include copy_dongle_images
	@$(MARKSTART)
	@echo " -- MARK build driver --"
	$(MAKE) -C src/dhd/wdm build_usb_xp_driver build_usb_vista_driver \
		   generate_infs
ifdef BCM_MFGTEST
	$(MAKE) -C src/dhd/wdm XPBLDDIR=buildusbxpsb \
		   WDK_REGEN_BUILDSPACE=1 \
		   EMBED_IMG_NAME="$(EMBED_DONGLE_IMAGE_SB)" \
		   build_usb_xp_driver \
		   generate_infs
endif # BCM_MFGTEST
	@$(MARKEND)

clean:
	@$(MARKSTART)
	$(MAKE) -C src/include clean
	$(MAKE) -C src/wl/exe clean
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll clean
	@$(MARKEND)

release: release_xp release_vista

## Win XP release packaging after everything is built
release_xp: $(RELEASEDIR)/WinXP/BcmDHD
	@$(MARKSTART)
	-install src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

# Win Vista
$(RELEASEDIR)/WinXP/BcmDHD:: \
			$(RELEASEDIR)/WinXP/BcmDHD/Bcm_USB_DriverOnly \
			$(RELEASEDIR)/WinXP/BcmDHD/Bcm_Apps

ifdef BCM_MFGTEST
$(RELEASEDIR)/WinXP/BcmDHD:: \
			$(RELEASEDIR)/WinXP/BcmDHD/Bcm_USB_DriverOnly_SingleBand
endif # BCM_MFGTEST

# Win XP - dual band driver package
$(RELEASEDIR)/WinXP/BcmDHD/Bcm_USB_DriverOnly :: FORCE $(PRE_RELEASE_DEPS)
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	install $(XP_x86_DHDDRIVER) $@/
	install $(XP_x64_DHDDRIVER) $@/
	install -p src/dhd/wdm/build/bcmusbdhdxp.inf $@/
	src/tools/release/wustamp -o -v "$(RELNUM)" -d "$(RELDATE)" \
		"`cygpath -w $(@)`"
ifeq ($(strip $(BRAND)),win_internal_dongle_usb)
	install -p $(subst .sys,.pdb,$(XP_x86_DHDDRIVER)) $@/
	install -p $(subst .sys,.map,$(XP_x86_DHDDRIVER)) $@/
	install -p $(subst .sys,.pdb,$(XP_x64_DHDDRIVER)) $@/
	install -p $(subst .sys,.map,$(XP_x64_DHDDRIVER)) $@/
endif
	-install -p src/doc/ReleaseNotesWinMfgtestSdio.txt $@/README.txt
	$(call RELEASE_SIGN_DRIVER,$@,X86,WINXP)
	$(call RELEASE_SIGN_DRIVER,$@,X64,WINXP)
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

ifdef BCM_MFGTEST
# Win XP - single band driver package
$(RELEASEDIR)/WinXP/BcmDHD/Bcm_USB_DriverOnly_SingleBand :: FORCE $(PRE_RELEASE_DEPS)
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	install $(subst usbxp/,usbxpsb/,$(XP_x86_DHDDRIVER)) $@/
	install $(subst usbxp/,usbxpsb/,$(XP_x64_DHDDRIVER)) $@/
	install -p src/dhd/wdm/build/bcmusbdhdxp.inf $@/
	src/tools/release/wustamp -o -v "$(RELNUM)" -d "$(RELDATE)" \
		"`cygpath -w $(@)`"
	-install -p src/doc/ReleaseNotesWinMfgtestSdio.txt $@/README.txt
	$(call RELEASE_SIGN_DRIVER,$@,X86,WINXP)
	$(call RELEASE_SIGN_DRIVER,$@,X64,WINXP)
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)
endif # BCM_MFGTEST

# Win XP
$(RELEASEDIR)/WinXP/BcmDHD/Bcm_Apps :: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
ifeq ($(strip $(BRAND)),win_external_dongle_usb)
	install -p src/wl/exe/windows/winxp/obj/free/wl.exe    $@/
	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.lib        $@/
	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.dll        $@/
else
	install -p src/wl/exe/windows/winxp/obj/checked/wl.exe $@/
endif
ifdef BCM_MFGTEST
	install -p src/wl/exe/windows/winxp/obj/mfg_dll/free/brcm_wlu.dll $@/
	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.lib        $@/
	install -p src/wl/exe/windows/win7/obj/wlm/free/wlm.dll        $@/
endif
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

## Win Vista release packaging after everything is built
release_vista: $(RELEASEDIR)/WinVista/BcmDHD
	@$(MARKSTART)
	-install src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)
#
# Release the BCM branded files.  These are really 'unbranded'
# by definition, but some customers may get these files and just
# live with that fact that they have Broadcom logos and copyrights.
#
# Win Vista
$(RELEASEDIR)/WinVista/BcmDHD:: \
			$(RELEASEDIR)/WinVista/BcmDHD/Bcm_USB_DriverOnly \
			$(RELEASEDIR)/WinVista/BcmDHD/Bcm_Apps

#ifdef BCM_MFGTEST
#$(RELEASEDIR)/WinVista/BcmDHD:: \
#		$(RELEASEDIR)/WinVista/BcmDHD/Bcm_USB_DriverOnly_SingleBand
#endif # BCM_MFGTEST

# Win Vista - dual band driver package
$(RELEASEDIR)/WinVista/BcmDHD/Bcm_USB_DriverOnly :: FORCE $(PRE_RELEASE_DEPS)
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
	install $(VISTA_x86_DHDDRIVER) $@/
	install $(VISTA_x64_DHDDRIVER) $@/
	install -p src/dhd/wdm/build/bcmusbdhdvista.inf $@/
	-install -p src/doc/ReleaseNotesWinMfgtestSdio.txt $@/README.txt
ifeq ($(strip $(BRAND)),win_internal_dongle_usb)
	install -p $(subst .sys,.pdb,$(VISTA_x86_DHDDRIVER)) $@/
	install -p $(subst .sys,.map,$(VISTA_x86_DHDDRIVER)) $@/
	install -p $(subst .sys,.pdb,$(VISTA_x64_DHDDRIVER)) $@/
	install -p $(subst .sys,.map,$(VISTA_x64_DHDDRIVER)) $@/
endif
	$(call RELEASE_SIGN_DRIVER,$@,X86,WINVISTA)
	$(call RELEASE_SIGN_DRIVER,$@,X64,WINVISTA)
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

#ifdef BCM_MFGTEST
# Win Vista - single band driver package
#$(RELEASEDIR)/WinVista/BcmDHD/Bcm_USB_DriverOnly_SingleBand :: FORCE $(PRE_RELEASE_DEPS)
#	mkdir -p $@
#	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
#	install $(subst usbvista/,usbvistasb/,$(VISTA_x86_DHDDRIVER)) $@/
#	install $(subst usbvista/,usbvistasb/,$(VISTA_x64_DHDDRIVER)) $@/
#	install -p src/dhd/wdm/bcmusbdhd.inf $@/
#	-install -p src/doc/ReleaseNotesWinMfgtestSdio.txt $@/README.txt
#	$(call RELEASE_SIGN_DRIVER,$@,X86,WINVISTA)
#	$(call RELEASE_SIGN_DRIVER,$@,X64,WINVISTA)
#	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
#endif # BCM_MFGTEST

# Win Vista
$(RELEASEDIR)/WinVista/BcmDHD/Bcm_Apps :: FORCE
	@$(MARKSTART)
	mkdir -p $@
	touch $@/_PARTIAL_CONTENTS_DO_NOT_USE
ifeq ($(strip $(BRAND)),win_external_dongle_usb)
	install -p src/wl/exe/windows/win7/obj/winvista/free/wl.exe $@/
else
	install -p src/wl/exe/windows/win7/obj/winvista/checked/wl.exe $@/
ifneq ($(BCM_MFGTEST),)
	mkdir -p $@/x64
	install -p src/wl/exe/windows/win7/obj/mfg_dll/Release/brcm_wlu.dll      $@/
	install -p src/wl/exe/windows/win7/obj/x64/mfg_dll/Release/brcm_wlu.dll  $@/x64
endif # BCM_MFGTEST
endif
	rm -f $@/_PARTIAL_CONTENTS_DO_NOT_USE
	@$(MARKEND)

#
# Convenience pseudo-targets
#
# Run mogrifier
mogrify: checkout
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

#pragma runlocal
build_clean: release
	@$(MARKSTART)
	-@find src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	rm -f $(HNDRTE_FLAG)
	@$(MARKEND_BRAND)

.PHONY: release release_vista build_dhd checkout mogrify build_include build_end FORCE

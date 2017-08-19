# Build the windows wireless drivers, app, tools, installer and package them
#
# $Id$
#
# HOW THIS MAKEFILE WORKS
#
# The release consists of 6 stages:
#
# 1. Checkout source from SVN to src/. The directories and modules to
# check out are in $(HNDSVN_BOM).
#
# 2. Run the transmogrifier on all source code to remove some selected
# defines and to force the expression of others. Also some comments
# undefined by the transmogrifier are in $(UNDEFS). The symbols to be
# defined by the transmogrifier are in $(DEFS).
#
# 3. Build binaries include both trayapp and driver
#
# 4. copy binary files to release folder
#
# Besides build target [all], there are two convenient targets [trayapp], [driver]
# to save time for testing build. They SHOULD NOT be released.

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

PARENT_MAKEFILE :=
default: all

DATE             = $(shell date -I)
BUILD_BASE       = $(CURDIR)
RELEASEDIR       = $(BUILD_BASE)/release
SHELL            = bash.exe
FIND             = /bin/find
INSTALL         := install -p
MKDIR           := mkdir -pv
MAKE_MODE       := unix
NULL            := /dev/null
BRAND           ?= win10_dhdsdio_internal_wl
RETRYCMD        ?= $(firstword \
                      $(wildcard \
                       Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                   ))

VS_VER            ?= 2015
VERSION_NAME      ?= Win10
WIN10_WDK_VERSION   ?= 10240
WIN10_WDK_INSTALL   ?= $(WIN10_WDK_VERSION)wdksdk
WIN10_WDK_LOCAL     ?= c:/tools/msdev
WIN10_WDK_GLOBAL    ?= z:/projects/hnd/tools/win/msdev
WIN10_WDK_ROOT      := $(firstword \
                         $(wildcard $(WIN10_WDK_LOCAL)/$(WIN10_WDK_INSTALL) \
                                    $(WIN10_WDK_GLOBAL)/$(WIN10_WDK_INSTALL) ) \
                         Makefile-Did-Not-Find-$(WIN10_WDK_INSTALL)-Error)
export WIN10_WDK_ROOT

UPPER_VERSION_NAME = $(subst $(space),$(empty), $(call .toupper, $(VERSION_NAME)))
LOWER_VERSION_NAME = $(subst $(space),$(empty), $(call .tolower, $(VERSION_NAME)))

WINOS = $(UPPER_VERSION_NAME)

OEM                   ?= Bcm
OEM_LIST_WIN10_RLSPKG ?= Bcm
OEM_LIST              ?= bcm
export OEM_LIST
BUILD_ARCHS           ?= x86 x64

FN2_PATH ?= components/drivers/bcmfn2
FN2_CHECKOUT_DIR = .
FN2_DRIVER_NAME = bcmfn2
#FN2_BUILD_CONFIG = Win10 Debug
FN2_BUILD_CONFIG = WinThresh Debug

# Make target start and end markers
MARKSTART    = date +"[%D %T] MARK-START: $@"
MARKEND      = date +"[%D %T] MARK-END  : $@"

# Whole build brand start and end markers. BRAND is defined
# mandatorily by build scripts and TAG is optional
MARKSTART_BRAND = date +"[%D %T] MARK-START: BRAND=$(BRAND) $(if $(TAG),TAG=$(TAG))"
MARKEND_BRAND   = date +"[%D %T] MARK-END  : BRAND=$(BRAND) $(if $(TAG),TAG=$(TAG))"

export MOGRIFY_RULES = mogrify_common_rules.mk

SIGN_OS      = WIN8X # $(UPPER_VERSION_NAME)
DRIVER_NAME  = bcmdhd63
#DRIVER_NAME  = bcmdhd64
DRIVER_INF   = bcmwdidhdsdio

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
RELNUM  := $(maj).$(min).$(rcnum).$(incremental)
RELDATE := $(shell date '+%m/%d/%Y')

# What shall be done before firmware build. Empty because firmware build
# started in secondary make, when all prerequisites are already built
HNDRTE_DEPS :=

# On top-level invocation win8x-dhdsdio-config.mk is not yet checked out, so it is not included here
# On second and third level invocations this file shall exist
ifneq ($(wildcard src/dhd/wdm/win8x-dhdsdio-config.mk),)
  # This file shall define the following variables: FIRMWARE_IMAGES, DRIVER_FILES_INTEL,
  # DRIVER_FILES_ARM, DRIVER_FILES_OTHER - see comments in file for more information
  include src/dhd/wdm/win8x-dhdsdio-config.mk
endif
HNDRTE_IMGFN = rtecdc.bin

FIRMWARE_NAME_SUFFIX = rtecdc.bin
FIRMWARE_FILES       = $(foreach img, $(FIRMWARE_IMAGES), $(word 1,$(subst -,$(space),$(word 3,$(subst .,$(space),$(img)))))$(FIRMWARE_NAME_SUFFIX))

# Checkout these BOM files. If any conditionals are needed, they are
# defined with -defs flag
HNDSVN_BOM  := wl-build.sparse

UNDEFS= # CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 CONFIG_BCM4710 CONFIG_MIPS_BRCM POCKET_PC BCMINTERNAL DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 WLMOTOROLALJ

# These symbols will be DEFINED in the source code by the transmogrifier
#
DEFS= # BCM47XX BCM47XX_CHOPS

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

SIGNTOOL_TIME  := "http://timestamp.verisign.com/scripts/timstamp.dll"
SIGNTOOL_OPTS  ?= /a /v /s my /n "Broadcom Corporation" /t $(SIGNTOOL_TIME)
SIGNTOOL_CMND_32  := $(RETRYCMD) $(WIN10_WDK_ROOT)/bin/x86/signtool sign $(SIGNTOOL_OPTS)
SIGNTOOL_CMND_64  := $(RETRYCMD) $(WIN10_WDK_ROOT)/bin/x64/signtool sign $(SIGNTOOL_OPTS)
export SIGNTOOL_OPTS

# Default installshield is used for all modules is still IS 12, but main
# installer upgraded to IS 2009
ifeq ($(origin ISSAB_ROOT), undefined)
    ISSAB_ROOT = $(firstword $(wildcard c:/tools/InstallShield12SAB d:/tools/InstallShield12SAB z:/projects/hnd/tools/win/InstallShield12SAB))
    ISSAB12_ROOT = $(ISSAB_ROOT)
endif
ifeq ($(origin ISSAB2009_ROOT), undefined)
    ISSAB2009_ROOT = $(firstword $(wildcard c:/tools/InstallShield2009SAB d:/tools/InstallShield2009SAB z:/projects/hnd/tools/win/InstallShield2009SAB))
endif
export IS12_ROOT  := $(ISSAB_ROOT)
export IS2009_ROOT:= $(ISSAB2009_ROOT)
TREECMD      := $(firstword $(wildcard c:/tools/utils/tree.exe d:/tools/utils/tree.exe z:/projects/hnd/tools/win/utils/tree.exe)) -a -s -D -f
INS_DIR      := components/apps/windows/install/app
INF_DIR      := $(INS_DIR)/installshield/BCMAppInstaller/is_bin
BIN_DIR      := components/shared/resources/tools/bin
WARN_FILE    := _WARNING_PARTIAL_BUILD_DO_NOT_USE
WIN10_WLINF   = bcmwl63.inf

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $@/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $@/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# Copy and timestamp inf files
define INSTALL_INF
	@echo "#- $0"
	$(INSTALL) $1 $2
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d $(RELDATE) "`cygpath -w $2`"
endef  # INSTALL_INF

# Signing several drivers
# Arg listing: $1=Driver-folder $2=CPU for signtool, $3=space-separated driver@cat list
define RELEASE_SIGN_WIN10_DRIVERS
	@echo "#- $0($1,$2,$3)"
	$(MAKE) -C src/dhd/wdm release_sign_driver \
		SIGN_DIR=$(strip $1) SIGN_ARCH=$(strip $2) \
		SIGN_DRIVERCAT="$(strip $3)" WINOS=$(SIGN_OS) WDK_VER=$(WIN10_WDK_VERSION) \
		RELNUM=$(RELNUM) RELDATE=$(RELDATE)
endef # RELEASE_SIGN_WIN10_DRIVERS

# Installs main (mandatory) binary files and some optional accompanying files
# Arg listing:
# $1 Destinaton directory
# $2 Source directory (with trailing slash)
# $3 Base names of main binary files
# $4 Space separated list of additional (optional) file extensions to copy
define INSTALL_BINARIES
	@echo "#- $0($1,$2,$3,$4)"
	@for name in $3; do \
		$(INSTALL) $(2)$${name} $1 || exit 1; \
	done
	-@for name in $3; do \
		for ext in $4; do \
			$(INSTALL) `echo $(2)$${name} | sed s/\.[[:alnum:]]*$$/.$${ext}/` $1; \
		done \
	done
endef #INSTALL_BINARIES

# Installs main driver's files to driver directory for particular architecture
# Arg listing:
# $1 Destinaton directory
# $2 Space-separated list of variables that contain lists of files to copy
# $3 OEM # Currently ignored, may be used for copying IHV
# $4 IHV file name suffix
# $5 .CAT file name (without path and extension)
# $6 Inf file subdirectory (relative to src/dhd/wdm)
# $7 Driver binary subdirectory (relative to src/dhd/wdm)
# $8 IHV binary subdirectory (relative to components/apps/windows/ihvfrm/ihv)
# $9 Space separated list of additional (besides sys, dll) file extensions to copy
define INSTALL_DRIVER_FILES
	@echo "#- $0($1,$2,$3,$4,$5,$6,$7,$8,$9)"
	mkdir -pv $1
	@for list in $2; do \
		$(MAKE) -f $(word 1,$(MAKEFILE_LIST)) copy_related_files LIST=$${list} \
			COPY_SRC_DIR=src/dhd/wdm COPY_DEST_DIR=$(strip $1); \
	done
	$(MAKE) -f $(word 1,$(MAKEFILE_LIST)) copy_firmware \
		COPY_SRC_DIR=src/dhd/wdm COPY_DEST_DIR=$(strip $1)
	$(INSTALL) src/dhd/wdm/empty.cat $(strip $1)$(strip $5).cat
	$(call INSTALL_BINARIES, $1, src/dhd/wdm/$(strip $7)/, $(DRIVER_NAME).sys, $9)
	$(call INSTALL_INF, src/dhd/wdm/$(strip $6)/$(DRIVER_INF).inf, $(strip $1)$(DRIVER_INF).inf)
endef #INSTALL_DRIVER_FILES
#	$(call INSTALL_BINARIES, $1, components/apps/windows/ihvfrm/ihv/$(strip $8)/$(strip $3)/, bcmihvui$(strip $4).dll bcmihvsrv$(strip $4).dll, $9)

# Installs FN2 driver's files to driver directory for particular architecture
# Arg listing:
# $1 Destinaton directory
# $2 Binary subdirectory (relative to FN2_PATH bcmfn2\src\bcmfn2)
# $3 Space separated list of additional (besides sys) file extensions to copy
define INSTALL_FN2_DRIVER_FILES
	@echo "#- $0($1,$2,$3)"
	$(call INSTALL_BINARIES, $1, $(FN2_CHECKOUT_DIR)/$(FN2_PATH)/$(strip $2)/, $(FN2_DRIVER_NAME).sys, $3)
	$(call INSTALL_INF, $(FN2_CHECKOUT_DIR)/$(FN2_PATH)/$(strip $2)/$(FN2_DRIVER_NAME).inf, $(strip $1)$(FN2_DRIVER_NAME).inf)
endef #INSTALL_FN2_DRIVER_FILES

# Installs Visual C++ redistributables
# Arg listing:
# $1 Destinaton directory
# $2 Visual Studio version (last 2 digits of year - e.g. 05, 08, 10, 12)
# $3 Redistributable type (checked or free)
# $4 Architecture (32, 64, arm (eventually))
define INSTALL_VCREDIST
	@echo "#- $0($1,$2,$3,$4)"
	mkdir -pv $1
	find $(BIN_DIR)/vs$(strip $2)/$(strip $3)/$(if $(findstring 32,$4),,$(if $(findstring 64,$4),x64,$4)) -maxdepth 1 -type f | xargs -I @ $(INSTALL) @ $1
endef  # INSTALL_VCREDIST

# $1 is <oem> $2 is <free or checked>
define	INSTALL_BCMWLS
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/bcmwls/Release/bcmwls32.exe  $@/
	$(INSTALL) $(INS_DIR)/bcmwls/x64/Release/bcmwls64.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst32.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst64.exe $@/
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/bcmwls*.exe)
endef # INSTALL_BCMWLS

# $1 is <oem>; $2 <free or checked>; $3 <type of installshield>
define INSTALL_INSTALLSHIELD_FILES
	@echo "#- $0($1,$2,$3)"
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data1.cab   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data1.hdr   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data2.cab   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/ISSetup.dll $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/_Setup.dll  $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/layout.bin  $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/setup.inx   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/Setup.exe   $@/IS.exe
	$(INSTALL) $(INS_DIR)/installshield/$1/setup.iss             $@/Setup.iss
	$(if $(wildcard $(INS_DIR)/installshield/$1/InfoPath.xml), \
	 $(INSTALL) $(INS_DIR)/installshield/$1/InfoPath.xml         $@/ )
	$(INSTALL) $(INS_DIR)/launcher/Release/Launcher.exe          $@/Setup.exe
	$(INSTALL) $(INS_DIR)/launcher/Launcher.ini                  $@/
	$(INSTALL) $(INS_DIR)/uninstall/Release/$1/bcmwlu00.exe      $@/
	$(call INSTALL_BCMWLS,$1,$2)
endef # INSTALL_INSTALLSHIELD_FILES

# Invokes vcvarsall.bat
# Arg listing:
# $1 Visual studio version (e.g. 2012)
# $2 Platform name to derive vcvars platform from. If contains 64 then x86_amd64 is used, if contains arm then x86_arm is used, otherwise x86 is used
define INVOKE_VCVARS
    C:/Tools/msdev/vs$(strip $1)/vc/vcvarsall.bat $(if $(findstring 64,$2),x86_amd64,$(if $(findstring arm,`echo $2|awk '{print tolower($$0)}'`),x86_arm,x86))
endef #INVOKE_VCVARS

# Builds solution or project using msbuild
# Arg listing:
# $1 OEM
# $2 Solution or project file directory
# $3 Solution or project file base name
# $4 Configuration to build for
# $5 Platform to build for
# $6 Visual Studio version to take environment from
define BUILD_MSBUILD
	@echo "#- $0($1,$2,$3,$4,$5,$6)"
	cmd /c "call $(call INVOKE_VCVARS,$6,$5) && \
	    cd $2 && set OEM=$1 && \
		msbuild /t:build $3 /p:Configuration=$4 /p:Platform=$5 /p:UseEnv=false"
endef #BUILD_MSBUILD

all: build_end
build_end: package
package: build \
	$(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly \
	$(RELEASEDIR)/$(VERSION_NAME)/checked/InstallShield
build: pre_build \
	build_firmware \
	build_fn2 \
	build_ihv \
	build_driver \
	build_install \
	build_app_libraries \
	$(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly \
	$(RELEASEDIR)/$(VERSION_NAME)/checked/InstallShield
pre_build: build_start checkout mogrify build_include

include linux-dongle-image-launch.mk
include $(MOGRIFY_RULES)
include unreleased-chiplist.mk

# Additionally copy this from dongle image build
DNGL_IMG_FILES += rtecdc.exe
# ALL_DNGL_HEADERS list is generated dynamically based on ALL_DNGL_IMAGES
DNGL_IMG_FILES += $(ALL_DNGL_HEADERS)

# check out files
checkout: build_start
	@$(MARKSTART)
	@echo "WARN: Temporary Win8X workaround to take out incompatible"
	@echo "WARN: src/wl/sys/ntddndis.h until other non-driver pieces"
	@echo "WARN: are all fixed in build"
	rm -fv src/wl/sys/ntddndis.h
	@$(MARKEND)

build_start:
	@$(MARKSTART_BRAND)

# All exported MOGRIFY_xxx variables are referenced by following step
mogrify: checkout
	@$(MARKSTART)
#disabled#	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

build_include: checkout mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

# Building firmware images. This is tricky, nonconventional, 3-level process
#
# build_firmware is a top-level target. When it is being read is not yet known
# (as src/dhd/wdm/win8xslate-config.mk not yet checked out). So it is executed
# after checkout by recursively calling this makefile with build_firmware2 target
#
# build_firmware2 by itself can't build firmware, because images are built
# off other branches. Thus it makes another recursive call, specifying as TAG
# firmware branch names

# Target for top-level make invocation
build_firmware:
	$(MAKE) -f $(word 1,$(MAKEFILE_LIST)) build_firmware2

# Target for second-level make invocation
build_firmware2: $(FIRMWARE_IMAGES)

# Evaluated on second level of make invocation, cause third-level invocations with different TAGs
$(FIRMWARE_IMAGES): FIRMWARE_TAG=$(word 1,$(subst .,$(space),$@))
$(FIRMWARE_IMAGES): FIRMWARE_DIR=$(word 2,$(subst .,$(space),$@))
$(FIRMWARE_IMAGES): FIRMWARE_IMAGE=$(word 3,$(subst .,$(space),$@))
$(FIRMWARE_IMAGES): DEST_PREFIX=$(word 1,$(subst -,$(space),$(word 3,$(subst .,$(space),$@))))
$(FIRMWARE_IMAGES): checkout
$(FIRMWARE_IMAGES):
	@$(MARKSTART)
	$(MAKE) -f $(word 1,$(MAKEFILE_LIST)) TAG=$(FIRMWARE_TAG) BRAND=$(TAG)_$(BRAND) ALL_DNGL_IMAGES=$(FIRMWARE_IMAGE) HNDRTE_PRIVATE_DIR=$(DEST_PREFIX) DNGL_IMGDIR=$(FIRMWARE_DIR) copy_dongle_images
	cp $(FIRMWARE_DIR)/$(FIRMWARE_IMAGE)/rtecdc.bin src/dhd/wdm/$(DEST_PREFIX)$(FIRMWARE_NAME_SUFFIX)
	@$(MARKEND)

# Used at second-level of make invocation, as at first level FIRMWARE_FILES is not known
copy_firmware:
	@for image in $(FIRMWARE_FILES); do \
		cp -v $(COPY_SRC_DIR)/$${image} $(COPY_DEST_DIR); \
	done

# Used at second-level of make invocation, as at first level list variables are not known
# Copy only the files which exist and give no error for the rest.
.PHONY: copy_related_files
copy_related_files:
	@for file in $(wildcard $(addprefix $(COPY_SRC_DIR)/,$($(LIST)))); do \
		cp -v $${file} $(COPY_DEST_DIR); \
	done

build_fn2: checkout mogrify
	@$(MARKSTART)
	$(MAKE) -C $(FN2_CHECKOUT_DIR)/$(FN2_PATH) WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WIN10_WDK_VERSION) BUILD_ARCHS="$(BUILD_ARCHS)" CONFIGURATION="$(FN2_BUILD_CONFIG)"
	@$(MARKEND)

build_driver: build_firmware
build_driver: pre_build
	@$(MARKSTART)
	$(MAKE) -C src/dhd/wdm WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WIN10_WDK_VERSION) BUILD_TYPES=checked BUS=sdio BUILD_ARCHS="$(BUILD_ARCHS)" -f win8xdriver.mk all
	$(MAKE) -C src/dhd/wdm WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WIN10_WDK_VERSION) BUILD_TYPES=checked BUS=pcie BUILD_ARCHS="$(BUILD_ARCHS)" -f win8xdriver.mk all
	$(MAKE) -C src/wl/exe WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WIN10_WDK_VERSION) BUILD_CONFS=Debugwb -f wl_vs13.mk all
	$(MAKE) -C src/dhd/exe WINOS=$(WINOS) VS_VER=$(VS_VER) WDK_VER=$(WIN10_WDK_VERSION) BUILD_CONFS=Debug -f dhd_vs13.mk all
	@$(MARKEND)

build_ihv: pre_build
	@$(MARKSTART)
	$(MAKE) -C components/apps/windows/ihvfrm/ihv VS_VER=$(VS_VER) WDK_VER=$(WIN10_WDK_VERSION) BUILD_CONFS=Debugw15 BUILD_OEMS=Bcm -f build_vs15_win10.mk
	@$(MARKEND)

build_install: export VS_PRE_BUILD_ENABLED=0
build_install: VERBOSE:=1
build_install: pre_build
	@$(MARKSTART)
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND)
	@$(MARKEND)

build_app_libraries: pre_build
	@$(MARKSTART)
	$(call BUILD_MSBUILD,bcm,components\apps\windows\libraries,WLWin8_VS15.sln,Debugw15,Win32,2015)
	$(call BUILD_MSBUILD,bcm,components\apps\windows\libraries,WLWin8_VS15.sln,Debugw15,x64,2015)
	@$(MARKEND)

pre_build: checkout
	@$(MARKSTART)
	mkdir -pv release/$(VERSION_NAME)/checked
	@$(MARKEND)

## Win8X release packaging after everything is built
package:
	@$(MARKSTART)
	#-$(INSTALL) src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

$(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -pv $@
	$(call WARN_PARTIAL_BUILD)
	$(call INSTALL_DRIVER_FILES, $@/x86/, DRIVER_FILES_INTEL DRIVER_FILES_OTHER, Bcm, , $(DRIVER_NAME), inf, wdi_dhd_sdio_checked/Win32, Debugw15, map pdb)
	$(call INSTALL_DRIVER_FILES, $@/x64/, DRIVER_FILES_INTEL DRIVER_FILES_OTHER, Bcm, 64, $(DRIVER_NAME), inf, wdi_dhd_sdio_checked/x64, x64/Debugw15, map pdb)
#	$(call INSTALL_BINARIES, $@/x86/, src/wl/exe/Debugwb/, wl.exe, map pdb)
#	$(call INSTALL_BINARIES, $@/x86/, src/dhd/exe/Debug/, dhd.exe, map pdb)
#	$(call INSTALL_BINARIES, $@/x64/, src/wl/exe/x64/Debugwb/, wl.exe, map pdb)
#	$(call INSTALL_BINARIES, $@/x64/, src/dhd/exe/x64/Debug/, dhd.exe, map pdb)
	$(call INSTALL_BINARIES, $@/x86/, components/apps/windows/ihvfrm/ihv/Debugw15/, wapiutil.exe, pdb)
	$(call INSTALL_BINARIES, $@/x64/, components/apps/windows/ihvfrm/ihv/x64/Debugw15/, wapiutil.exe, pdb)
	$(call INSTALL_FN2_DRIVER_FILES, $@/x86/, $(subst $(space),$(empty),$(FN2_BUILD_CONFIG)), pdb)
	$(call INSTALL_FN2_DRIVER_FILES, $@/x64/, x64/$(subst $(space),$(empty),$(FN2_BUILD_CONFIG)), pdb)
	$(call RELEASE_SIGN_WIN10_DRIVERS, $@/x86, X86, $(DRIVER_NAME).sys@$(DRIVER_NAME).cat $(FN2_DRIVER_NAME).sys@$(FN2_DRIVER_NAME).cat)
	$(call RELEASE_SIGN_WIN10_DRIVERS, $@/x64, X64, $(DRIVER_NAME).sys@$(DRIVER_NAME).cat $(FN2_DRIVER_NAME).sys@$(FN2_DRIVER_NAME).cat)
#InfBroken	$(call RELEASE_SIGN_WIN10_DRIVERS, $@/arm, ARM, $(DRIVER_NAME).sys@$(DRIVER_NAME).cat $(FN2_DRIVER_NAME).sys@$(FN2_DRIVER_NAME).cat)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)

$(RELEASEDIR)/$(VERSION_NAME)/checked/InstallShield: build FORCE
	@$(MARKSTART)
	mkdir -pv $@
	rm -rf $@/*
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/x64
	mkdir -pv $@/Drivers/$(VERSION_NAME)/WL/x64

#disabled#	$(call INSTALL_INF, $(INF_DIR)/bcm/$(WIN10_WLINF), $@/Drivers/$(VERSION_NAME)/WL)
	$(call INSTALL_DRIVER_FILES, $@/Drivers/$(VERSION_NAME)/WL/, DRIVER_FILES_INTEL, Bcm, , $(DRIVER_NAME), inf, wdi_dhd_sdio_checked/Win32, Debugw15, map pdb)
#	$(call INSTALL_BINARIES, $@/Drivers/$(VERSION_NAME)/WL/, components/apps/windows/ihvfrm/ihv/Debugw15/, wapiutil.exe, map pdb)
	$(call INSTALL_FN2_DRIVER_FILES, $@/Drivers/$(VERSION_NAME)/WL/, $(subst $(space),$(empty),$(FN2_BUILD_CONFIG)), pdb)
	$(call RELEASE_SIGN_WIN10_DRIVERS, $@/Drivers/$(VERSION_NAME)/WL, X86, $(DRIVER_NAME).sys@$(DRIVER_NAME).cat $(FN2_DRIVER_NAME).sys@$(FN2_DRIVER_NAME).cat)
	$(call INSTALL_DRIVER_FILES, $@/Drivers/$(VERSION_NAME)/WL/x64/, DRIVER_FILES_INTEL, Bcm, 64, $(DRIVER_NAME), inf, wdi_dhd_sdio_checked/x64, x64/Debugwb, map pdb)
#	$(call INSTALL_BINARIES, $@/Drivers/$(VERSION_NAME)/WL/x64/, components/apps/windows/ihvfrm/ihv/x64/Debugw15/, wapiutil.exe, pdb)
	$(call INSTALL_FN2_DRIVER_FILES, $@/Drivers/$(VERSION_NAME)/WL/x64/, x64/$(subst $(space),$(empty),$(FN2_BUILD_CONFIG)), pdb)
	$(call RELEASE_SIGN_WIN10_DRIVERS, $@/Drivers/$(VERSION_NAME)/WL/x64, X64, $(DRIVER_NAME).sys@$(DRIVER_NAME).cat $(FN2_DRIVER_NAME).sys@$(FN2_DRIVER_NAME).cat)
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,checked,BCMAppInstaller)
	$(INSTALL) $(INS_DIR)/ini_win10_dhdsdio/bcm/checked/driver.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini_win10_dhdsdio/bcm/checked/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup_win10_dhdsdio.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)

build_clean: package
	@$(MARKSTART)
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

.PHONY: build_ihv build_driver build_firmware build_firmware2 copy_firmware copy_dongle_images checkout mogrify build pre_build package build_include build_status build_end $(FIRMWARE_IMAGES) build_install build_app_libraries FORCE

################################################################################
## Simple text substitution utilities to avoid excessive $(shell ) calls.
## Only usable for = and not := due to position at the end of this file
################################################################################

.tolower = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))
.toupper = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

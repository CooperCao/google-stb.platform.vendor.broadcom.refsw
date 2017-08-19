# Build the windows wireless drivers, app, tools, installer and package them
#
# $Id: win8x_dhdpcie_external_wl.mk 366774 2012-11-05 06:46:14Z $
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
DEFAULT_TARGET  := default
ALL_TARGET      := all

$(DEFAULT_TARGET): $(ALL_TARGET)

DATE                = $(shell date -I)
BUILD_BASE          = $(shell pwd)
RELEASEDIR          = $(BUILD_BASE)/release
SHELL               = bash.exe
FIND                = /bin/find
INSTALL            := install -p
MKDIR              := mkdir -pv
MAKE_MODE          := unix
NULL               := /dev/null
BRAND              ?= win8x_dhdpcie_external_wl
RETRYCMD           ?= $(firstword \
                      $(wildcard \
                       Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                   ))

VS_VER              ?= 2013
VERSION_NAME        ?= Win8X
WIN8X_WDK_VERSION   ?= 9600
WIN8X_WDK_INSTALL   ?= $(WIN8X_WDK_VERSION)wdksdk
WIN8X_WDK_LOCAL     ?= c:/tools/msdev
WIN8X_WDK_GLOBAL    ?= z:/projects/hnd/tools/win/msdev
WIN8X_WDK_ROOT      := $(firstword \
                         $(wildcard $(WIN8X_WDK_LOCAL)/$(WIN8X_WDK_INSTALL) \
                                    $(WIN8X_WDK_GLOBAL)/$(WIN8X_WDK_INSTALL) ) \
                         Makefile-Did-Not-Find-$(WIN8X_WDK_INSTALL)-Error)
export WIN8X_WDK_ROOT

UPPER_VERSION_NAME = $(subst $(space),$(empty), $(call .toupper, $(VERSION_NAME)))
LOWER_VERSION_NAME = $(subst $(space),$(empty), $(call .tolower, $(VERSION_NAME)))

WINOS=$(UPPER_VERSION_NAME)

OEM_LIST_WIN8X_RLSPKG   ?=  Bcm
OEM_LIST                ?=  bcm
export OEM_LIST
BUILD_ARCHS             ?= x86 x64

ifdef BUILT_DATE
	BUILD_PARTIAL = true
	OEM_LIST_WIN8X_RLSPKG = Dell
	OEM_LIST = dell
	BUILT_DIR = /projects/hnd_swbuild/build_windows/$(TAG)/$(BRAND)/$(BUILT_DATE)/
endif

# Make target start and end markers
MARKSTART    = date +"[%D %T] MARK-START: $@"
MARKEND      = date +"[%D %T] MARK-END  : $@"

# Whole build brand start and end markers. BRAND is defined
# mandatorily by build scripts and TAG is optional
MARKSTART_BRAND = date +"[%D %T] MARK-START: BRAND=$(BRAND) $(if $(TAG),TAG=$(TAG))"
MARKEND_BRAND   = date +"[%D %T] MARK-END  : BRAND=$(BRAND) $(if $(TAG),TAG=$(TAG))"

export MOGRIFY_RULES     = mogrify_common_rules.mk

DRIVER_NAME      = bcmpciedhd63
DRIVER_INF       = bcmpciedhd63
DRIVER_TEMPLATE_INF = bcmpciedhd

NDISVER_WIN8X = 2
ARCH_WIN32 = ""
ARCH_WIN64 = 64
NDISARCH_64 = amd64
NDISARCH_32 = x86

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

# On top-level invocation win8x-dhdpcie-config.mk not yet checked out, so it doesn't included here
# On second and third level invocations this file shall exist
ifneq ($(wildcard src/dhd/wdm/win8x-dhdpcie-config.mk),)
  # This file shall define the following variables: FIRMWARE_IMAGES, DRIVER_FILES_NVRAM
  # see comments in file for more information
  include src/dhd/wdm/win8x-dhdpcie-config.mk
endif
HNDRTE_IMGFN = rtecdc.bin

FIRMWARE_NAME_SUFFIX = rtecdc.bin
FIRMWARE_FILES       = $(foreach img, $(FIRMWARE_IMAGES), $(word 1,$(subst -,$(space),$(word 3,$(subst .,$(space),$(img)))))$(FIRMWARE_NAME_SUFFIX))

# Checkout these BOM files. If any conditionals are needed, they are
# defined with -defs flag
HNDSVN_BOM  := wl-build.sparse

UNDEFS= # CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ CONFIG_BCM93725 BCM4413_CHOPS BCM42X_CHOPS BCM33X_CHOPS CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 CONFIG_BCM4710 CONFIG_MIPS_BRCM POCKET_PC BCMINTERNAL DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 WLMOTOROLALJ

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS= # BCM47X BCM47X_CHOPS

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

SIGN_OS          = $(UPPER_VERSION_NAME)

SIGNTOOL_TIME  := "http://timestamp.verisign.com/scripts/timstamp.dll"
SIGNTOOL_OPTS  ?= /a /v /s my /n "Broadcom Corporation" /t $(SIGNTOOL_TIME)
SIGNTOOL_CMND_32  := $(RETRYCMD) $(WIN8X_WDK_ROOT)/bin/x86/signtool sign $(SIGNTOOL_OPTS)
SIGNTOOL_CMND_64  := $(RETRYCMD) $(WIN8X_WDK_ROOT)/bin/x64/signtool sign $(SIGNTOOL_OPTS)
export SIGNTOOL_OPTS

INSTALL_INF_SCRIPT := src/tools/release/install_inf_win.sh
CLEAN_INF_SCRIPT := src/tools/release/clean_inf_win.sh

INS_DIR        := components/apps/windows/install/app
WARN_FILE      := _WARNING_PARTIAL_BUILD_DO_NOT_USE

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $@/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $@/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

define CP_INF_FILE_WIN
	cp $(strip $1)/$(DRIVER_TEMPLATE_INF).inf $(strip $1)/$(strip $2).inf
endef # CP_INF_FILE_WIN

# Copy and timestamp inf files
define INSTALL_INF
	@echo "#- $0"
	$(INSTALL) $1 $2
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d $(RELDATE) "`cygpath -w $2`"
endef  # INSTALL_INF

# Signing several drivers
# Arg listing: $1=Driver-folder $2=CPU for signtool, $3=space-separated driver@cat list
define RELEASE_SIGN_WIN8X_DRIVERS
	@echo "#- $0($1,$2)"
	$(MAKE) -C src/dhd/wdm release_sign_driver \
		SIGN_DIR=$(strip $1) SIGN_ARCH=$(strip $2) \
		SIGN_DRIVER=$(DRIVER_NAME).sys SIGN_DRIVERCAT=$(DRIVER_NAME).cat \
		WINOS=$(SIGN_OS) WDK_VER=$(WIN8X_WDK_VERSION) VS_VER=$(VS_VER) \
		RELNUM=$(RELNUM) RELDATE=$(RELDATE)
endef # RELEASE_SIGN_WIN8X_DRIVERS

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
			COPY_SRC_DIR=$(BUILT_DIR)src/dhd/wdm COPY_DEST_DIR=$(strip $1); \
	done
	$(MAKE) -f $(word 1,$(MAKEFILE_LIST)) copy_firmware \
		COPY_SRC_DIR=$(BUILT_DIR)src/dhd/wdm COPY_DEST_DIR=$(strip $1)
	$(INSTALL) $(BUILT_DIR)src/dhd/wdm/empty.cat $(strip $1)$(strip $5).cat
	$(call INSTALL_BINARIES, $1, $(BUILT_DIR)src/dhd/wdm/$(strip $7)/,$(DRIVER_NAME).sys, $9)
	$(call INSTALL_INF, $(BUILT_DIR)src/dhd/wdm/$(strip $6)/$(DRIVER_INF).inf, $(strip $1)$(DRIVER_INF).inf)
endef #INSTALL_DRIVER_FILES

define INSTALL_INF_WIN
	@$(MARKSTART)
	$(INSTALL_INF_SCRIPT) $1 $2 $3 $4 $5 $6 $7
	@$(MARKEND)
endef #INSTALL_INF_WIN

define CLEAN_INF_WIN
	@$(MARKSTART)
	$(CLEAN_INF_SCRIPT) $1 $2 $3 $4
	@$(MARKEND)
endef #CLEAN_INF_WIN

all: build_end
build_end: package
package: build $(OEM_LIST_WIN8X_RLSPKG:%=$(RELEASEDIR)/$(VERSION_NAME)/%)

ifeq ($(BUILD_PARTIAL),true)
build: pre_build \
	build_install
else
build: pre_build \
	build_firmware \
	build_driver \
	build_install
endif # BUILD_PARTIAL
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
# (as src/dhd/wdm/win8x-dhdpcie-config.mk not yet checked out). So it is executed
# after checkout by recursively call this makefile with build_firmware2 target
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

build_driver: build_firmware
build_driver: pre_build
	@$(MARKSTART)
	$(MAKE) -C src/dhd/wdm VS_VER=$(VS_VER) WDK_VER=$(WIN8X_WDK_VERSION) BUILD_TYPES=free BUS=pcie BUILD_ARCHS="$(BUILD_ARCHS)" -f win8xdriver.mk all
	$(MAKE) -C src/wl/exe VS_VER=$(VS_VER) WDK_VER=$(WIN8X_WDK_VERSION) BUILD_CONFS=Releasewb -f wl_vs13.mk all
	$(MAKE) -C src/dhd/exe VS_VER=$(VS_VER) WDK_VER=$(WIN8X_WDK_VERSION) BUILD_CONFS=Release -f dhd_vs13.mk all
	@$(MARKEND)

build_install: export VS_PRE_BUILD_ENABLED=0
build_install: VERBOSE:=1
build_install: pre_build
	@$(MARKSTART)
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND)
	@$(MARKEND)

pre_build: checkout
	@$(MARKSTART)
	@mkdir -pv release
	@for oem in $(OEM_LIST_WIN8X_RLSPKG); do \
	  mkdir -pv release/$(VERSION_NAME)/$${oem}; \
	done
	mkdir -pv release/$(VERSION_NAME)/Internal
	@$(MARKEND)

## Win8X release packaging after everything is built
package:
	@$(MARKSTART)
	-$(INSTALL) src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

$(RELEASEDIR)/$(VERSION_NAME)/Bcm: \
	$(RELEASEDIR)/$(VERSION_NAME)/Internal \
	$(RELEASEDIR)/$(VERSION_NAME)/Bcm/Bcm_DriverOnly

$(RELEASEDIR)/$(VERSION_NAME)/Internal: build FORCE
	@$(MARKSTART)
	mkdir -pv $@
	$(call WARN_PARTIAL_BUILD)
	$(call CP_INF_FILE_WIN, src/dhd/wdm/inf, $(DRIVER_INF))
	$(call INSTALL_DRIVER_FILES, $@/x86/, DRIVER_FILES_NVRAM, Bcm, , $(DRIVER_NAME), inf, win8x_pcie_free/Win32, Releasewb, map pdb)
	$(call INSTALL_INF_WIN, $@/x86, $(DRIVER_INF), w8dhd, $(NDISVER_WIN8X), $(ARCH_WIN32), $(NDISARCH_32), $(DRIVER_NAME))
	$(call CLEAN_INF_WIN, $@/x86, $(DRIVER_INF), w7dhd, w10dhd)
	$(call INSTALL_DRIVER_FILES, $@/x64/, DRIVER_FILES_NVRAM, Bcm, 64, $(DRIVER_NAME), inf, win8x_pcie_free/x64, x64/Releasewb, map pdb)
	$(call INSTALL_INF_WIN, $@/x64, $(DRIVER_INF), w8dhd, $(NDISVER_WIN8X), $(ARCH_WIN64), $(NDISARCH_64), $(DRIVER_NAME))
	$(call CLEAN_INF_WIN, $@/x64, $(DRIVER_INF), w7dhd, w10dhd)
	$(call RELEASE_SIGN_WIN8X_DRIVERS, $@/x86, x86, )
	$(call RELEASE_SIGN_WIN8X_DRIVERS, $@/x64, X64, )
	$(call INSTALL_BINARIES, $@/x86/, src/wl/exe/Releasewb/, wl.exe, pdb)
	$(call INSTALL_BINARIES, $@/x86/, src/dhd/exe/Release/, dhd.exe, pdb)
	$(call INSTALL_BINARIES, $@/x64/, src/wl/exe/x64/Releasewb/, wl.exe, pdb)
	$(call INSTALL_BINARIES, $@/x64/, src/dhd/exe/x64/Release/, dhd.exe, pdb)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)

$(RELEASEDIR)/$(VERSION_NAME)/Bcm/Bcm_DriverOnly: OEM=Bcm
$(RELEASEDIR)/$(VERSION_NAME)/Bcm/Bcm_DriverOnly: build FORCE
	@$(MARKSTART)
	mkdir -pv $@
	$(call WARN_PARTIAL_BUILD)
	$(call INSTALL_DRIVER_FILES, $@/x86/, DRIVER_FILES_NVRAM, Bcm, , $(DRIVER_NAME), inf, win8x_pcie_free/Win32, Releasewb,)
	$(call INSTALL_INF_WIN, $@/x86, $(DRIVER_INF), w8dhd, $(NDISVER_WIN8X), $(ARCH_WIN32), $(NDISARCH_32), $(DRIVER_NAME))
	$(call CLEAN_INF_WIN, $@/x86, $(DRIVER_INF), w7dhd, w10dhd)
	$(call INSTALL_DRIVER_FILES, $@/x64/, DRIVER_FILES_NVRAM, Bcm, 64, $(DRIVER_NAME), inf, win8x_pcie_free/x64, x64/Releasewb,)
	$(call INSTALL_INF_WIN, $@/x64, $(DRIVER_INF), w8dhd, $(NDISVER_WIN8X), $(ARCH_WIN64), $(NDISARCH_64), $(DRIVER_NAME))
	$(call CLEAN_INF_WIN, $@/x64, $(DRIVER_INF), w7dhd, w10dhd)
	$(call RELEASE_SIGN_WIN8X_DRIVERS, $@/x86, x86, )
	$(call RELEASE_SIGN_WIN8X_DRIVERS, $@/x64, X64, )
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)

build_clean: package
	@$(MARKSTART)
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

.PHONY: build_driver build_firmware build_firmware2 copy_firmware copy_dongle_images checkout mogrify build pre_build package build_include build_status build_end $(FIRMWARE_IMAGES) build_install build_app_libraries FORCE


################################################################################
## Simple text substitution utilities to avoid excessive $(shell ) calls.
## Only usable for = and not := due to position at the end of this file
################################################################################

.tolower = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))
.toupper = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

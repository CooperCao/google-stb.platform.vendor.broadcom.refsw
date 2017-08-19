#
# Makefile to generate XP and Vista source tree based on wlconfigs flags and
# then build/prefast_verify/sign x86 and amd64 xp and vista USB DHD drivers.
#
# Notes:
#  - This is a standalone makefile to build xp and vista drivers using WDK toolset
#  - By default this makefile builds xp and vista drivers
#    To use WDK ver NNNN       "make WDK_VER=NNNN"
# Want more granular (for now, we can add these make targets)
#    To build xp x86 checked   "make WINOS=WINXP BUILD_TYPES=checked BUILD_ARCHS=x86 build_usb_xp_driver"
#    To build all variations of Win8x SDIO driver:
#       "make WINOS=WIN8X BUILD_TYPES="Debugwb Releasewb" VS_VER=2013 BUILD_ARCHS="x86 x64 arm" WDK_VER=9600 BUS=sdio -f win8xdriver.mk build_sdio_win8x_driver"
#    To build all variations of Win8x PCIE driver:
#       "make WINOS=WIN8X BUILD_TYPES="Debugwb Releasewb" VS_VER=2013 BUILD_ARCHS="x86 x64 arm" WDK_VER=9600 BUS=pcie -f win8xdriver.mk build_pcie_win8x_driver"
#    To build all variations of Win10 PCIE driver:
#       "make WINOS=WIN10 BUILD_TYPES="Debugwb Releasewb" VS_VER=2015 BUILD_ARCHS="x86 x64" WDK_VER=10240 BUS=pcie -f win8xdriver.mk build_pcie_win10_driver"
#    For more usage examples, please refer to HowToCompileThings twiki
#
# $Id: win8xdriver.mk 432931 2015-10-30 01:17:15Z $

THIS_MAKEFILE  := $(lastword $(MAKEFILE_LIST))

# Pick up standard definitions.
include $(dir $(THIS_MAKEFILE))../../makefiles/WLAN_Common.mk

SHELL          := bash.exe
WDK_VER        ?= 9600
VS_VER         ?= 2013
#WDK_OACR       := no_oacr
SRCBASE        := ../..
WLCONF_GEN     := true
WLCFGDIR       := $(SRCBASE)/wl/config
BUS            ?= pcie
WINOS          ?= WIN8X

WINOS_LOWER    := $(strip $(call wlan_tolower,$(WINOS)))

$(WINOS)BLDDIR := build$(BUS)$(WINOS_LOWER)

TITLE          := $(if $(TAG),$(TAG),NIGHTLY)$(if $(BRAND),_$(BRAND))
ACTION         := build
BATSUFFIX      := $(if $(WDK_REGEN_BUILDSPACE),_regen)

BLDDIR         := $($(WINOS)BLDDIR)
WLTUNEFILE     := wltunable_sample.h
# Single band dongle (Dual band image size exceeds limit)
EMBED_IMG_NAME ?= 43236b1-roml/sdio-ag-ndis-vista
#  CONFIG_WL_CONF ?= wlconfig_win_dhd_$(BUS)_$(WINOS_LOWER)
BCMDRVPFX_X86  := bcm$(BUS)dhd$(WINOS_LOWER)
BCMDRVPFX_X64  := bcm$(BUS)dhd$(WINOS_LOWER)64
BCMDRVPFX_ARM  := bcm$(BUS)dhd$(WINOS_LOWER)arm

ifdef VCTHREADS
  ifndef CL
    $(info Parallel compilation enabled for VS projects)
    export CL := /MP
  endif
  ifeq (,$(filter /m%,$(VSFLAGS)))
    VSFLAGS += /m
  endif
endif

# Temporary change to allow for using dhd embedded with single band dongle image
# Rename this to EMBED_IMG_NAME if you want dhd with single band dongle image
EMBED_IMG_NAME_SB := 43236b1-roml/sdio-ag-ndis-vista

BLDDIRDOS      := $(subst /,\,$(BLDDIR))
SOURCES        := $(BLDDIR)/sources
CWDIR          := $(shell cygpath -p -m "$(CURDIR)\$(BLDDIRDOS)")
## Following error condition very rarely fires
ifeq ($(findstring //,$(CWDIR)),//)
  $(error "ERROR: Can't use UNC style paths. Change to dir which shows non-UNC path for 'pwd' command")
endif

empty          :=
space          := $(empty) $(empty)
NULL           := /dev/null

## For now if BRAND is not specified it is assumed to be a developer build
ifeq ($(BRAND),)
  DEVBUILD     := 1
endif # BRAND

NATIVE_WDK_DIR := "Program Files (x86)/Windows Kits/8.0"
TOOLS_WDK_DIR  := $(WDK_VER)wdksdk

WDKDIR        := $(strip $(if $(BASEDIR),$(BASEDIR),\
                   $(subst /,\,$(firstword $(wildcard C:/$(NATIVE_WDK_DIR) \
		                                      C:/tools/msdev/$(TOOLS_WDK_DIR) \
						      D:/$(NATIVE_WDK_DIR) \
						      D:/tools/msdev/$(WDK_VER)wdk \
						      C:/tools/msdev/$(TOOLS_WDK_DIR)) \
					   $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(TOOLS_WDK_DIR)))))
WDKDIR_UX     := $(subst \,/,$(WDKDIR))
BASEDIR       := $(WDKDIR)
BASEDIR_UX    := $(WDKDIR_UX)
VSDIR         := $(subst /,\,$(firstword $(wildcard C:/tools/msdev/VS$(VS_VER) \
                                                    D:/tools/msdev/VS$(VS_VER) \
						    C:/tools/mdev/VS$(VS_VER))))
VSDIR_UX      := $(subst \,/,$(VSDIR))
CHKINF_DDK_VER := 3790ddk1830
CHKINF_DDK_DIR := $(firstword $(wildcard C:/tools/msdev/$(CHKINF_DDK_VER) \
                                         D:/tools/msdev/$(CHKINF_DDK_VER) \
					 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(CHKINF_DDK_VER)))
CHKINF_BAT      = $(CHKINF_DDK_DIR)/tools/chkinf/chkinf.bat
DOSCMD         := $(subst \,/,$(COMSPEC))
CHECKED_CFG    := checked
FREE_CFG       := free

export BUILD_ARCHS ?= x86 x64 arm
export BUILD_TYPES ?= $(CHECKED_CFG) $(FREE_CFG)

ifneq ($(filter WIN10,$(WINOS)),)
  CHECKED_TGT    := wdi_dhd_$(BUS)_checked
  FREE_TGT       := wdi_dhd_$(BUS)_free
  PROJFILE       := wdidhd63.vcxproj
else
  CHECKED_TGT    := $(WINOS_LOWER)_$(BUS)_checked
  FREE_TGT       := $(WINOS_LOWER)_$(BUS)_free
  PROJFILE       := bcmdhd63.vcxproj
endif

BUILD_VARIANTS ?= $(foreach arch,$(BUILD_ARCHS),$(foreach type,$(BUILD_TYPES),$(ACTION)_$(arch)_$(BUS)_$(type)))

BUILD_DRIVERS  := build_$(BUS)_$(WINOS_LOWER)_driver

MSBLD_TEMPLATE := .temp_msbuild_template_$(WINOS_LOWER)_$(BUS).bat
export PATH    := /usr/bin:/bin:$(PATH)
RETRYCMD       ?= $(firstword $(wildcard \
                     C:/tools/build/bcmretrycmd.exe \
                     $(WLAN_WINPFX)/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                  ))

# SoftICE debug option flags
NTICE_FLAGS    := -translate:always,source,package

# Vars used for signing vista driver after build
OEM            ?=bcm
RELNUM         :=$(shell date '+%Y.%-m.%-d.%-S')
RELDATE        :=$(shell date '+%m/%d/%Y')
SIGN_ARCH      ?=amd64

SIGN_TIMESTAMP_URL ?=http://timestamp.verisign.com/scripts/timstamp.dll

## Driver signing
ifneq ($(filter %64,$(SIGN_ARCH)),)
    SIGN_DRIVER    :=bcm$(BUS)dhd$(WINOS_LOWER).sys
    SIGN_DRIVERCAT :=bcm$(BUS)dhd$(WINOS_LOWER).cat
endif # SIGN_ARCH

export WLCONF_GEN
export WDK_VER

## Paths to search wl headers

## Prerelease contains microsoft wdk/ddk pre-release header files
INC_DIRS+= $(SRCBASE)/include/prerelease
PRERELEASE_FILES := $(SRCBASE)/include/prerelease/windot11.h
EXTRA_C_SOURCES += wlc_alloc.c

INC_DIRS  += $(SRCBASE)/include
INC_DIRS  += $(SRCBASE)/shared
INC_DIRS  += $(SRCBASE)/wl/sys
INC_DIRS  += $(SRCBASE)/../components/phy/old
INC_DIRS  += $(SRCBASE)/dongle
INC_DIRS  += $(SRCBASE)/dhd/sys
INC_DIRS  += $(SRCBASE)/../build/dongle/$(EMBED_IMG_NAME)
# REMOVE when BISON is no longer used
INC_DIRS  += $(SRCBASE)/dongle/rte/wl/builds/$(EMBED_IMG_NAME)

ifdef DEVBUILD
  BSRCBASE  := $(shell cygpath -m $(SRCBASE))
  BINC_DIRS := $(subst $(SRCBASE),$(BSRCBASE),$(SRCBASE)/include/prerelease)
  BINC_DIRS += $(BASEDIR_UX)/inc/api
  BINC_DIRS += $(subst $(SRCBASE),$(BSRCBASE),$(INC_DIRS))
  BINC_DIRS += src/include
  BINC_DIRS := $(subst //,/,$(BINC_DIRS))
endif #DEVBUILD

EXTRA_C_DEFINES   += -DNDIS -DNDIS_MINIPORT_DRIVER -DNDIS_WDM -DWDM
# NOTE1: BCMWL is a dummy flag, probably used to ensure that non-wl (or non-dhd) builds
#        donot include ndshared.h (or other ndis stuff) by mistake
EXTRA_C_DEFINES   += -DBCMWL -DBCMDRIVER

# All usb and sdio wdm dhd drivers need to embed an image
BCM_DNGL_EMBEDIMAGE=1

# BUS is recursively passed by this makefile for either sdio or usb driver types
ifeq ($(BUS),usb)
	ifneq ($(findstring 43236b1,$(EMBED_IMG_NAME)),)
		EXTRA_C_DEFINES += -DEMBED_IMAGE_43236b1
		WLTUNEFILE     := wltunable_rte_43236b1.h
	endif
	ifneq ($(findstring 43236b0,$(EMBED_IMG_NAME)),)
		EXTRA_C_DEFINES += -DEMBED_IMAGE_43236b0
		WLTUNEFILE     := wltunable_rte_43236b0.h
	endif
else
  EXTRA_C_DEFINES += -DEMBED_IMAGE_GENERIC
endif # BUS

#ifdef WLTEST
ifdef WLTEST
  EXTRA_C_DEFINES += -DWLTEST
endif
#endif
EXTRA_C_DEFINES   += -DNDIS_DMAWAR
EXTRA_C_DEFINES   += $(if $(BRAND),-DBRAND=$(BRAND))
EXTRA_C_DEFINES   += -DBINARY_COMPATIBLE=0
WL_CHK_C_DEFINES  := -DBCMDBG -DBCMINTERNAL -DBCMDBG_MEM -DDHD_DEBUG

EXTRA_C_DEFINES += -DSIMPLE_ISCAN -DDHD_WIN7 -DND_ALL_PASSIVE
EXTRA_C_DEFINES += -DBCMEMBEDIMAGE="\<..\/rtecdc.h\>"

# Strange but true: -DWIN7 (not -DWIN8)
EXTRA_C_DEFINES += -DNDIS630_MINIPORT=1 -DNDIS630 -DWIN7 -DBCMNDIS6

## Aggregate individual flags/defines
C_DEFINES  = $(WLFLAGS) $(EXTRA_C_DEFINES) $(MY_C_DEFINES)
ifeq ($(findstring $(CHECKED_CFG),$(BUILD_TYPES)),$(CHECKED_CFG))
  CPP_DEFINES= $(C_DEFINES) $(WL_CHK_C_DEFINES)
else
  CPP_DEFINES= $(C_DEFINES)
endif
C_SOURCES  = $(WLFILES) $(EXTRA_C_SOURCES)
C_SRC_PATHS= $(WLFILES_SRC) $(EXTRA_C_SRC_PATHS)

## Derive VPATH from wl.mk listing of source files
WDMVPATH   = $(sort $(foreach C,$(C_SRC_PATHS),$(subst src/,,$(SRCBASE)/$(dir $(C)))))

INC_FLAGS  = $(INC_DIRS:%=-I%)

# WDK build tool doesn't exit with error code, if driver is not built!
# So this function verified explicitly if the driver was built or not
OBJDIR_x86   := i386
OBJDIR_amd64 := amd64
OBJDIR_x64   := amd64
OBJDIR_arm   := arm
SYSFILE_x86  := $(BCMDRVPFX_X86)
SYSFILE_amd64:= $(BCMDRVPFX_X64)
SYSFILE_x64  := $(BCMDRVPFX_X64)
SYSFILE_arm  := $(BCMDRVPFX_ARM)
## Args $1=ARCH, $2=WDKOS, $3=TYPE
define POST_BUILD_VERIFY
	@echo "#- $0($1,$2,$3)"
	@objpath="obj$(3)_$(2)_$(if $(findstring 64,$1),amd64,$(if $(findstring arm,$1)arm,x86))"; \
	sys="$(BLDDIR)/$${objpath}/$(OBJDIR_$(1))/$(SYSFILE_$(1)).sys"; \
	if [ ! -f "$${sys}" ]; then \
	   echo -e "\nERROR: $${sys} BUILD FAILED\n"; \
	   exit 1; \
	elif [ -f "$(NTICE)/nmsym.exe" -a "$(2)" != "wlh" -a "$(3)" == "$(CHECKED_CF)" ]; then \
	   echo "Running SoftICE on $${sys}"; \
	   "$(NTICE)/nmsym.exe" $(NTICE_FLAGS) -SOURCE:.  $${sys}; \
	fi
endef # POST_BUILD_VERIFY

# WDK checkinf shows more warnings and errors from unlocalized string
# TODO: Use WDK chkinf instead of old ddk
# Use multiple string searches as different ddks produce different htm output
# Args $1=INF-PREFIX-NAME
define CHECK_INF
	@echo "#- $0($1)"
	@rm -rf build/chkinf/$1/htm
	@mkdir -p build/chkinf/$1/
	@echo "Copying $(@F) for check inf step"
	@cp -pv $@ build/chkinf/$1/
	@echo "Running chkinf on $(@F) (logs in chkinf/$1/htm/summary.htm"
	@cd build/chkinf/$1/ && $(CHKINF_BAT) $(@F)
	@echo -e "\n----------- $@ chkinf Errors ------"
	@cat build/chkinf/$1/htm/summary.htm | \
		egrep 'Error' | sed 's/^.*\(Error[^<]*\).*$$/\1/'
	@cat build/chkinf/$1/htm/@+__*.htm | egrep 'Line .* \(' | \
		egrep '\(E' | \
		sed -e 's/<.*">/ Error: /g' -e 's#</A>##g' | \
		sed -e 's/^[[:space:]]*//g'
	@echo -e "\n----------- $@ chkinf Warnings ------"
	@cat build/chkinf/$1/htm/summary.htm | egrep 'Warning' | \
		sed 's/^.*\(Warning[^<]*\).*$$/\1/'
	@cat build/chkinf/$1/htm/@+__*.htm | egrep 'Line .* \(' | \
		egrep '\(W|\(DW' | \
		sed -e 's/<.*">/ Warning: /g' -e 's#</A>##g' | \
		sed -e 's/^[[:space:]]*//g'
endef

# Default build target
.PHONY: all
all:     $(BUILD_DRIVERS)

.PHONY: prefast
prefast: $(PREFAST_DRIVERS)

ifdef PREFAST
all: prefast
endif # PREFAST

showinfo:
	@echo "==========================================="
	@echo "$(WINOS) WDMVPATH         = $(WDMVPATH)"
	@echo "$(WINOS) WLFLAGS          = $(WLFLAGS)"
	@echo "$(WINOS) EXTRA_C_DEFINES  = $(EXTRA_C_DEFINES)"
	@echo "$(WINOS) FINAL C_DEFINES  = $(sort $(C_DEFINES))"
ifeq ($(WINOS),WINXP)
	@echo "$(WINOS) MSX86_C_DEFINES  = $(MSX86_C_DEFINES)"
endif
	@echo "==========================================="
	@echo "$(WINOS) WLFILES          = $(WLFILES)"
	@echo "$(WINOS) EXTRA_C_SOURCES  = $(EXTRA_C_SOURCES)"
	@echo "$(WINOS) FINAL C_SOURCES  = $(sort $(WLFILES) $(EXTRA_C_SOURCES))"
	@echo "==========================================="
	@echo "$(WINOS) FINAL INC_DIRS   = $(INC_DIRS) $(OTHER_INC_DIRS)"
	@echo "==========================================="

$(BUILD_DRIVERS): BUS=$(if $(findstring _sdio_,$@),sdio,pcie)
$(BUILD_DRIVERS): CURBLDDIR=$(if $(findstring _$(WINOS_LOWER)_,$@),$($(WINOS)BLDDIR),$($(WINOS)BLDDIR))
$(BUILD_DRIVERS): CURWINOS=$(if $(findstring _$(WINOS_LOWER)_,$@),$(WINOS),WIN10)
$(BUILD_DRIVERS):
	@echo "====================================================="
	@echo "Making $@ inside $(CURBLDDIR) (platform=$(WINOS_LOWER))"
	@echo "Platform=$(WINOS_LOWER)"
	$(MAKE) -f $(THIS_MAKEFILE) BUS=$(BUS) WINOS=$(CURWINOS) \
		BLDDIR=$(CURBLDDIR) WDK_VER=$(WDK_VER) build_driver

# gen_sources: generates only msft ddk/wdk compatible 'sources' file
# build_sources: builds bcm wl drivers from msft ddk/wdk 'sources' file
.PHONY: build_driver
build_driver: build_sources

# Prepare the sources file with pre-requisites
prep_sources:
	@echo -e "#\n# Preparing $(SOURCES) file\n#\n"
#	For iterative developer build do not purge old BLDDIR
ifndef DEVBUILD
	@if [ -d "$(BLDDIR)" ]; then rm -rf $(BLDDIR); fi
endif #DEVBUILD
	@mkdir -p $(BLDDIR)
	@echo -e "#" > $(SOURCES)
	@echo -e "# Don't edit. This is automagically generated!!" >> $(SOURCES)
	@echo -e "# Use wdk's 'build' to build vista driver\n" >> $(SOURCES)
	@echo -e "# Generated on $$(date)" >> $(SOURCES)
	@echo -e "# $(if $(BRAND),BRAND = $(BRAND))\n" >> $(SOURCES)
#	For developer build and for better build performance
#	we skip copy_includes rule and refer to original headers
ifdef DEVBUILD
	@echo -e "INCLUDES        = $(subst $(space),; ,$(BINC_DIRS) $(OTHER_INC_DIRS)); \0044(SDK_INC_PATH); \0044(DDK_INC_PATH)"  >> $(SOURCES)
else #DEVBUILD
	@echo -e "INCLUDES        = $(subst $(space),; ,$(subst $(SRCBASE),src,$(INC_DIRS) $(OTHER_INC_DIRS))); \0044(SDK_INC_PATH); \0044(DDK_INC_PATH)"  >> $(SOURCES)
endif #DEVBUILD
	@echo -e "TARGETTYPE      = DRIVER"              >> $(SOURCES)
	@echo -e "TARGETPATH      = obj"                 >> $(SOURCES)
	@echo -e "TARGETLIBS      = \0044(DDK_LIB_PATH)/ndis.lib \\" >> $(SOURCES)
	@echo -e "                  \0044(DDK_LIB_PATH)/sdbus.lib \\" >> $(SOURCES)
	@echo -e "                  \0044(DDK_LIB_PATH)/ntstrsafe.lib\n" >> $(SOURCES)
ifeq ($(WINOS),WINXP)
	@echo -e "!IF \"\0044(_BUILDARCH)\" == \"x86\""  >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_X86)"    >> $(SOURCES)
	@echo -e "MS_C_DEFINES    = $(MSX86_C_DEFINES)"  >> $(SOURCES)
	@echo -e "!ELSE"                                 >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_X64)"    >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
	@echo -e "WL_C_DEFINES    = $(WLFLAGS)\n" >> $(SOURCES)
	@echo -e "EXTRA_C_DEFINES = $(EXTRA_C_DEFINES)\n" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(WL_C_DEFINES) \0044(EXTRA_C_DEFINES) \0044(MS_C_DEFINES)\n" >> $(SOURCES)
	@echo -e "!IF \"\0044(DDKBUILDENV)\" == \"$(CHECKED_CF)\"" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(C_DEFINES) $(WL_CHK_C_DEFINES)" >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
else # VISTA
	@echo -e "!IF \"\0044(_BUILDARCH)\" == \"x86\""  >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_X86)"    >> $(SOURCES)
	@echo -e "!ELSE"                                 >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_X64)"    >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
	@echo -e "WL_C_DEFINES    = $(WLFLAGS)\n" >> $(SOURCES)
	@echo -e "EXTRA_C_DEFINES = $(EXTRA_C_DEFINES)\n" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(WL_C_DEFINES) \0044(EXTRA_C_DEFINES)\n" >> $(SOURCES)
	@echo -e "!IF \"\0044(DDKBUILDENV)\" == \"$(CHECKED_CF)\"" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(C_DEFINES) $(WL_CHK_C_DEFINES)" >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
endif # WINOS
	@echo -e "LINKER_FLAGS    = \0044(LINKER_FLAGS) -MAP:\0044(TARGETPATH)/\0044(TARGET_DIRECTORY)/\0044(TARGETNAME).map\n" >> $(SOURCES)
ifeq ($(WINOS),WINVISTA)
	@echo -e "KMDF_VERSION_MAJOR    = 1\n" >> $(SOURCES)
endif # WINVISTA

# List the wl config filtered source files into $(BLDDIR)/sources file
# For developer build, only updated files are copied over and built
copy_sources: prep_sources
	@echo "Copying sources from $(WINOS) VPATH=$(WDMVPATH)"
	@echo -e "#\n# Copying wl source files\n#\n"
	@echo "SOURCES         = \\" >> $(SOURCES)
	@for wlsrc in $(C_SOURCES); do \
	    found_wlsrc=0; \
	    for vdir in $(WDMVPATH); do \
		if [ -f "$$vdir/$$wlsrc" ]; then \
		   found_wlsrc=1; \
		   if [ ! -f $(BLDDIR)/$$wlsrc -o $(BLDDIR)/$$wlsrc -ot $$vdir/$$wlsrc ]; then \
	              install -p -v $$vdir/$$wlsrc $(BLDDIR)/$$wlsrc; \
		   fi; \
		   echo "    $$wlsrc \\" >> $(SOURCES); \
		fi; \
	    done; \
	    if [ $${found_wlsrc} == 0 ]; then \
	       echo "Couldn't find $$wlsrc in $(WDMVPATH) paths!!"; \
	       exit 1; \
	    fi; \
	done
	@echo -e "\n# End of $(WINOS) Sources file\n" >> $(SOURCES)

# Scan the source files to generate list of wl header files
# needed to compile sources generated in 'sources' make rule.
copy_includes: copy_sources
ifndef DEVBUILD
	@echo "WDK ntddndis.h replaces src/wl/sys/ntddndis.h"
	$(strip python $(SRCBASE)/tools/build/wdkinc.py \
	    --tree-base=$(SRCBASE)/.. --to-dir=$(BLDDIR) --skip wl/sys/ntddndis.h \
	    -- $(INC_FLAGS) $(C_DEFINES) $(C_SOURCES) \
	    $(if $(filter 6001% 7% 8%,$(WDK_VER)),,$(PRERELEASE_FILES)))
endif # DEVBUILD

## --------------------------------------------------------------------------
## By default build both x86 and amd64 free/checked drivers
## To build individual driver types, use e.g: make build_x86_checked
.PHONY: build_sources
build_sources: prep_sources $(MSBLD_TEMPLATE) $(BUILD_VARIANTS)

.PHONY: $(BUILD_VARIANTS)
$(BUILD_VARIANTS): WDKOS=$(WINOS_LOWER)
$(BUILD_VARIANTS): TYPE=$(if $(findstring $(FREE_CFG),$@),$(FREE_TGT),$(CHECKED_TGT))
$(BUILD_VARIANTS): ARCH=$(word 2,$(subst _,$(space),$@))
$(BUILD_VARIANTS): MSG:=$(strip $(call wlan_toupper,$(ACTION)))
$(BUILD_VARIANTS): BAT=$(ACTION)_$(TYPE)_$(BUS)_$(WDKOS)_$(ARCH)$(BATSUFFIX).bat
$(BUILD_VARIANTS): BLD_TEMPLATE:=$(MSBLD_TEMPLATE)
$(BUILD_VARIANTS): .SHELLFLAGS:=-o pipefail $(.SHELLFLAGS)
$(BUILD_VARIANTS):
	@echo "Building variant '$@' from ($(BUILD_VARIANTS))"
	@install -pvD $(SRCBASE)/wl/config/$(WLTUNEFILE) $(TYPE)/wlconf.h
	@echo "INFO: Generating new $(BAT) from $(BLD_TEMPLATE)"
	sed \
	   -e "s/%VCPROJFILE%/$(PROJFILE)/g"   \
	   -e "s/%TYPE%/$(TYPE)/g"   \
	   -e "s/%WDKOS%/$(WDKOS)/g" \
	   -e "s/%ARCH%/$(ARCH)/g"   \
	   -e "s/%ACTION%/$(ACTION)/g"   \
	   -e "s/%WINOS%/$(WINOS)/g"   \
	   -e "s/%MSG%/$(MSG)/g"   \
	   -e "s/%WDK_VER%/$(WDK_VER)/g"   \
	   -e "s/%VSDIR%/$(VSDIR)/g"   \
	   -e "s!%WDKDIR%!$(subst \,~,$(WDKDIR))!g"   \
	   -e "s!%BASEDIR%!$(subst /,~,$(BASEDIR_UX))!g"   \
	   -e "s!%BLDDIR%!$(subst /,~,$(CWDIR))!g"   \
	   -e "s/%TITLE%/$(TITLE)_$(MSG)/g"   \
	$(BLD_TEMPLATE) | sed -e "s/~/\\\\/g" > $(BAT) && \
	$(DOSCMD) /c "$(BAT)" && \
	$(RM) $(BAT)

# This technique protects script lines from shell escaping. Looks weird but is more robust.
# The 'if' tests below need &&'s against the prev variable values or spaces will be appended to the value
export MSLINE_1  := @echo off
export MSLINE_2  := @REM Automatically generated on $(shell date). Do not edit
export MSLINE_3  := if "%ARCH%"=="x86" set BLDOUTARCH=Win32&& set VCVARSARCH=x86&&        set INTROOT=.
export MSLINE_4  := if "%ARCH%"=="x64" set BLDOUTARCH=x64&&   set VCVARSARCH=x64&&        set INTROOT=x64
export MSLINE_5  := if "%ARCH%"=="arm" set BLDOUTARCH=arm&&   set VCVARSARCH=x86_arm&&    set INTROOT=arm
export MSLINE_6  := set INTDIR=%INTROOT%\%TYPE%
export MSLINE_7  := set VARIANT=%TYPE%_%WDKOS%_%BLDOUTARCH%
export MSLINE_8  := set VCDIR="$(VSDIR_UX)/VC"
export MSLINE_9  := set TEMP=C:\temp
export MSLINE_10 := set TMP=C:\temp
export MSLINE_11 := if "%ALLUSERSPROFILE%"=="" SET ALLUSERSPROFILE=C:\ProgramData
export MSLINE_12 := if "%CommonProgramFiles%"=="" SET CommonProgramFiles=C:\Program Files\Common Files
export MSLINE_13 := if "%CommonProgramFiles(x86)%"=="" SET CommonProgramFiles(x86)=C:\Program Files (x86)\Common Files
export MSLINE_14 := if "%CommonProgramW6432%"=="" SET CommonProgramW6432=C:\Program Files\Common Files
export MSLINE_15 := if "%ComSpec%"=="" SET ComSpec=C:\Windows\system32\cmd.exe
export MSLINE_16 := if "%DLGTEST_NOTIFY%"=="" SET DLGTEST_NOTIFY=false
export MSLINE_17 := if "%ProgramData%"=="" SET ProgramData=C:\ProgramData
export MSLINE_18 := if "%ProgramFiles%"=="" SET ProgramFiles=C:\Program Files
export MSLINE_19 := if "%ProgramFiles(x86)%"=="" SET ProgramFiles(x86)=C:\Program Files (x86)
export MSLINE_20 := if "%ProgramW6432%"=="" SET ProgramW6432=C:\Program Files
export MSLINE_21 := if "%WINDIR%"=="" SET WINDIR=C:\Windows
export MSLINE_22 := if NOT EXIST %VSDIR%\VC\vcvarsall.bat goto vcenverror
export MSLINE_23 := echo %WINOS% %MSG% %VARIANT% with VS%VS_VER%
export MSLINE_24 := echo Running at %date% %time% on %computername%
export MSLINE_25 := set MAKEFLAGS=
export MSLINE_26 := call %VCDIR%\vcvarsall.bat %VCVARSARCH%
export MSLINE_27 := goto ec%ERRORLEVEL%
export MSLINE_28 := :ec0
export MSLINE_29 := title %VARIANT%
export MSLINE_30 := echo Current Dir: %cd% VSDIR %VSDIR% Path: %PATH%
export MSLINE_31 := which msbuild.exe
export MSLINE_32 := echo msbuild.exe /property:Configuration="%TYPE%" /property:Platform=%BLDOUTARCH% %VCPROJFILE% /property:WDKContentRoot=%WDKDIR%\ /fileLoggerParameters:LogFile=%ACTION%_%VARIANT%.log %VSFLAGS%
export MSLINE_33 := msbuild.exe /property:Configuration="%TYPE%" /property:Platform=%BLDOUTARCH% %VCPROJFILE% /property:WDKContentRoot=%WDKDIR%\ /fileLoggerParameters:LogFile=%ACTION%_%VARIANT%.log %VSFLAGS%
export MSLINE_34 := set buildec=%ERRORLEVEL%
export MSLINE_35 := if /I NOT "%buildec%"=="0" goto buildec1
export MSLINE_36 := title DONE_%VARIANT%
export MSLINE_37 := goto done
export MSLINE_38 := :vcenverror
export MSLINE_39 := echo ERROR: VS directory '%VCDIR%' is not found
export MSLINE_40 := exit /B 1
export MSLINE_41 := :buildec1
export MSLINE_42 := echo ERROR: %ACTION% failed with error code 1
export MSLINE_43 := exit /B 1
export MSLINE_44 := :buildec0
export MSLINE_45 := :done
export MSLINE_46 := echo Done with %WINOS% %MSG% for %VARIANT%
MSLINE_COUNT = 46

## Echo each raw env var in order creating .bat script
MSBLD_CMD := FOR /L %i IN (1,1,$(MSLINE_COUNT)) DO @cmd /v /c echo !MSLINE_%i!

$(MSBLD_TEMPLATE):
	@rm -fv $@
	@cmd /c '$(MSBLD_CMD)' >> $@

# Force .temp*.bat to refresh every time makefile is invoked
.PHONY: $(MSBLD_TEMPLATE)

## ---------------------------------------------------------------------------
# Signing works for WIN8 and, reportedly, for Vista
# If more than one driver shall be signed, drivers and their cats shall be specified in
# SIGN_DRIVERCAT as space separated list. Each list element contains
# @-separated driver@cat name pair

release_sign_driver: SIGN_ARCH := $(subst amd64,X64,$(SIGN_ARCH))
release_sign_driver: PATH := /usr/bin:/bin:$(shell cygpath -u $(subst \,/,$(WDKDIR))/bin/x86):$(PATH)

MS_CROSSCERT     ?=$(WLAN_WINPFX)/projects/hnd/tools/win/verisign/driver/"DigiCert High Assurance EV Root CA.crt"

ifneq ($(findstring WIN8X,$(WINOS)),)
    SIGN_OSARG=6_3_$(SIGN_ARCH)
else
    SIGN_OSARG=$(subst WIN,,$(WINOS))_$(SIGN_ARCH)
endif

.PHONY: release_sign_driver
release_sign_driver:
ifeq ($(words $(SIGN_DRIVERCAT)),1)
	@if [ ! -f "$(SIGN_DIR)/$(SIGN_DRIVER)" ]; then \
	    echo "ERROR:"; \
	    echo "ERROR: $(SIGN_DIR)/$(SIGN_DRIVER) not found to sign"; \
	    echo "ERROR: Does the $(SIGN_DRIVER) exist?"; \
	    echo "ERROR: Verify WINOS or SIGN_ARCH values provided"; \
	    echo "ERROR:"; \
	    exit 1; \
	fi
	@echo -e "\nSigning $(SIGN_ARCH) $(WINOS) driver $(SIGN_DIR)/$(SIGN_DRIVER)\n"
	@echo -e "\nWDKDIR=$(WDKDIR)\n"
	@echo -e "\nBASEDIR=$(BASEDIR)\n"
	@echo -e "\nPATH=$(PATH)\n"
	@$(SRCBASE)/tools/release/wustamp -o -r -v "$(RELNUM)" -d "$(RELDATE)" "$$(cygpath -w $(SIGN_DIR))"
	cd $(SIGN_DIR) && $(RETRYCMD) signtool sign /v /s my /n "Broadcom Corporation" /t "$(SIGN_TIMESTAMP_URL)" /ac $(MS_CROSSCERT) $(SIGN_DRIVER)
	cd $(SIGN_DIR) && Inf2cat /driver:. /os:$(SIGN_OSARG)
	cd $(SIGN_DIR) && $(RETRYCMD) signtool sign /v /s my /n "Broadcom Corporation" /t "$(SIGN_TIMESTAMP_URL)" /ac $(MS_CROSSCERT) $(SIGN_DRIVERCAT)
else
	@echo -e "\nSigning $(SIGN_ARCH) $(WINOS) at $(SIGN_DIR)\n"
	@for drivercat in $(SIGN_DRIVERCAT); do \
	    declare -a driver_and_cat; \
	    driver_and_cat=($${drivercat//@/ }); \
	    echo "Driver $${driver_and_cat[0]}, cat $${driver_and_cat[1]}"; \
	    if [ ! -f "$(SIGN_DIR)/$${driver_and_cat[0]}" ]; then \
		echo "ERROR:"; \
		echo "ERROR: $(SIGN_DIR)/$${driver_and_cat[0]} not found to sign"; \
		echo "ERROR: Does the $${driver_and_cat[0]} exist?"; \
		echo "ERROR: Verify WINOS or SIGN_ARCH values provided"; \
		echo "ERROR:"; \
		exit 1; \
	    fi; \
	done
	$(SRCBASE)/tools/release/wustamp -o -r -v "$(RELNUM)" -d "$(RELDATE)" "$$(cygpath -w $(SIGN_DIR))"
	cd $(SIGN_DIR) && Inf2cat /driver:. /os:$(SIGN_OSARG)
	cd $(SIGN_DIR) && \
	    for drivercat in $(SIGN_DRIVERCAT); do \
		declare -a driver_and_cat; \
		driver_and_cat=($${drivercat//@/ }); \
		$(RETRYCMD) signtool sign /v /s my /n "Broadcom Corporation" /t "$(SIGN_TIMESTAMP_URL)" /ac $(MS_CROSSCERT) $${driver_and_cat[1]}; \
		if (( $$? )); then exit 1; fi; \
		$(RETRYCMD) signtool sign /v /s my /n "Broadcom Corporation" /t "$(SIGN_TIMESTAMP_URL)" /ac $(MS_CROSSCERT) $${driver_and_cat[0]}; \
		if (( $$? )); then exit 1; fi; \
	    done; \
	exit 0
endif

# Target to generate xp and vista infs from a common template inf
# and run checkinf on that generated inf file.
generate_infs: build/bcm$(BUS)dhdxp.inf build/bcm$(BUS)dhdvista.inf

# Generate xp and vista infs from a common template
GENERATE_INF = \
	sed -e "s/;$${currentos};[[:blank:]]\+/\t/gi" \
	    -e "/;$${filterout};[[:blank:]]\+/d"

build/bcm$(BUS)dhdxp.inf build/bcm$(BUS)dhdvista.inf: bcm$(BUS)dhd.inf FORCE
	@echo -e "\nGenerating $@ from $<"
	@mkdir -pv $(@D)
	@currentos=$(if $(findstring xp.inf,$@),xp,vista); \
	    filterout=$(if $(findstring xp.inf,$@),vista,xp); \
	    < $< $(GENERATE_INF) > $@
	$(call CHECK_INF,$(basename $(@F)))

.PHONY: clean
clean:
	$(RM) -r $(wildcard $(foreach dir,$(wildcard *_checked *_free),$(foreach arch,$(BUILD_ARCHS),$(dir)/$(arch))))

.PHONY: clean_all
clean_all: clean
	$(RM) $(wildcard build_ch*.bat build_fre*.bat build_Debug*.bat build_Release*.bat $(BLD_TEMPLATE) $(MSBLD_TEMPLATE) build_*.log)
	$(RM) -r $(strip $(wildcard $(WIN7BLDDIR) $(WIN8XBLDDIR) *_checked *_free))

.PHONY: FORCE
FORCE:

PHONY: bcm$(BUS)dhdxp.inf bcm$(BUS)dhdvista.inf
PHONY: build_$(BUS)_win7_driver build_$(BUS)_win8x_driver build_$(BUS)_win10_driver

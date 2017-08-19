#
# Makefile to generate XP and Vista source tree based on wlconfigs flags and
# then build/prefast_verify/sign x86 and amd64 xp and vista USB DHD drivers.
#
# Notes:
#  - This is a standalone makefile to build xp and vista drivers using WDK toolset
#  - By default this makefile builds xp and vista drivers
#    To build xp only use      "make build_[usb/sdstd]_xp_driver"
#    To build vista only use   "make build_usb_vista_driver"
#    To prefast xp only use    "make prefast_[usb/sdstd]_xp_driver"
#    To prefast vista only use "make prefast_usb_vista_driver"
#    To use WDK ver NNNN       "make WDK_VER=NNNN"
# Want more granular (for now, we can add these make targets)
#    To build xp x86 checked   "make WINOS=WINXP BUILD_TYPES=checked BUILD_ARCHS=x86 build_usb_xp_driver"
#    For more usage examples, please refer to HowToCompileThings twiki
#
# $Id$

WLAN_ComponentsInUse := bcmwifi clm-api ppr ndis olpc keymgmt iocv dump dongle xp
include ../../makefiles/WLAN_Common.mk
SRCBASE        := $(WLAN_SrcBaseR)

SHELL          := bash.exe
WDK_VER        ?= 6000
WLCONF_GEN     := true
WLCFGDIR       := $(SRCBASE)/wl/config
BUS            ?= usb
WINOS          ?= WINVISTA
XPPFDIR        ?= prefast$(BUS)xp
XPBLDDIR       ?= build$(BUS)xp
VISTAPFDIR     ?= prefast$(BUS)vista
VISTABLDDIR    ?= build$(BUS)vista
TITLE          := $(if $(TAG),$(TAG),NIGHTLY)$(if $(BRAND),_$(BRAND))
ACTION         := $(if $(PREFAST),prefast,build)
BATSUFFIX      := $(if $(WDK_REGEN_BUILDSPACE),_regen)

ifeq ($(WINOS),WINXP)
  BLDDIR         ?= $(XPBLDDIR)
  WLTUNEFILE     :=
  EMBED_IMG_NAME :=
  CONFIG_WL_CONF ?= wlconfig_win_dhd_$(BUS)_xp
  BCMDRVPFX_X86  ?= bcm$(BUS)dhdxp
  BCMDRVPFX_X64  ?= bcm$(BUS)dhdxp64
endif #WINOS

ifeq ($(WINOS),WINVISTA)
  BLDDIR         ?= $(VISTABLDDIR)
  WLTUNEFILE     :=
  EMBED_IMG_NAME :=
  CONFIG_WL_CONF ?= wlconfig_win_dhd_$(BUS)_vista
  BCMDRVPFX_X86  ?= bcm$(BUS)dhdlh
  BCMDRVPFX_X64  ?= bcm$(BUS)dhdlh64
endif #WINOS

# Temporary change to allow for using dhd embedded with single band dongle image
# Rename this to EMBED_IMG_NAME if you want dhd with single band dongle image
EMBED_IMG_NAME_SB :=

# All usb and sdio wdm dhd drivers need to embed an image
BCM_DNGL_EMBEDIMAGE=1

ifeq ($(EMBED_IMG_NAME),)
$(error Missing embedded image name)
endif

EMBED_IMG_DIR = $(SRCBASE)/../build/dongle/$(EMBED_IMG_NAME)
# REMOVE AFTER ALL BRANCHES HAVE BEEN MOVED TO THE NEW DONGLE DIRECTORY STRUCTURE
EMBED_IMG_DIR_OLD = $(SRCBASE)/dongle/rte/wl/builds/$(EMBED_IMG_NAME)

BLDDIRDOS      := $(subst /,\,$(BLDDIR))
SOURCES        := $(BLDDIR)/sources
PWD_DOS        := $(shell cygpath -p -w "$(shell pwd)\$(BLDDIRDOS)")
CWDIR          := $(shell cygpath -p -m "$(shell pwd)\$(BLDDIRDOS)")
## Following error condition very rarely fires
ifeq ($(findstring //,$(CWDIR)),//)
  $(error "ERROR: Can't use UNC style paths: $(CWDIR)")
endif

empty          :=
space          := $(empty) $(empty)
NULL           := /dev/null

## For now if BRAND is not specified it is assumed to be a developer build
ifeq ($(BRAND),)
  DEVBUILD     := 1
endif # BRAND

RETRYCMD       ?= $(firstword \
                  $(wildcard \
		     C:/tools/build/bcmretrycmd.exe \
                     Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                  ))
WDKDIR         := $(if $(BASEDIR),$(BASEDIR),$(subst /,\,$(firstword $(wildcard C:/WinDDK/$(WDK_VER) C:/tools/msdev/$(WDK_VER)wdk C:/tools/mdev/wdk$(WDK_VER) D:/WinDDK/$(WDK_VER) D:/tools/msdev/$(WDK_VER)wdk C:/tools/mdev/wdk$(WDK_VER) Z:/projects/hnd/tools/win/msdev/$(WDK_VER)wdk))))
WDKDIR_UX      := $(subst \,/,$(WDKDIR))
CHKINF_DDK_VER := 3790ddk1830
CHKINF_DDK_DIR := $(firstword $(wildcard C:/tools/msdev/$(CHKINF_DDK_VER) D:/tools/msdev/$(CHKINF_DDK_VER) Z:/projects/hnd/tools/win/msdev/$(CHKINF_DDK_VER)))
CHKINF_BAT      = $(CHKINF_DDK_DIR)/tools/chkinf/chkinf.bat
DOSCMD         := $(subst \,/,$(COMSPEC))
BASEDIR         = $(shell if [ -d "$(WDKDIR)" ]; then echo "$(WDKDIR)"; else echo Z:/projects/hnd/tools/win/msdev/$(WDK_VER)wdk; fi)
BASEDIR_UX     := $(subst \,/,$(BASEDIR))
ifeq ($(findstring 6001,$(WDK_VER)),)
BUILD_ARCHS    ?= x86 amd64
else
BUILD_ARCHS    ?= x86 x64
endif
BUILD_TYPES    ?= checked free
BUILD_VARIANTS ?= $(foreach arch,$(BUILD_ARCHS),$(foreach type,$(BUILD_TYPES),$(ACTION)_$(arch)_$(type)))
# For usb build both xp+vista for 32+64bit. For sdio, build only xp 32bit
BUILD_DRIVERS  := $(foreach os,xp vista,build_usb_$(os)_driver)
BUILD_DRIVERS  += $(foreach os,xp vista,build_sdstd_$(os)_driver)
PREFAST_DRIVERS:= $(subst build_,prefast_,$(BUILD_DRIVERS))
BLD_TEMPLATE   := .temp_wdk_build_template.bat
export PATH    := /usr/bin:/bin:$(PATH)

export BUILD_TYPES BUILD_ARCHS

# SoftICE debug option flags
NTICE_FLAGS    := -translate:always,source,package

# Vars used for signing vista driver after build
OEM            ?=bcm
RELNUM         ?=$(shell date '+%Y.%-m.%-d.%-S')
RELDATE        ?=$(shell date '+%m/%d/%Y')
SIGN_ARCH      ?=amd64

# This can be overriden from release brand makefile or on cmd line
# when primary signing server is not reachable
SIGN_TIMESTAMP_URL ?=http://timestamp.verisign.com/scripts/timstamp.dll
#SIGN_TIMESTAMP_URL?=https://timestamp.geotrust.com/tsa

## Driver signing
ifneq ($(filter %64,$(SIGN_ARCH)),)
  ifeq ($(WINOS),WINXP)
    SIGN_DRIVER    :=bcm$(BUS)dhdxp64.sys
    SIGN_DRIVERCAT :=bcm$(BUS)dhdxp64.cat
  else  # WINOS
    SIGN_DRIVER    :=bcm$(BUS)dhdlh64.sys
    SIGN_DRIVERCAT :=bcm$(BUS)dhdlh64.cat
  endif # WINOS
else # SIGN_ARCH
  ifeq ($(WINOS),WINXP)
    SIGN_DRIVER    :=bcm$(BUS)dhdxp.sys
    SIGN_DRIVERCAT :=bcm$(BUS)dhdxp.cat
  else  # WINOS
    SIGN_DRIVER    :=bcm$(BUS)dhdlh.sys
    SIGN_DRIVERCAT :=bcm$(BUS)dhdlh.cat
  endif # WINOS
endif # SIGN_ARCH

export WLCONF_GEN
export WDK_VER

# Suppress OACR step post wdk build for newer WDK tools
ifeq ($(filter 6%,$(WDK_VER)),)
  WDK_OACR := no_oacr
else
  WDK_OACR :=
endif

include $(WLCFGDIR)/$(CONFIG_WL_CONF)
include $(WLCFGDIR)/wl.mk

## Paths to search wl headers

## Prerelease contains microsoft wdk/ddk pre-release header files
INC_DIRS+= $(SRCBASE)/include/prerelease
ifeq ($(WINOS),WINVISTA)
  PRERELEASE_FILES := $(SRCBASE)/include/prerelease/windot11.h
  EXTRA_C_SOURCES += wlc_alloc.c
endif # WINVISTA

INC_DIRS  += $(WLAN_ComponentIncDirsR)
INC_DIRS  += $(WLAN_IncDirsR)
INC_DIRS  += $(SRCBASE)/shared/zlib
INC_DIRS  += $(SRCBASE)/dhd/sys
INC_DIRS  += $(COMPONENTS_SRCBASE)/drivers/wl/shim/include

# REMOVE FOLLOWING ONCE DHD and WLC ARE FULLY SEPARATED
INC_DIRS  += $(WLAN_TreeBaseR)/components/phy/cmn/hal
# REMOVE ABOVE ONCE DHD and WLC ARE FULLY SEPARATED
INC_DIRS  += $(EMBED_IMG_DIR) $(EMBED_IMG_DIR_OLD)
# REMOVE BELOW ONCE BISON is no longer auto merged from trunk
INC_DIRS  += $(SRCBASE)/dongle


ifdef DEVBUILD
  BSRCBASE  := $(shell cygpath -m $(SRCBASE))
  BINC_DIRS := $(subst $(SRCBASE),$(BSRCBASE),$(SRCBASE)/include/prerelease)
  BINC_DIRS += $(BASEDIR_UX)/inc/api
  BINC_DIRS += $(subst $(SRCBASE),$(BSRCBASE),$(INC_DIRS))
  BINC_DIRS += src/include
  BINC_DIRS := $(subst //,/,$(BINC_DIRS))
endif #DEVBUILD

EXTRA_C_DEFINES   += -DNDIS -DNDIS_MINIPORT_DRIVER -DNDIS_WDM -DWDM
EXTRA_C_DEFINES   += -DBCMDRIVER

# BUS is recursively passed by this makefile for either sdio or usb driver types
ifeq ($(BUS),usb)
	ifneq ($(findstring 43236b1,$(EMBED_IMG_NAME)),)
		EXTRA_C_DEFINES += -DEMBED_IMAGE_43236b1
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
WL_CHK_C_DEFINES  := -DBCMDBG -DBCMINTERNAL -DBCMDBG_MEM

ifeq ($(WINOS),WINVISTA)
  EXTRA_C_DEFINES += -DNDIS60_MINIPORT -DNDIS60
else
  EXTRA_C_DEFINES += -DNDIS51_MINIPORT -DNDIS51
  # Override wdk default winver value to make single driver for xp and 2k
  MSX86_C_DEFINES := -UWINVER -DWINVER=0x0500
endif

## Aggregate individual flags/defines
C_DEFINES  = $(WLFLAGS) $(EXTRA_C_DEFINES) $(MY_C_DEFINES)
ifeq ($(findstring checked,$(BUILD_TYPES)),checked)
  CPP_DEFINES= $(C_DEFINES) $(WL_CHK_C_DEFINES)
else
  CPP_DEFINES= $(C_DEFINES)
endif
C_SOURCES  = $(WLFILES) $(EXTRA_C_SOURCES)
C_SRC_PATHS= $(WLFILES_SRC) $(EXTRA_C_SRC_PATHS)

## Derive VPATH from wl.mk listing of source files
WDMVPATH   = $(sort $(foreach C,$(C_SRC_PATHS),$(subst src/,,$(SRCBASE)/$(dir $(C))))) \
             $(WLAN_StdSrcDirsR) $(WLAN_ComponentSrcDirsR)
WDMVPATH += $(COMPONENTS_SRCBASE)/drivers/wl/shim/src

INC_FLAGS  = $(INC_DIRS:%=-I%)

# WDK build tool doesn't exit with error code, if driver is not built!
# So this function verified explicitly if the driver was built or not
OBJDIR_x86   := i386
OBJDIR_amd64 := amd64
OBJDIR_x64   := amd64
SYSFILE_x86  := $(BCMDRVPFX_X86)
SYSFILE_amd64:= $(BCMDRVPFX_X64)
SYSFILE_x64  := $(BCMDRVPFX_X64)
## Args $1=ARCH, $2=WDKOS, $3=TYPE
define POST_BUILD_VERIFY
	@echo "#- $0($1,$2,$3)"
	@objpath="obj$(3)_$(2)_$(if $(findstring 64,$1),amd64,x86)"; \
	sys="$(BLDDIR)/$${objpath}/$(OBJDIR_$(1))/$(SYSFILE_$(1)).sys"; \
	if [ ! -f "$${sys}" ]; then \
	   echo -e "\nERROR: $${sys} BUILD FAILED\n"; \
	   exit 1; \
	elif [ -f "$(NTICE)/nmsym.exe" -a "$(2)" != "wlh" -a "$(3)" == "chk" ]; then \
	   echo "Running SoftICE on $${sys}"; \
	   "$(NTICE)/nmsym.exe" $(NTICE_FLAGS) -SOURCE:.  $${sys}; \
	fi
endef # POST_BUILD_VERIFY

# Generate xp and vista infs from a common template
GENERATE_INF = \
	sed -e "s/;$${currentos};[[:blank:]]\+/\t/gi" \
	    -e "/;$${filterout};[[:blank:]]\+/d"

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
all:     $(BUILD_DRIVERS)
prefast: $(PREFAST_DRIVERS)

ifneq ($(PREFAST),)
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

$(BUILD_DRIVERS): BUS=$(if $(findstring _usb_,$@),usb,sdstd)
$(BUILD_DRIVERS): CURBLDDIR=$(if $(findstring _xp_,$@),$(XPBLDDIR),$(VISTABLDDIR))
$(BUILD_DRIVERS): CURWINOS=$(if $(findstring _xp_,$@),WINXP,WINVISTA)
$(BUILD_DRIVERS):
	@echo -e "\n=====================================================\n"
	@echo -e "\nRunning $@ now inside $(CURBLDDIR)\n"
	$(MAKE) BUS=$(BUS) WINOS=$(CURWINOS) \
		BLDDIR=$(CURBLDDIR) WDK_VER=$(WDK_VER) \
		$(if $(findstring sdstd,$(BUS)),BUILD_ARCHS=x86) \
		build_driver

$(PREFAST_DRIVERS): BUS=$(if $(findstring _usb_,$@),usb,sdstd)
$(PREFAST_DRIVERS): CURPFDIR=$(if $(findstring _xp_,$@),$(XPPFDIR),$(VISTAPFDIR))
$(PREFAST_DRIVERS): CURWINOS=$(if $(findstring _xp_,$@),WINXP,WINVISTA)
$(PREFAST_DRIVERS):
	@echo -e "\n=====================================================\n"
	@echo -e "\nRunning $@ now inside $(CURPFDIR)\n"
	$(MAKE) BUS=$(BUS) WINOS=$(CURWINOS) \
		BLDDIR=$(CURPFDIR) WDK_VER=$(WDK_VER) \
		$(if $(findstring sdstd,$(BUS)),BUILD_ARCHS=x86) \
		PREFAST=1 \
		prefast_driver

# gen_sources: generates only msft ddk/wdk compatible 'sources' file
# build_sources: builds bcm wl drivers from msft ddk/wdk 'sources' file
build_driver prefast_driver: gen_sources build_sources

ifeq ($(SKIP_GEN_SOURCES),)
gen_sources: prep_sources copy_sources copy_includes
else
gen_sources:
endif

# Prepare the sources file with pre-requisites
prep_sources:
	@echo "*** Preparing $(SOURCES)"
	@if [ -d "$(EMBED_IMG_DIR)" -o "$(EMBED_IMG_NAME) == FakeDongle" ]; then \
		echo -e "Embedded image directory: $(EMBED_IMG_DIR)"; \
	else \
	if [ -d "$(EMBED_IMG_DIR_OLD)" -o "$(EMBED_IMG_NAME) == FakeDongle" ]; then \
		echo -e "Embedded image directory: $(EMBED_IMG_DIR_OLD)"; \
	else \
		echo "Missing embedded image directory $(EMBED_IMG_DIR) $(EMBED_IMG_DIR_OLD)"; \
		exit 1; \
	fi \
	fi
#	For iterative developer build do not purge old BLDDIR
ifndef DEVBUILD
	@if [ -d "$(BLDDIR)" ]; then rm -rf $(BLDDIR); fi
endif #DEVBUILD
	@mkdir -p $(BLDDIR)
	@echo -e "#" > $(SOURCES)
	@echo -e "# Don't edit. This is automagically generated!!" >> $(SOURCES)
	@echo -e "# Use wdk's 'build' to build vista driver\n" >> $(SOURCES)
	@echo -e "# Generated on $(shell date)"          >> $(SOURCES)
	@echo -e "# $(if $(BRAND),BRAND = $(BRAND))\n"   >> $(SOURCES)
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
	@echo -e "                  \0044(DDK_LIB_PATH)/usbd.lib \\" >> $(SOURCES)
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
	@echo -e "!IF \"\0044(DDKBUILDENV)\" == \"chk\"" >> $(SOURCES)
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
	@echo -e "!IF \"\0044(DDKBUILDENV)\" == \"chk\"" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(C_DEFINES) $(WL_CHK_C_DEFINES)" >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
endif # WINOS
	@echo -e "LINKER_FLAGS    = \0044(LINKER_FLAGS) -MAP:\0044(TARGETPATH)/\0044(TARGET_DIRECTORY)/\0044(TARGETNAME).map\n" >> $(SOURCES)
ifeq ($(WINOS),WINVISTA)
	@echo -e "KMDF_VERSION    = 1\n" >> $(SOURCES)
endif # WINVISTA

# List the wl config filtered source files into $(BLDDIR)/sources file
# For developer build, only updated files are copied over and built
copy_sources: prep_sources
	@echo "*** Copying sources from $(WINOS) VPATH=$(WDMVPATH)"
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

build_include:
	@install -pd $(BLDDIR)/src/include
	@install -p  $(SRCBASE)/include/Makefile     $(BLDDIR)/src/include/
	@install -p  $(SRCBASE)/include/epivers.sh   $(BLDDIR)/src/include/
	@install -p  $(SRCBASE)/include/epivers.h.in $(BLDDIR)/src/include/
	@install -p  $(SRCBASE)/wl/config/$(WLTUNEFILE) $(BLDDIR)/src/include/wlconf.h
	@install -p  $(SRCBASE)/include/vendor.h $(BLDDIR)/src/include/
	$(MAKE) -C $(BLDDIR)/src/include

## --------------------------------------------------------------------------
## By default build both x86 and amd64 free/checked drivers
## To build individual driver types, use e.g: make build_x86_checked
build_sources: $(BLD_TEMPLATE) build_include $(BUILD_VARIANTS)

ifeq ($(WINOS),WINVISTA)
$(BUILD_VARIANTS): WDKOS=wlh
else  # XP
$(BUILD_VARIANTS): WDKOS=$(if $(findstring x86,$@),wxp,wnet)
endif # WINOS
$(BUILD_VARIANTS): TYPE=$(if $(findstring free,$@),fre,chk)
$(BUILD_VARIANTS): ARCH=$(word 2,$(subst _,$(space),$@))
$(BUILD_VARIANTS): MSG:=$(shell echo $(ACTION) | tr 'a-z' 'A-Z')
$(BUILD_VARIANTS): BAT=$(ACTION)_$(TYPE)_$(BUS)_$(WDKOS)_$(ARCH)$(BATSUFFIX).bat
$(BUILD_VARIANTS): build_include
$(BUILD_VARIANTS):
	@if [ "$(BAT)" -ot "$(BLD_TEMPLATE)" ]; \
	then \
	   echo "INFO: Generating new $(BAT)"; \
	   sed \
	     -e "s/%TYPE%/$(TYPE)/g"   \
	     -e "s/%WDKOS%/$(WDKOS)/g" \
	     -e "s/%ARCH%/$(ARCH)/g"   \
	     -e "s/%ACTION%/$(ACTION)/g"   \
	     -e "s/%WINOS%/$(WINOS)/g"   \
	     -e "s/%MSG%/$(MSG)/g"   \
	     -e "s/%WDK_VER%/$(WDK_VER)/g"   \
	     -e "s!%BASEDIR%!$(subst /,~,$(BASEDIR_UX))!g"   \
	     -e "s!%BLDDIR%!$(subst /,~,$(CWDIR))!g"   \
	     -e "s/%TITLE%/$(TITLE)_$(MSG)/g"   \
	     -e "s/%PREFAST%/$(if $(PREFAST),prefast \/LOG=$(ACTION)_%VARIANT%.log)/g" \
	  $(BLD_TEMPLATE) | sed -e "s/~/\\\\/g" > $(BAT); \
	else \
	   echo "INFO: Reusing existing $(BAT)"; \
	fi
	$(DOSCMD) /c "$(BAT)"
	$(call POST_BUILD_VERIFY,$(ARCH),$(WDKOS),$(TYPE))
	@echo -e "\n\n"
#	@rm -f $(BAT)

## ---------------------------------------------------------------------------
## In order to speed up build process, generate a wdk build batch file
## template and use it for launching subsequent build variants
## note: wdk build tool exit codes do not indicate some error conditions
## correctly. We need to manually check built objects for correctness!

$(BLD_TEMPLATE):
	@echo -e "@echo off\n\n\
	set VARIANT=%TYPE%_%WDKOS%_%ARCH%\n\
	set PREFIX=%ACTION%%VARIANT%\n\
	echo %WINOS% %MSG% %VARIANT% with %WDK_VER%\n\
	set PATH=%BASEDIR%\\\\bin\\\\x86;%path%\n\n\
	echo Running at %date% %time% on %computername%\n\
	call %BASEDIR%\\\\bin\\\\setenv %BASEDIR% %TYPE% %ARCH% %WDKOS% $(WDK_OACR)\n\
	set MAKEFLAGS=\n\
	cd /D %BLDDIR%\n\
	goto ec%ERRORLEVEL%\n\n\
	:ec0\n\
	if /I NOT \"%cd%\"==\"%BLDDIR%\" goto ec1\n\
	title %TITLE%\n\
	echo Current Dir : %cd%\n\
	%PREFAST% build -be\n\
	set buildec=%ERRORLEVEL%\n\
	if EXIST %PREFIX%.log if DEFINED BRAND type %PREFIX%.log\n\
	if EXIST %PREFIX%.wrn type %PREFIX%.wrn\n\
	if EXIST %PREFIX%.err type %PREFIX%.err\n\
	if /I NOT \"%buildec%\"==\"0\" goto buildec1\n\
	title DONE_%TITLE%\n\
	goto done\n\n\
	:buildec1\n\
	echo %ACTION% failed with error code in '%BLDDIR%'\n\
	exit /B 1\n\n\
	:ec1\n\
	echo Could not change dir to '%BLDDIR%'\n\
	goto done\n\n\
	:buildec0\n\
	:done\n\
	$(if $(PREFAST),rmdir /s /q obj%VARIANT%)\n\
	echo Done with %WINOS% %MSG% of %VARIANT%\n" \
	| sed -e 's/^[[:space:]]//g' > $@

## ---------------------------------------------------------------------------
## Release sign drivers - requires certificate to be installed on local machine
## June 2016: updated to use Utimaco HSM signing credentials server
.PHONY: release_sign_driver
release_sign_driver: SIGN_ARCH := $(subst amd64,X64,$(SIGN_ARCH))
release_sign_driver: PATH := /usr/bin:/bin:$(shell cygpath -u \
     $(subst \,/,$(BASEDIR))/bin/SelfSign):$(shell cygpath -u \
     Z:/projects/hnd/tools/win/msdev/$(WDK_VER)wdk/bin/SelfSign)

MS_CROSSCERT     ?=$(WLAN_WINPFX)/projects/hnd/tools/win/verisign/driver/"DigiCert High Assurance EV Root CA.crt"

release_sign_driver:
	@if [ ! -f "$(SIGN_DIR)/$(SIGN_DRIVER)" ]; then \
	    echo "ERROR:"; \
	    echo "ERROR: $(SIGN_DIR)/$(SIGN_DRIVER) not found to sign"; \
	    echo "ERROR: Does the $(SIGN_DRIVER) exist?"; \
	    echo "ERROR: Verify WINOS or SIGN_ARCH values provided"; \
	    echo "ERROR:"; \
	    exit 1; \
	fi
	@echo -e "\nSigning $(SIGN_ARCH) $(WINOS) drivers from: $(SIGN_DIR)\n"
	@$(SRCBASE)/tools/release/wustamp -o -r -v "$(RELNUM)" -d "$(RELDATE)" "`cygpath -w $(SIGN_DIR)`"
	@cd $(SIGN_DIR) && $(RETRYCMD) signtool sign /v /s my /n "Broadcom Corporation" /t "$(SIGN_TIMESTAMP_URL)" /ac $(MS_CROSSCERT) $(SIGN_DRIVER)
	cd $(SIGN_DIR) && Inf2cat /driver:. /os:$(subst WIN,,$(WINOS))_$(SIGN_ARCH)
	@cd $(SIGN_DIR) && $(RETRYCMD) signtool sign /v /s my /n "Broadcom Corporation" /t "$(SIGN_TIMESTAMP_URL)" /ac $(MS_CROSSCERT) $(SIGN_DRIVERCAT)

# Target to generate xp and vista infs from a common template inf
# and run checkinf on that generated inf file.
generate_infs: build/bcm$(BUS)dhdxp.inf build/bcm$(BUS)dhdvista.inf

build/bcm$(BUS)dhdxp.inf build/bcm$(BUS)dhdvista.inf: bcm$(BUS)dhd.inf FORCE
	@echo -e "\nGenerating $@ from $<"
	@mkdir -pv $(@D)
	@currentos=$(if $(findstring xp.inf,$@),xp,vista); \
	filterout=$(if $(findstring xp.inf,$@),vista,xp); \
	cat $< | $(GENERATE_INF) > $@
	$(call CHECK_INF,$(basename $(@F)))

.PHONY: clean
clean:
	@if [ -d "$(XPBLDDIR)" ]; then \
		echo rm -rf $(XPBLDDIR)/obj{chk,fre}_w*_*; \
		rm -rf $(XPBLDDIR)/obj{chk,fre}_w*_*; \
	fi
	@if [ -d "$(VISTABLDDIR)" ]; then \
		echo rm -rf $(VISTABLDDIR)/obj{chk,fre}_w*_*; \
		rm -rf $(VISTABLDDIR)/obj{chk,fre}_w*_*; \
	fi
	@rm -rf build

.PHONY: clean_all
clean_all:
	rm -f build_ch*.bat build_fre*.bat prefast_ch*.bat prefast_fre*.bat
	rm -f $(BLD_TEMPLATE)
	rm -rf $(XPBLDDIR)
	rm -rf $(VISTABLDDIR)
	rm -rf $(XPPFDIR)
	rm -rf $(VISTAPFDIR)
	rm -rf buildsdstdxp
	rm -rf buildsdstdvista
	@rm -rf build

.PHONY: FORCE
FORCE:

PHONY: all copy_sources copy_includes gen_sources prep_sources build_sources \
       build_driver prefast_driver \
       build_$(BUS)_xp_driver build_$(BUS)_vista_driver \
       prefast_$(BUS)_xp_driver prefast_$(BUS)_vista_driver \
       bcm$(BUS)dhdxp.inf bcm$(BUS)dhdvista.inf \
       $(BUILD_VARIANTS)

#
#
# Makefile to generate XP/Win7/Win8x source tree based on wlconfigs flags and then
# build/sign x86 and amd64 xp drivers.
#
#
# Notes:
#  - This is a standalone makefile to build XP/Win7/Win8x/Win10 drivers using the
#    WDK toolset
#  - For a list of supported targets, run "make help"
#  - For other details, please refer to HowToCompileThings twiki
#
# $Id$

WLAN_ComponentsInUse := accel bcmwifi clm-api ppr olpc keymgmt iocv dump phy ndis xp msch chctx
include ../../../makefiles/WLAN_Common.mk
SRCBASE        := $(WLAN_SrcBaseR)
COMPONENTS_SRCBASE := $(SRCBASE)/../components

SHELL          := bash.exe
empty          :=
space          := $(empty) $(empty)
comma          := $(empty),$(empty)
compose         = $(subst $(space),$(strip $1),$(strip $2))
NULL           := /dev/null
MYTAG          := $(if $(TAG),$(TAG),NIGHTLY)
TITLE          := $(MYTAG)$(if $(BRAND),_$(BRAND))
ACTION         := build
ifndef PARTIAL_BUILD
  export WDKFLAGS?= -Z
endif

ifdef VCTHREADS
  ifndef CL
    $(info Parallel compilation enabled for VS projects)
    export CL := /MP
  endif
  ifeq (,$(filter /m%,$(VSFLAGS)))
    VSFLAGS += /m
  endif
endif

WLTUNEFILE_DEFAULT              := wltunable_sample.h
WLCONF_GEN                      := true
WLCFGDIR                        := $(SRCBASE)/wl/config

#
# To support a TXQ and a non-TXQ version of the Win7 x64 build
#
ifneq ($(TXQ_MUX),)
TXQ_KEY = _txq
endif

#Start: Per WINOS specific settings
XPBLDDIR                        ?= $(ObjPfx)buildxp
WIN7BLDDIR                      ?= $(ObjPfx)buildwin7$(TXQ_KEY)
WIN8XBLDDIR                     ?= $(SRCBASE)/wl/sys/wdm
WIN10BLDDIR                     ?= $(SRCBASE)/wl/sys/wdm

CURWINOS_build_xp_driver        := WINXP
CURWINOS_build_win7_driver      := WIN7
CURWINOS_build_win8x_driver     := WIN8X
CURWINOS_build_win10_driver     := WIN10

CURBLDDIR_WINXP                 := $(XPBLDDIR)
CURBLDDIR_WIN7                  := $(WIN7BLDDIR)
CURBLDDIR_WIN8X                 := $(WIN8XBLDDIR)
CURBLDDIR_WIN10                 := $(WIN10BLDDIR)

# xp nic driver
WL_CONF_WINXP                   := wlconfig_win_nic_xp
WLTUNEFILE_WINXP                := wltunable_win.h
BCMDRVPFX_x86_WINXP             := bcmwl5
BCMDRVPFX_x64_WINXP             := bcmwl564

# win7 nic driver
WL_CONF_WIN7                    := wlconfig_win_nic_win7
WLTUNEFILE_WIN7                 := wltunable_win.h
BCMDRVPFX_x86_WIN7              := bcmwl6
BCMDRVPFX_x64_WIN7              := bcmwl664

# win8x nic driver
WLTUNEFILE_WIN8X                := wltunable_win.h
BCMDRVPFX_x86_WIN8X             := bcmwl63
BCMDRVPFX_x64_WIN8X             := bcmwl63a

# win10 nic driver
WLTUNEFILE_WIN10                := wltunable_win.h
BCMDRVPFX_x86_WIN10             := bcmwl63
BCMDRVPFX_x64_WIN10             := bcmwl63a
#End: Per WINOS specific settings

# Derive Current WINOS from current target name
CURWINOS                         = $(CURWINOS_$(@))
# Derive Current WINOS from current target name
CURBLDDIR                        = $(CURBLDDIR_$(CURWINOS))

# Final derived variables from above variants (Default WINOS is WIN7)
# WIN8_WDK_VER name preserved for backward compat with brand makes, not WIN8X here.
export WDK_VER         ?= 7600
export WIN8_WDK_VER    ?= 9600
export WIN10_WDK_VER   ?= 10240
VS_VER                 ?= 2010
export WINOS           ?= $(if $(CURWINOS),$(CURWINOS),WIN7)
export BLDDIR          ?= $(if $(CURBLDDIR),$(CURBLDDIR),$(CURBLDDIR_$(WINOS)))
export CONFIG_WL_CONF  ?= $(WL_CONF_$(WINOS))
export WLTUNEFILE      ?= $(if $(WLTUNEFILE_$(WINOS)),$(WLTUNEFILE_$(WINOS)),$(WLTUNEFILE_DEFAULT))
export BCMDRVPFX_x86   ?= $(BCMDRVPFX_x86_$(WINOS))
export BCMDRVPFX_x64   ?= $(BCMDRVPFX_x64_$(WINOS))
export BCMDRVPFX_amd64 ?= $(BCMDRVPFX_x64)
export WLCONF_GEN

# Co-Installer KMDF versions differ in different WDK distributions
WDK_KMDF_VERSION           := 1
ifeq ($(WDK_VER),6000)
  WDK_KMDF_VERSION         := 1.5
endif
ifeq ($(WDK_VER),6001)
  WDK_KMDF_VERSION         := 1.7
endif
ifneq ($(filter 7% 8%,$(WDK_VER)),)
  WDK_KMDF_VERSION         := 1.9
  export WDK_OACR          ?= no_oacr
endif
ifneq ($(filter 7% 8%,$(WDK_VER)),)
  WDK_KMDF_VERSION_MAJOR   := 1
  WDK_KMDF_VERSION_MINOR   := 9
  export WDK_OACR          ?= no_oacr
endif

BLDDIRDOS := $(subst /,\,$(BLDDIR))
SOURCES   := $(BLDDIR)/sources
CWDIR     := $(shell cygpath -m -a "$(BLDDIR)" 2> $(NULL))

## Following error condition very rarely fires
ifeq ($(findstring //,$(CWDIR)),//)
  $(error "ERROR: Can't use UNC style paths: $(CWDIR)")
endif

## For now if BRAND is not specified it is assumed to be a developer build
ifeq ($(BRAND),)
  DEVBUILD     := 1
endif

ifneq ($(BRAND),)
  export VERBOSE=1
endif

PREPWDK        := perl prepwdkdir.pl
RETRYCMD       ?= $(firstword \
                  $(wildcard \
                     C:/tools/build/bcmretrycmd.exe \
                     $(WLAN_WINPFX)/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                  ))
WDKDIR         := $(strip $(if $(BASEDIR),$(BASEDIR),$(subst /,\,$(firstword $(wildcard \
			C:/WinDDK/$(WDK_VER) C:/WINDDK/$(WDK_VER).0.0 \
			D:/WinDDK/$(WDK_VER) D:/WinDDK/$(WDK_VER).0.0 \
			E:/WinDDK/$(WDK_VER) E:/WinDDK/$(WDK_VER).0.0 \
			C:/tools/msdev/$(WDK_VER)wdksdk \
			C:/tools/msdev/$(WDK_VER)wdk \
			C:/tools/mdev/wdk$(WDK_VER) \
			C:/tools/mdev/wdk$(WDK_VER).0.0 \
			D:/tools/msdev/$(WDK_VER)wdksdk \
			D:/tools/msdev/$(WDK_VER)wdk \
			$(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(WDK_VER)wdk)))))
WDKDIR_UX      := $(subst \,/,$(WDKDIR))
VSDIR          := $(strip $(if $(VSINSTALLDIR),$(VSINSTALLDIR),$(subst /,\,$(firstword $(wildcard \
			C:/tools/msdev/VS$(VS_VER) \
			D:/tools/msdev/VS$(VS_VER) \
			$(WLAN_WINPFX)/projects/hnd/tools/win/msdev/VS$(VS_VER))))))
VCDIR          := $(VSDIR)/VC
VSDIR_UX       := $(subst \,/,$(VSDIR))
VCDIR_UX       := $(subst \,/,$(VCDIR))
DOSCMD         := $(subst \,/,$(COMSPEC))
# In clustered environments, if a preconfigured dir is not accessible use $(WLAN_WINPFX)
BASEDIR         = $(shell if [ -d "$(WDKDIR)" ]; then echo "$(WDKDIR)"; else echo $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(WDK_VER)wdk; fi)
BASEDIR_UX     := $(subst \,/,$(BASEDIR))


# hardcoded overrides not scalable but makefile will be rebuilt so use ifeq for now
ifneq ($(filter WIN10%,$(WINOS)),)
WIN10_VSDIR :=$(strip $(firstword $(wildcard $(foreach suffix,2015 2013 2012 2010,$(foreach prefix,c:/tools/msdev/VS d:/tools/msdev/VS,$(strip $(prefix))$(strip $(suffix)))))))
else ifneq ($(filter WIN8X%,$(WINOS)),)
WIN8X_VSDIR :=$(strip $(firstword $(wildcard $(foreach suffix,2013 2012 2010,$(foreach prefix,c:/tools/msdev/VS d:/tools/msdev/VS,$(strip $(prefix))$(strip $(suffix)))))))
else #pre-win8x
WIN8X_VSDIR :=$(strip $(firstword $(wildcard $(foreach suffix,2012 2010,$(foreach prefix,c:/tools/msdev/VS d:/tools/msdev/VS,$(strip $(prefix))$(strip $(suffix)))))))
endif

ifneq ($(findstring WIN10,$(WINOS)),)
WIN10_WDKDIR :=$(strip $(firstword $(wildcard $(foreach prefix,c:/tools/ d:/tools/ $(WLAN_WINPFX)/projects/hnd/tools/win/,$(foreach suffix,wdk wdksdk,$(strip $(prefix))msdev/$(WIN10_WDK_VER)$(strip $(suffix)))))))
else ifneq ($(findstring WIN8X,$(WINOS)),)
WIN8X_WDKDIR :=$(strip $(firstword $(wildcard $(foreach prefix,c:/tools/ d:/tools/ $(WLAN_WINPFX)/projects/hnd/tools/win/,$(foreach suffix,wdk wdksdk,$(strip $(prefix))msdev/$(WIN8_WDK_VER)$(strip $(suffix)))))))
endif

#disabled# # When new MS tools are all deployed across all servers
#disabled# # enable this
#disabled# ifeq ($(VSDIR),)
#disabled# $(error ERROR: Missing Visual Studio $VS_VER Installation)
#disabled# endif

# WDK produces a mix of x64 and amd64 folder and logfile names by default
# e.g: setenv and signtool use x64 and build.exe uses amd64. Default values
#      are not overriden. So BLDOUT_ARCH_<arch> shows output log/dir name
ifneq ($(filter WIN8X% WIN10%,$(WINOS)),)
  BLDOUT_ARCH_x86 := Win32
  BLDOUT_ARCH_x64 := x64
else
  BLDOUT_ARCH_x86 := x86
  BLDOUT_ARCH_x64 := amd64
endif
ifneq ($(filter 10%,$(WIN10_WDK_VER)),)
  BUILD_ARCHS  ?= x86 x64
else ifneq ($(filter 9%,$(WIN8_WDK_VER)),)
  BUILD_ARCHS  ?= x86 x64
else ifeq ($(filter 6001% 7% 8%,$(WDK_VER)),)
  BUILD_ARCHS  ?= x86 amd64
else
  BUILD_ARCHS  ?= x86 x64
endif
BUILD_TYPES    ?= checked free
#No Inbox BUILD_TYPES    ?= checked free $(if $(filter WIN8X%,${WINOS}),checkedexternal)
BUILD_VARIANTS ?= $(foreach arch,$(BUILD_ARCHS),$(foreach type,$(BUILD_TYPES),$(ACTION)_$(arch)_$(type)))
BLD_TEMPLATE   := .temp_wdk_build_template_$(WINOS).bat
MSBLD_TEMPLATE := .temp_msbuild_template_$(WINOS).bat
export PATH    := /usr/bin:/bin:$(PATH):$(shell cygpath -u $(subst \,/,$(BASEDIR))/bin/SelfSign):$(shell cygpath -u $(subst \,/,$(BASEDIR))/bin/x86):


OTHER_INC_DIRS :=
INC_DIRS       :=

## Prerelease contains microsoft wdk/ddk pre-release header files
ifeq ($(filter 6001% 7% 8%,$(WDK_VER)),)
  ifneq ($(filter WIN7%,$(WINOS)),)
    INC_DIRS         += $(SRCBASE)/include/prerelease
    PRERELEASE_FILES := $(SRCBASE)/include/prerelease/windot11.h
  endif # WIN7
endif # WDK_VER

INC_DIRS  += $(WLAN_ComponentIncDirsR)
INC_DIRS  += $(WLAN_IncDirsR)
INC_DIRS  += $(COMPONENTS_SRCBASE)/drivers/wl/shim/include
INC_DIRS  += $(COMPONENTS_SRCBASE)/chips/sr_asm
INC_DIRS  += $(SRCBASE)/wl/sys/wdm
ifneq ($(WL_FW_DECOMP)),)
INC_DIRS  += $(SRCBASE)/shared/zlib
endif

ifneq ($(filter WIN7%,$(WINOS)),)
  INC_DIRS+= $(COMPONENTS_SRCBASE)/apps/windows/ihvfrm/ihv/IHVService/inc
  ifneq ($(BCMCCX),0)
    # If you update CCX_SDK_VER here, you need to do similar change for
    # ihv project in src/makefiles/msvs_defs.mk
    CCX_SDK_VER    ?= 1.1.13
    OTHER_INC_DIRS += $(WLAN_WINPFX)/projects/hnd/restrictedTools/CCX_SDK/$(CCX_SDK_VER)/inc
  endif # BCMCCX=0
endif # WINOS

ifeq ($(filter WIN7%,$(WINOS)),)
  ## Include dirs outside of cvs source tree
  FIPS_DIR       := $(firstword $(wildcard C:/tools/src/fips/funk D:/tools/src/fips/funk $(WLAN_WINPFX)/projects/hnd/tools/fips/funk))
  FIPS_DIR       := $(shell if [ -d "$(FIPS_DIR)" ]; then echo "$(FIPS_DIR)"; else echo $(WLAN_WINPFX)/projects/hnd/tools/fips/funk; fi)
  FIPS_INC_DIR    = $(FIPS_DIR)/inc
endif # WINOS != WIN7

ifdef DEVBUILD
  BSRCBASE  := $(shell cygpath -m $(SRCBASE))
  ifeq ($(filter 6001% 7% 8%,$(WDK_VER)),)
    ifneq ($(filter WIN7%,$(WINOS)),)
      BINC_DIRS := $(subst $(SRCBASE),$(BSRCBASE),$(SRCBASE)/include/prerelease)
    endif # WIN7
  endif # WDK_VER
  BINC_DIRS += $(BASEDIR_UX)/inc/api
  BINC_DIRS += src/include
  BINC_DIRS += $(subst $(SRCBASE),$(BSRCBASE),$(INC_DIRS))
  BINC_DIRS := $(subst //,/,$(BINC_DIRS))
  ifeq ($(filter WIN7%,$(WINOS)),)
    OTHER_INC_DIRS += $(FIPS_INC_DIR)
  endif # WINOS != WIN7
else
  ifeq ($(filter WIN7%,$(WINOS)),)
    INC_DIRS       += $(SRCBASE)/wl/sys/fips/funk/inc
  endif # WINOS != WIN7
endif # DEVBUILD

EXTRA_C_DEFINES   += -DNDIS -DNDIS_MINIPORT_DRIVER -DNDIS_WDM -DWDM -DDELTASTATS
EXTRA_C_DEFINES   += -DBCMDRIVER


#ifdef WLTEST
ifdef WLTEST
  EXTRA_C_DEFINES += -DWLTEST
endif
#endif /* WLTEST */
EXTRA_C_DEFINES   += -DNDIS_DMAWAR
EXTRA_C_DEFINES   += $(if $(BRAND),-DBRAND=$(BRAND))
WL_CHK_C_DEFINES  := -DBCMDBG -DBCMINTERNAL -DBCMDBG_MEM

ifneq ($(filter WIN7%,$(WINOS)),)
  EXTRA_C_DEFINES += -DNDIS60_MINIPORT=1 -DNDIS620_MINIPORT=1 -DNDIS620 -DWIN7
endif

ifneq ($(filter WINXP%,$(WINOS)),)
    EXTRA_C_DEFINES += -DNDIS51_MINIPORT -DNDIS51
    MSX86_C_DEFINES := -UWINVER -DWINVER=0x0500
endif

ifneq ($(CONFIG_WL_CONF),)
  include $(WLCFGDIR)/$(CONFIG_WL_CONF)
  include $(WLCFGDIR)/wl.mk
endif

## Aggregate individual flags/defines
C_DEFINES  = $(WLFLAGS) $(EXTRA_C_DEFINES) $(MY_C_DEFINES)
C_SOURCES  = $(WLFILES_SRC) $(EXTRA_C_SOURCES)
INC_FLAGS  = $(INC_DIRS:%=-I%)

ifdef SIGN_DIR
  BCMCAT_x86_NIC             := bcm43xx.cat
  BCMCAT_x64_NIC             := bcm43xx64.cat

  BCMCAT_x86_WINXP           := $(BCMCAT_x86_NIC)
  BCMCAT_x64_WINXP           := $(BCMCAT_x64_NIC)

  BCMCAT_x86_WIN7            := $(BCMCAT_x86_NIC)
  BCMCAT_x64_WIN7            := $(BCMCAT_x64_NIC)

  BCMCAT_x86_WIN8X         := $(BCMCAT_x86_NIC)
  BCMCAT_x64_WIN8X         := $(BCMCAT_x64_NIC)

  BCMCAT_x86_WIN10         := $(BCMCAT_x86_NIC)
  BCMCAT_x64_WIN10         := $(BCMCAT_x64_NIC)

  SIGN_LOG         := $(CWDIR)/../signtool.log
  RELNUM           ?= $(shell date '+%Y.%-m.%-d.%H%M')
  RELDATE          ?= $(shell date '+%m/%d/%Y')
  MS_CROSSCERT     ?=$(WLAN_WINPFX)/projects/hnd/tools/win/verisign/driver/"DigiCert High Assurance EV Root CA.crt"
  # By default sign only 64bit driver, unless SIGN_ARCH is specified
  SIGN_ARCH        ?=amd64
  SIGN_ARCH        := $(subst amd64,X64,$(SIGN_ARCH))
  SIGN_DRIVER      ?=$(BCMDRVPFX_x64).sys
  SIGN_DRIVERCAT   ?=$(BCMCAT_x64_$(WINOS))

  ifneq ($(filter %86,$(SIGN_ARCH)),)
    SIGN_DRIVER    :=$(subst $(BCMDRVPFX_x64),$(BCMDRVPFX_x86),$(SIGN_DRIVER))
    SIGN_DRIVERCAT :=$(subst 64.cat,.cat,$(SIGN_DRIVERCAT))
  endif # SIGN_ARCH

  SIGN_OS_WINXP        := XP2K
  SIGN_OS_WIN7         := 7
  SIGN_OS_WIN8X        := 6_3
  SIGN_OS_WIN10        := 6_3
  SIGN_OS               = $(SIGN_OS_$(WINOS))
endif # SIGN_DIR

# Stuff related to copying .vcxproj build source files to other (local)
# directory before build
# This build mode is enabled by setting COPY_VCXPROJ_TARGET make parameter.
# Its value is a path to directory to which sources shall be copied. This
# path shall be specified as _absolute_. Path format shall be compatible
# with Python interpreter being used - probably the best is mixed
ifdef COPY_VCXPROJ_TARGET
  # Root directory of all source files
  COPY_VCXPROJ_SOURCE := $(SRCBASE)/..

  # Projection of current directory (that, ostensibly, contains .vcxproj file) after copy to destination directory
  COPY_VCXPROJ_CUR_DIR_PROJECTION := $(shell cygpath -m $(COPY_VCXPROJ_TARGET)/$(call relpath,$(COPY_VCXPROJ_SOURCE),.)/)

  # Script that copies source files
  COPY_VCXPROJ_SCRIPT := $(WLAN_WINPFX)/projects/hnd/tools/infrastructure/helpers/copy_vcxproj.py
endif # COPY_VCXPROJ_TARGET

# WDK build tool exit code may not indicate error code, if driver is not built!
# So this function verified explicitly if the driver was built or not
OBJDIR_x86   := i386
OBJDIR_x64   := amd64
OBJDIR_amd64 := amd64
## Args $1=ARCH, $2=WDKOS, $3=TYPE
ifneq ($(findstring WIN10,$(WINOS)),)
define POST_BUILD_VERIFY
	@echo -e "WIN10 POST_BUILD_VERIFY \n"
	@echo "#- $0($1,$2,$3)"
	@objpath="obj/$(2)/$(3)/$(1)"; \
	sys="$${objpath}/$(BCMDRVPFX_$(1)).sys"; \
	if [ ! -f "$${sys}" ]; then \
	   echo "ERROR:"; \
	   echo "ERROR: $${sys} BUILD FAILED"; \
	   echo "ERROR:"; \
	   exit 1; \
	fi
	rm -rf win10_nic_$(TYPE)_$(BLDOUT_ARCH_$(ARCH))
endef # POST_BUILD_VERIFY
else ifneq ($(findstring WIN8X,$(WINOS)),)
define POST_BUILD_VERIFY
	@echo -e "WIN8 POST_BUILD_VERIFY \n"
	@echo "#- $0($1,$2,$3)"
	@objpath="obj/$(2)/$(3)/$(1)"; \
	sys="$${objpath}/$(BCMDRVPFX_$(1)).sys"; \
	if [ ! -f "$${sys}" ]; then \
	   echo "ERROR:"; \
	   echo "ERROR: $${sys} BUILD FAILED"; \
	   echo "ERROR:"; \
	   exit 1; \
	fi
	rm -rf win8x_nic_$(TYPE)_$(BLDOUT_ARCH_$(ARCH))
endef # POST_BUILD_VERIFY
else # pre-Win8x
define POST_BUILD_VERIFY
	@echo -e "WIN7 POST_BUILD_VERIFY \n"
	@echo "#- $0($1,$2,$3)"
	@objpath="obj$(3)_$(2)_$(if $(findstring 64,$1),amd64,x86)"; \
	sys="$(BLDDIR)/$${objpath}/$(OBJDIR_$(1))/$(BCMDRVPFX_$(1)).sys"; \
	if [ ! -f "$${sys}" ]; then \
	   echo "ERROR:"; \
	   echo "ERROR: $${sys} BUILD FAILED"; \
	   echo "ERROR:"; \
	   exit 1; \
	fi
endef # POST_BUILD_VERIFY
endif # WINOS

## Default list of build targets
all: build_xp_driver
all: build_win7_driver

# Win8 driver target is conditionally launched for certain brands
ifneq ($(findstring win8x_,$(BRAND)),)
all: build_win8x_driver
endif # win8x

# Win10 driver target is conditionally launched for certain brands
ifneq ($(findstring win10_,$(BRAND)),)
all: build_win10_driver
endif # win10

showinfo:
	@echo "============================================================="
	@echo "$(CURWINOS) BLDDIR   = $(CURBLDDIR)"
	@echo "$(CURWINOS) WL_CONF  = $(CONFIG_WL_CONF)"
	@echo "$(CURWINOS) SOURCES  = $(sort $(notdir $(C_SOURCES)))"
	@echo "$(CURWINOS) INC_DIRS = $(INC_DIRS) $(OTHER_INC_DIRS)"
	@echo "-------------------------------------------------------------"
	@echo "$(CURWINOS) WLFLAGS          = $(sort $(WLFLAGS))"
	@echo "$(CURWINOS) EXTRA_C_DEFINES  = $(sort $(EXTRA_C_DEFINES))"
	@echo "$(CURWINOS) MY_C_DEFINES     = $(sort $(MY_C_DEFINES))"
	@echo "$(CURWINOS) C_DEFINES        = $(sort $(C_DEFINES))"
	@echo "============================================================="

# WINOS specific build targets
build_xp_driver build_win7_driver \
build_win8x_driver build_win10_driver:
	@echo -e "\n=====================================================\n"
	@echo -e "\nRunning $@ now [$$(date)]\n"
	$(MAKE) build_driver

# Core (final) driver build target
# gen_sources: generates only msft ddk/wdk compatible 'sources' file
# build_sources: builds bcm wl drivers from msft ddk/wdk 'sources' file
build_driver: showinfo gen_sources build_sources

GENERATED_SOURCES := wlc_clm_data.c d11shm.c

# specifies temp folder for use by d11shm
# folder is created in d11shm.mk and deleted in this file after copying
# to bld folder.
D11SHM_TEMPDIR := $(shell mktemp -d -p . d11shm.XXXXXXXXXX)

ifeq ($(SKIP_GEN_SOURCES),)
  ifneq (,$(filter WIN8X% WIN10%,${WINOS}))
    gen_sources: clm_compiled d11_create
    gen_sources: $(addprefix copy_dongle_headers-,${BUILD_VARIANTS})
  else
    gen_sources: prep_sources copy_sources copy_includes
  endif
else
  gen_sources:
endif # SKIP_GEN_SOURCES

# Prepare the sources file with pre-requisites
prep_sources: _prep_sources

_prep_sources:
ifeq ($(SKIP_PREP_SOURCES),)
	@echo -e "#\n# Preparing $(SOURCES) file now\n#\n"
	@[ -d $(BLDDIR) ] || mkdir -pv $(BLDDIR)
	@echo -e "#" > $(SOURCES)
	@echo -e "# Don't edit. This is automagically generated!!" >> $(SOURCES)
	@echo -e "# Use 'build.exe' from wdk to build wl driver\n" >> $(SOURCES)
	@echo -e "# WDK $(WDK_VER) sources file for $(WINOS)"      >> $(SOURCES)
	@echo -e "# Generated on $(shell date)"          >> $(SOURCES)
	@echo -e "# $(if $(BRAND),BRAND = $(BRAND))\n"   >> $(SOURCES)
#	For developer build and for better build performance
#	we skip copy_includes rule and refer to original headers
ifdef DEVBUILD
	@echo -e "SRCBASE         = $(BSRCBASE)\n" >> $(SOURCES)
	@echo -e "WDKDIR          = $(BASEDIR_UX)\n" >> $(SOURCES)
	@echo -e "INCLUDES        = $(subst $(space),; ,$(subst $(BSRCBASE),\0044(SRCBASE)/,$(strip $(BINC_DIRS)) $(strip $(OTHER_INC_DIRS)))); \0044(DDK_INC_PATH); \0044(INCLUDE)"  >> $(SOURCES)
else #DEVBUILD
	@echo -e "SRCBASE         = src\n" >> $(SOURCES)
	@echo -e "INCLUDES        = $(subst $(space),; ,$(subst $(SRCBASE),\0044(SRCBASE),$(strip $(INC_DIRS)) $(strip $(OTHER_INC_DIRS)))); \0044(DDK_INC_PATH); \0044(INCLUDE)"  >> $(SOURCES)
endif #DEVBUILD
	@echo -e "TARGETTYPE      = DRIVER"              >> $(SOURCES)
	@echo -e "TARGETPATH      = obj"                 >> $(SOURCES)
	@echo -e "TARGETLIBS      = \0044(DDK_LIB_PATH)/ndis.lib \\" >> $(SOURCES)
	@echo -e "\n"                                    >> $(SOURCES)
ifneq ($(findstring WINXP,$(WINOS)),)
	@echo -e "!IF \"\0044(_BUILDARCH)\" == \"x86\""  >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_x86)"    >> $(SOURCES)
	@echo -e "!ELSE"                                 >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_x64)"    >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
	@echo -e "C_DEFINES       = $(C_DEFINES) $(MSX86_C_DEFINES)\n" >> $(SOURCES)
	@echo -e "!IF \"\0044(DDKBUILDENV)\" == \"chk\"" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(C_DEFINES) $(WL_CHK_C_DEFINES)\n" >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
else # VISTA
	@echo -e "!IF \"\0044(_BUILDARCH)\" == \"x86\""  >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_x86)"    >> $(SOURCES)
	@echo -e "!ELSE"                                 >> $(SOURCES)
	@echo -e "TARGETNAME      = $(BCMDRVPFX_x64)"    >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
	@echo -e "C_DEFINES       = $(C_DEFINES)\n"      >> $(SOURCES)
	@echo -e "!IF \"\0044(DDKBUILDENV)\" == \"chk\"" >> $(SOURCES)
	@echo -e "C_DEFINES       = \0044(C_DEFINES) $(WL_CHK_C_DEFINES)" >> $(SOURCES)
	@echo -e "!ENDIF\n"                              >> $(SOURCES)
endif # WINOS
	@echo -e "LINKER_FLAGS    = \0044(LINKER_FLAGS) -MAP:\0044(TARGETPATH)/\0044(TARGET_DIRECTORY)/\0044(TARGETNAME).map\n"          >> $(SOURCES)
endif # SKIP_PREP_SOURCES

# Update the CLM database C code from XML inputs if present.
CLM_TYPE := 43xx_pcoem
ifneq ($(findstring WIN10,$(WINOS)),)
$(foreach arch,$(BUILD_ARCHS),\
  $(call WLAN_GenClmCompilerRule,$(BLDDIR)/obj/win10_nic/$(BUILD_TYPES)/$(arch),$(SRCBASE)))
else ifneq ($(findstring WIN8X,$(WINOS)),)
$(foreach arch,$(BUILD_ARCHS),\
  $(call WLAN_GenClmCompilerRule,$(BLDDIR)/obj/win8x_nic/$(BUILD_TYPES)/$(arch),$(SRCBASE)))
else # pre-Win8x
$(call WLAN_GenClmCompilerRule,$(BLDDIR),$(SRCBASE))
endif

prep_sources: clm_compiled d11_create

# List the wl config filtered source files into $(BLDDIR)/sources file
# For developer build, only updated files are copied over and built
copy_sources: prep_sources
ifeq ($(SKIP_COPY_SOURCES),)
	@echo -e "#\n# Copying wl source files now\n#\n"
	$(PREPWDK) -install -output "$(SOURCES)" -blddir "$(BLDDIR)" -generated "$(GENERATED_SOURCES)" -treebase "$(dir $(abspath $(SRCBASE)))" -src_files "src/wl/sys/wl.rc $(C_SOURCES)"
endif # SKIP_COPY_SOURCES

# Scan the source files to generate list of wl header files
# needed to compile sources generated in 'sources' make rule.
copy_includes: copy_sources
ifndef SKIP_COPY_INCLUDES
ifndef DEVBUILD
	@echo "WDK ntddndis.h replaces src/wl/sys/ntddndis.h"
	$(strip python $(SRCBASE)/tools/build/wdkinc.py \
	    --tree-base=$(SRCBASE)/.. --to-dir=$(BLDDIR) --skip wl/sys/ntddndis.h \
	    $(if $(filter WIN7%,$(WINOS)),,--copy-tree $(FIPS_INC_DIR) $(BLDDIR)/src/wl/sys/fips/funk/inc) \
	    -- $(INC_FLAGS) $(C_DEFINES) $(C_SOURCES) \
	    $(if $(filter 6001% 7% 8%,$(WDK_VER)),,$(PRERELEASE_FILES)))
endif # DEVBUILD
endif # SKIP_COPY_INCLUDES

build_include:
ifneq ($(findstring WIN10,$(WINOS)),)
	@install -pvD  $(SRCBASE)/wl/config/$(WLTUNEFILE) $(BLDDIR)/obj/win10_nic/$(BUILD_TYPES)/x86/wlconf.h
	@install -pvD  $(SRCBASE)/wl/config/$(WLTUNEFILE) $(BLDDIR)/obj/win10_nic/$(BUILD_TYPES)/x64/wlconf.h
	@install -p  $(D11SHM_TEMPDIR)/d11* $(BLDDIR)/obj/win10_nic/$(BUILD_TYPES)/x86/
	@install -p  $(D11SHM_TEMPDIR)/d11* $(BLDDIR)/obj/win10_nic/$(BUILD_TYPES)/x64/
	rm -rf $(D11SHM_TEMPDIR)
	$(MAKE) -C $(SRCBASE)/include
else ifneq ($(findstring WIN8X,$(WINOS)),)
	@install -pvD  $(SRCBASE)/wl/config/$(WLTUNEFILE) $(BLDDIR)/obj/win8x_nic/$(BUILD_TYPES)/x86/wlconf.h
	@install -pvD  $(SRCBASE)/wl/config/$(WLTUNEFILE) $(BLDDIR)/obj/win8x_nic/$(BUILD_TYPES)/x64/wlconf.h
	@install -p  $(D11SHM_TEMPDIR)/d11* $(BLDDIR)/obj/win8x_nic/$(BUILD_TYPES)/x86/
	@install -p  $(D11SHM_TEMPDIR)/d11* $(BLDDIR)/obj/win8x_nic/$(BUILD_TYPES)/x64/
	rm -rf $(D11SHM_TEMPDIR)
	$(MAKE) -C $(SRCBASE)/include
else # pre-Win8x
	@install -pd $(BLDDIR)/src/include
	@install -p  $(SRCBASE)/include/Makefile     $(BLDDIR)/src/include/
	@install -p  $(SRCBASE)/include/epivers.sh   $(BLDDIR)/src/include/
	@install -p  $(SRCBASE)/include/epivers.h.in $(BLDDIR)/src/include/
	@if [ ! -s "$(BLDDIR)/src/include/epivers.h" -a \
		-s "$(SRCBASE)/include/epivers.h" ]; then \
	    install -pv  $(SRCBASE)/include/epivers.h $(BLDDIR)/src/include/; \
	fi
	@install -p  $(SRCBASE)/wl/config/$(WLTUNEFILE) $(BLDDIR)/src/include/wlconf.h
	@install -p  $(SRCBASE)/include/vendor.h $(BLDDIR)/src/include/
	$(MAKE) -C $(BLDDIR)/src/include
endif

# Auto-SHM related stuff
D11SHM_WOWL = 1
D11SHM_ULP = 1
D11SHM_NAMED_INIT = 0

ifneq ($(findstring WIN10,$(WINOS)),)
D11SHM_CFLAGS = /FI./wlconfig_win_nic_win8x.h
else ifneq ($(findstring WIN8X,$(WINOS)),)
D11SHM_CFLAGS = /FI./wlconfig_win_nic_win8x.h
else # pre-Win8x
D11SHM_CFLAGS := $(C_DEFINES)
endif
D11SHM_WIN = 1
D11SHM_CFGFILE := $(SRCBASE)/wl/sys/wlc_cfg.h
include $(SRCBASE)/makefiles/d11shm.mk

wlconf.h:
	touch wlconf.h

.PHONY: d11_create
d11_create: wlconf.h $(D11SHM_HEADER)
ifneq ($(findstring WIN10,$(WINOS)),)
#  For Win10, d11 shm files are installed at different build stage
else ifneq ($(findstring WIN8X,$(WINOS)),)
#  For Win8x, d11 shm files are installed at different build stage
else
	@echo -e "Install pre-Win8x D11 SHM files"
	@install -p  $(D11SHM_TEMPDIR)/d11* $(BLDDIR)/
	rm -rf $(D11SHM_TEMPDIR)
endif

# .wdkos (build-variant [from ${BUILD_VARIANTS}]):
#
# Depends on variables:
#     WINOS - passed into makefile, or taking default value (WIN7).
#
# Description:
#     Produces the definitive Windows Driver Kit operating system tag,
#     useful for feeding the MS development suite, to target a particular
#     operating system, running on a particular architecture.
#
.wdkos   = $(strip \
             $(or \
               $(if $(filter WIN10,${WINOS}),win10_nic) \
               ,$(if $(filter WIN8X,${WINOS}),win8x_nic) \
               ,$(if $(filter WIN7%,${WINOS}),win7) \
               ,$(if $(filter WINXP%,${WINOS}), \
                  $(if $(findstring x86,$1),wxp,wnet) \
                 ) \
               ,$(error UNKNOWN WINDOWS OS: '${WINOS}') \
              ) \
            )

# .type (build-variant [from ${BUILD_VARIANTS}]):
#
# Depends on variables:
#     WINOS - passed into makefile, or taking default value (WIN7).
#
# Description:
#     Produces the definitive build type (free or checked), based upon
#     the value of $WINOS, and the given build variant.
#
.type    = $(strip \
             $(if $(filter WIN8X% WIN10%,${WINOS}), \
               $(or $(findstring free,$1), \
                 $(or $(findstring checkedexternal,$1),checked) \
                ), \
               $(if $(filter WIN8X% WIN10%,${WINOS}), \
                 $(or $(findstring free,$1),checked), \
                 $(if $(findstring free,$1),fre,chk) \
                ) \
              ) \
            )

# .arch (build-variant [from ${BUILD_VARIANTS}]):
#
# Description:
#     Produces the build target architecture, from the given variant.
#
.arch    = $(strip $(word 2,$(subst _, ,$1)))

# .msg (build-variant [from ${BUILD_VARIANTS}]):
#
# Description:
#     Produces an upper-case "tag" for the error message, which is then
#     substituted into the Windows batch file which actually runs the
#     build automatically.
#
.msg     = $(strip \
             $(shell \
               echo '$(firstword $(subst _, ,$1))' \
                 | tr '[:lower:]' '[:upper:]' \
              ) \
            )

# .action (build-variant [from ${BUILD_VARIANTS}]):
#
# Description:
#     Produces the name of the activity associated with the given build variant.
#
.action  = $(firstword $(subst _, ,$1))

# .bat (build-variant [from ${BUILD_VARIANTS}]):
#
# Depends on variables:
#     WINOS  - passed into makefile, or taking default value (WIN7).
#     ObjPfx - Provided by a different makefile, this is where the
#              build will deposit build artifacts.
#
# Description:
#     Produces the name of the batch file which will be generated
#     by a rule within this file.  The file so generated is the actual
#     executable entity which eventually starts the build on a Windows
#     build host.
#
.bat     = $(strip \
             $(shell cygpath -m \
               $(call compose,, \
                 ${ObjPfx} \
                 $(call compose,_, \
                   $(call .action,$1) $(call .type,$1) \
                   ${WINOS} $(call .arch,$1)$(TXQ_KEY) \
                  ) \
                 .bat \
                ) \
              ) \
            )

# .bldoutarch (build-variant [from ${BUILD_VARIANTS}]):
#
# Description:
#     Produces the name of the build output architecture, which is then
#     substituted into the Windows batch file, generated by a rule within
#     this makefile.
#
.bldoutarch  = $(strip ${BLDOUT_ARCH_$(word 2,$(subst _, ,$1))})

# .bldtemplate (none):
#
# Depends on variables:
#     WINOS          - passed into makefile, or taking default value (WIN7).
#     MSBLD_TEMPLATE - Win8/Win8X variant of the build batch script.
#     BLD_TEMPLATE   - Other Windows target variant of the build batch script.
#
# Description:
#     Dependent upon the value found in $WINOS, produces either the name of
#     the Win8/Win8X build script, or the name of the build script which
#     executes the build for other Windows platforms.
#
.bldtemplate = $(strip \
                 $(if $(filter WIN8X% WIN10%,${WINOS}) \
                   , ${MSBLD_TEMPLATE} \
                   , ${BLD_TEMPLATE} \
                  ) \
                )


## --------------------------------------------------------------------------
## By default build both x86 and amd64 free/checked drivers
## To build individual driver types, use e.g: make build_x86_checked
build_sources: $(call .bldtemplate) build_include ${BUILD_VARIANTS}
build_dongle_images: build_include

# This loop constructs a set of target-specific variables, which are
# useful both for copying a per-configuration set of dongle headers
# into place, and also directs the driver build to depend upon the
# associated dongle header copy.
$(foreach v,${BUILD_VARIANTS}, \
  $(eval .PHONY: $v copy_dongle_headers-$v) \
  $(eval $v: copy_dongle_headers-$v) \
  $(eval $v: $(call .bldtemplate)) \
  $(eval $v copy_dongle_headers-$v: BLD_TEMPLATE := $(call .bldtemplate)) \
  $(eval $v copy_dongle_headers-$v: BLDOUTARCH   := $(call .bldoutarch,$v)) \
  $(eval $v copy_dongle_headers-$v: WDKOS        := $(call .wdkos,$v)) \
  $(eval $v copy_dongle_headers-$v: ARCH         := $(call .arch,$v)) \
  $(eval $v copy_dongle_headers-$v: TYPE         := $(call .type,$v)) \
  $(eval $v copy_dongle_headers-$v: MSG          := $(call .msg,$v)) \
  $(eval $v copy_dongle_headers-$v: BAT          := $(call .bat,$v)) \
  $(eval $v copy_dongle_headers-$v: VARIANT      := $v) \
 )

$(BUILD_VARIANTS):
	@$(MARKSTART)
	@if [ "$(BAT)" -ot "$(BLD_TEMPLATE)" ]; \
	then \
	   echo "INFO: Generating new $(BAT) for $@ target"; \
	   sed \
	     -e "s/%TYPE%/$(TYPE)/g"   \
	     -e "s/%WDKOS%/$(WDKOS)/g" \
	     -e "s/%ARCH%/$(ARCH)/g"   \
	     -e "s/%BLDOUTARCH%/$(BLDOUTARCH)/g"   \
	     -e "s/%ACTION%/$(ACTION)/g"   \
	     -e "s/%WINOS%/$(WINOS)/g"   \
	     -e "s/%MSG%/$(MSG)/g"   \
	     -e "s/%WDK_VER%/$(WDK_VER)/g"   \
	     -e "s!%BASEDIR%!$(subst /,~,$(BASEDIR_UX))!g"   \
	     -e "s!%BLDDIR%!$(subst /,~,$(CWDIR))!g"   \
	     -e "s/%TITLE%/$(TITLE)_$(MSG)/g"   \
	     -e "s!%VCXPROJDIR%!$(COPY_VCXPROJ_CUR_DIR_PROJECTION)!g" \
	  $(BLD_TEMPLATE) | sed -e "s/~/\\\\/g" > $(BAT); \
	else \
	   echo "INFO: Reusing existing $(BAT)"; \
	fi
ifdef COPY_VCXPROJ_TARGET
	$(COPY_VCXPROJ_SCRIPT) --src_root $(COPY_VCXPROJ_SOURCE) --dst_root $(COPY_VCXPROJ_TARGET) --vcxproj win8driver.vcxproj
endif # COPY_VCXPROJ_TARGET
ifndef SKIP_BUILD_SOURCES
#	# WDK build.exe outputs the log as build<winos>/*.log files
#	# Let them show the output on same build window. Needed
#	# in precommit and cont integ shell contexts builds
	@$(DOSCMD) /C "$(BAT) && set BUILD_EC=%ERRORLEVEL% && exit %BUILD_EC%"
	$(call POST_BUILD_VERIFY,$(ARCH),$(WDKOS),$(TYPE))
endif # SKIP_BUILD_SOURCES
	@echo -e "\nFinished with '$@' target at $$(date)\n"
	@rm -f $(BAT)
	@$(MARKEND)


## ---------------------------------------------------------------------------
## In order to speed up build process, generate a wdk build batch file
## template and use it for launching subsequent build variants
## note: wdk build tool exit codes do not indicate some error conditions
## correctly. We need to manually check built objects for correctness!

$(BLD_TEMPLATE):
	@echo -e "@echo off\n\
	@REM Automatically generated on $(shell date). Do not edit\n\n\
	set VARIANT=%TYPE%_%WDKOS%_%BLDOUTARCH%\n\
	set PREFIX=build%VARIANT%\n\
	set WDKDIR=%BASEDIR%\n\
	set PATH=%WDKDIR%\\\\bin\\\\x86;%path%\n\n\
	if NOT EXIST %WDKDIR%\\\\bin\\\\x86\\\\build.exe goto wdkenverror\n\
	echo %WINOS% %MSG% %VARIANT% with %WDK_VER% WDK\n\
	echo Running at %date% %time% on %computername%\n\
	call %WDKDIR%\\\\bin\\\\setenv %WDKDIR% %TYPE% %ARCH% %WDKOS% $(WDK_OACR)\n\
	set MAKEFLAGS=\n\
	cd /D %BLDDIR%\n\
	goto ec%ERRORLEVEL%\n\n\
	:ec0\n\
	if /I NOT \"%cd%\"==\"%BLDDIR%\" goto ec1\n\
	title %TITLE%\n\
	echo Current Dir : %cd%\n\
	echo build -be %WDKFLAGS%\n\
	build -be %WDKFLAGS%\n\
	set buildec=%ERRORLEVEL%\n\
	if EXIST %PREFIX%.log if DEFINED VERBOSE type %PREFIX%.log\n\
	if EXIST %PREFIX%.wrn type %PREFIX%.wrn\n\
	if EXIST %PREFIX%.err type %PREFIX%.err\n\
	if /I NOT \"%buildec%\"==\"0\" goto buildec1\n\
	title DONE_%TITLE%\n\
	goto done\n\n\
	:wdkenverror\n\
	echo ERROR: WDK directory '%WDKDIR%' is not found\n\
	echo  INFO: You may have incompatible generated build scripts\n\
	echo  INFO: You can regenerate them with 'make SKIP_BUILD_SOURCES=1'\n\
	exit /B 1\n\n\
	:buildec1\n\
	echo ERROR: %ACTION% failed with error code in '%BLDDIR%'\n\
	exit /B 1\n\n\
	:ec1\n\
	echo ERROR: Could not change dir to '%BLDDIR%'\n\
	goto done\n\n\
	:buildec0\n\
	:done\n\
	echo Done with %WINOS% %MSG% for %VARIANT%\n" \
	| sed -e 's/^[[:space:]]//g' > $@

## ---------------------------------------------------------------------------
## Use DOS to echo ENV Vars in the raw. No need to escape char's from bash.exe.
## Outer CMD loops from 1 to N, inner echos lines using cmd /v !VAR! feature.
## cygwin ssh strips win variables like %PROGRAMFILES%, add back for precommit.
## Temp must be \temp,  not long path with spaces or VStudio batch build can fail.
## To right of := is the .BAT script written "as shown", no escape chars needed
## ---------------------------------------------------------------------------

export MSLINE_1  := @echo off
export MSLINE_2  := REM Automatically generated on $(shell date). Do not edit
export MSLINE_3  := set VARIANT=%TYPE%_%WDKOS%_%BLDOUTARCH%
export MSLINE_4  := set OUTDIR=%cd%/obj/%WDKOS%/%TYPE%/%ARCH%
ifneq ($(findstring WIN10,$(WINOS)),)
export MSLINE_5  := set VCDIR=$(WIN10_VSDIR)\VC
else ifneq ($(findstring WIN8X,$(WINOS)),)
export MSLINE_5  := set VCDIR=$(WIN8X_VSDIR)\VC
endif
export MSLINE_6  := set TEMP=C:\temp
export MSLINE_7  := set TMP=C:\temp
export MSLINE_8  := if "%ALLUSERSPROFILE%"=="" SET ALLUSERSPROFILE=C:\ProgramData
export MSLINE_9  := if "%CommonProgramFiles%"=="" SET CommonProgramFiles=C:\Program Files\Common Files
export MSLINE_10 := if "%CommonProgramFiles(x86)%"=="" SET CommonProgramFiles(x86)=C:\Program Files (x86)\Common Files
export MSLINE_11 := if "%CommonProgramW6432%"=="" SET CommonProgramW6432=C:\Program Files\Common Files
export MSLINE_12 := if "%ComSpec%"=="" SET ComSpec=C:\Windows\system32\cmd.exe
export MSLINE_13 := if "%DLGTEST_NOTIFY%"=="" SET DLGTEST_NOTIFY=false
export MSLINE_14 := if "%ProgramData%"=="" SET ProgramData=C:\ProgramData
export MSLINE_15 := if "%ProgramFiles%"=="" SET ProgramFiles=C:\Program Files
export MSLINE_16 := if "%ProgramFiles(x86)%"=="" SET ProgramFiles(x86)=C:\Program Files (x86)
export MSLINE_17 := if "%ProgramW6432%"=="" SET ProgramW6432=C:\Program Files
export MSLINE_18 := if "%WINDIR%"=="" SET WINDIR=C:\Windows
export MSLINE_19 := if NOT EXIST %VCDIR%\vcvarsall.bat goto vcenverr
export MSLINE_20 := echo %WINOS% %MSG% %VARIANT% with VS%VS_VER%
export MSLINE_21 := echo Running at %date% %time% on %computername%
export MSLINE_22 := set MAKEFLAGS=
export MSLINE_23 := call %VCDIR%\vcvarsall.bat %ARCH%
export MSLINE_24 := goto ec%ERRORLEVEL%
export MSLINE_25 := :ec0
export MSLINE_26 := title %VARIANT%
export MSLINE_27 := echo Current Dir : %cd% VCDIR: %VSDIR% Path: %PATH%
export MSLINE_28 := which msbuild.exe
ifneq ($(findstring WIN10,$(WINOS)),)
export MSLINE_29 := echo msbuild.exe /property:Configuration='%WDKOS%_%TYPE%' /property:Platform=%BLDOUTARCH% /property:OutDir=%OUTDIR%/ /property:WDKContentRoot=$(WIN10_WDKDIR)/ /property:GenerateManifest=false %VSFLAGS% %VCXPROJDIR%wdidriver.vcxproj
export MSLINE_30 := msbuild.exe /property:Configuration="%WDKOS%_%TYPE%" /property:Platform=%BLDOUTARCH% /property:OutDir=%OUTDIR%/ /property:WDKContentRoot=$(WIN10_WDKDIR)/ /property:GenerateManifest=false %VSFLAGS% %VCXPROJDIR%wdidriver.vcxproj
else ifneq ($(findstring WIN8X,$(WINOS)),)
export MSLINE_29 := echo msbuild.exe /property:Configuration='%WDKOS%_%TYPE%' /property:Platform=%BLDOUTARCH% /property:OutDir=%OUTDIR%/ /property:WDKContentRoot=$(WIN8X_WDKDIR)/ /property:GenerateManifest=false %VSFLAGS% %VCXPROJDIR%win8driver.vcxproj
export MSLINE_30 := msbuild.exe /property:Configuration="%WDKOS%_%TYPE%" /property:Platform=%BLDOUTARCH% /property:OutDir=%OUTDIR%/ /property:WDKContentRoot=$(WIN8X_WDKDIR)/ /property:GenerateManifest=false %VSFLAGS% %VCXPROJDIR%win8driver.vcxproj
endif
export MSLINE_31 := set buildec=%ERRORLEVEL%
export MSLINE_32 := if /I NOT "%buildec%"=="0" goto buildec1
export MSLINE_33 := title DONE_%VARIANT%
export MSLINE_34 := goto done
export MSLINE_35 := :vcenverr
export MSLINE_36 := echo ERROR: VS directory '%VCDIR%' is not found
export MSLINE_37 := exit /B 1
export MSLINE_38 := :buildec1
export MSLINE_39 := echo ERROR: %ACTION% failed with error code 1
export MSLINE_40 := exit /B 1
export MSLINE_41 := :buildec0
export MSLINE_42 := :done
export MSLINE_43 := echo Done with %WINOS% %MSG% for %VARIANT%
MSLINE_COUNT = 43

## Echo each raw env var in order creating .BAT script
MSBLD_CMD := FOR /L %i IN (1,1,$(MSLINE_COUNT)) DO @cmd /v /c echo !MSLINE_%i!

$(MSBLD_TEMPLATE):
	@rm -fv $@
	@cmd /c '$(MSBLD_CMD)' >> $@

# Force .temp*.BAT to refresh every time makefile is invoked
.PHONY: $(MSBLD_TEMPLATE)

## ---------------------------------------------------------------------------
## Release sign drivers - needs certificate to be installed on local machine
## as documented in HowToCompileThings twiki

ifdef SIGN_DIR

# If driver signing fails for any reason during timestamping record log by curl utility
# And keep trying for 10mins to connect to verisign timeserver
define SIGN_AND_TIMESTAMP
	@echo "#- $0($1)"
	@cd $(SIGN_DIR) && \
	rm -f $(SIGN_LOG); \
	if [ ! -s "$(MS_CROSSCERT)" ]; then \
		echo "ERROR: Microsoft CrossCertificate MS_CROSSCERT missing"; \
		echo "ERROR: It needs to be available at $(MS_CROSSCERT)"; \
		exit 1; \
	fi; \
	echo "$(RETRYCMD) signtool sign $(SIGNTOOL_OPTS) /ac $(MS_CROSSCERT) $(1)"; \
	$(RETRYCMD) signtool sign $(SIGNTOOL_OPTS) /ac $(MS_CROSSCERT) $(1); \
	echo "DONE WITH SIGN_AND_TIMESTAMP"
endef # SIGN_AND_TIMESTAMP

ifneq ($(findstring WIN10,$(WINOS)),)
release_sign_driver  sign_driver \
release_sign_sysfile sign_sysfile: PATH := $(shell cygpath -u Z:/projects/hnd/tools/win/msdev/$(WIN10_WDK_VER)wdksdk/bin/$(SIGN_ARCH)):$(shell cygpath -u Z:/projects/hnd/tools/win/msdev/$(WIN10_WDK_VER)wdksdk/bin/$(SIGN_ARCH)):$(shell cygpath -u C:/tools/msdev/$(WIN10_WDK_VER)wdksdk/bin/$(SIGN_ARCH)):$(shell cygpath -u C:/tools/msdev/$(WIN10_WDK_VER)wdksdk/bin/x86):$(PATH)
else ifneq ($(findstring WIN8X,$(WINOS)),)
release_sign_driver  sign_driver \
release_sign_sysfile sign_sysfile: PATH := $(shell cygpath -u Z:/projects/hnd/tools/win/msdev/$(WIN8_WDK_VER)wdksdk/bin/$(SIGN_ARCH)):$(shell cygpath -u Z:/projects/hnd/tools/win/msdev/$(WIN8_WDK_VER)wdksdk/bin/$(SIGN_ARCH)):$(shell cygpath -u C:/tools/msdev/$(WIN8_WDK_VER)wdksdk/bin/$(SIGN_ARCH)):$(shell cygpath -u C:/tools/msdev/$(WIN8_WDK_VER)wdksdk/bin/x86):$(PATH)
else # !WIN8X
release_sign_driver  sign_driver \
release_sign_sysfile sign_sysfile: PATH := $(shell cygpath -u $(subst \,/,$(BASEDIR))/bin/SelfSign):$(shell cygpath -u Z:/projects/hnd/tools/win/msdev/$(WDK_VER)wdk/bin/SelfSign):$(WDKDIR)/bin/SelfSign:$(PATH)
endif # !WIN8X

release_sign_driver sign_driver:
	@echo "INFO: Sign tools from $(WDKDIR) will be used"
	@if [ ! -f "$(SIGN_DIR)/$(SIGN_DRIVER)" ]; then \
	    echo "ERROR:"; \
	    echo "ERROR: $(SIGN_DIR)/$(SIGN_DRIVER) not found to sign"; \
	    echo "ERROR: Does the $(SIGN_DRIVER) exist?"; \
	    echo "ERROR: Verify WINOS or SIGN_ARCH values provided"; \
	    echo "ERROR:"; \
	    exit 1; \
	fi
	@echo -e "\nSigning $(SIGN_ARCH) $(SIGN_OS) drivers from: $(SIGN_DIR)\n"
	@$(SRCBASE)/tools/release/wustamp -o -r -v "$(RELNUM)" -d "$(RELDATE)" "`cygpath -w $(SIGN_DIR)`"
	@echo -e "SIGN_AND_TIMESTAMP $(SIGN_DRIVER) \n"
	$(call SIGN_AND_TIMESTAMP,$(SIGN_DRIVER))
	which Inf2Cat.exe
	cd $(SIGN_DIR) && Inf2cat /driver:. /os:$(SIGN_OS)_$(SIGN_ARCH)
	@echo -e "SIGN_AND_TIMESTAMP $(SIGN_DRIVERCAT) \n"
	$(call SIGN_AND_TIMESTAMP,$(SIGN_DRIVERCAT))

release_sign_sysfile sign_sysfile:
	@echo "INFO: Sign tools from $(WDKDIR) will be used"
	@if [ ! -f "$(SIGN_DIR)/$(SIGN_DRIVER)" ]; then \
	    echo "ERROR:"; \
	    echo "ERROR: $(SIGN_DIR)/$(SIGN_DRIVER) not found to sign"; \
	    echo "ERROR: Does the $(SIGN_DRIVER) exist?"; \
	    echo "ERROR: Verify WINOS or SIGN_ARCH values provided"; \
	    echo "ERROR:"; \
	    exit 1; \
	fi
	@echo -e "\nSigning $(SIGN_ARCH) $(SIGN_OS) drivers from: $(SIGN_DIR)\n"
	@$(SRCBASE)/tools/release/wustamp -o -r -v "$(RELNUM)" -d "$(RELDATE)" "`cygpath -w $(SIGN_DIR)`"
	$(call SIGN_AND_TIMESTAMP,$(SIGN_DRIVER))
	cd $(SIGN_DIR) && $(RETRYCMD) signtool verify /kp $(SIGN_DRIVER)

gen_cat_driver: PATH := /usr/bin:/bin:$(shell cygpath -u $(subst \,/,$(BASEDIR))/bin/SelfSign):$(PATH)
gen_cat_driver: SIGN_OS := XP2K
gen_cat_driver:
	@if [ ! -f "$(SIGN_DIR)/$(SIGN_DRIVER)" ]; then \
	    echo "ERROR:"; \
	    echo "ERROR: $(SIGN_DIR)/$(SIGN_DRIVER) not found to sign"; \
	    echo "ERROR: Does the $(SIGN_DRIVER) exist?"; \
	    echo "ERROR: Verify WINOS or SIGN_ARCH values provided"; \
	    echo "ERROR:"; \
	    exit 1; \
	fi
	@echo -e "\nSigning $(SIGN_ARCH) $(SIGN_OS) drivers from: $(SIGN_DIR)\n"
	@$(SRCBASE)/tools/release/wustamp -o -r -v "$(RELNUM)" -d "$(RELDATE)" "`cygpath -w $(SIGN_DIR)`"
	cd $(SIGN_DIR) && Inf2cat /driver:. /os:$(subst $(space),$(comma),$(SIGN_ARCH))
endif # SIGN_DIR

clean:
	rm -rf $(XPBLDDIR)/src/include/{epivers,wlconf}.h $(XPBLDDIR)/obj*_w*
	rm -rf $(WIN7BLDDIR)/src/include/{epivers,wlconf}.h $(WIN7BLDDIR)/obj*_w*
	rm -rf obj/*.sys
	rm -f  $(BLD_TEMPLATE) $(MSBLD_TEMPLATE)
	rm -f build_ch*.bat build_fre*.bat

clean_all:
	rm -f build_ch*.bat build_fre*.bat
	rm -f $(BLD_TEMPLATE) $(MSBLD_TEMPLATE)
	rm -rf buildxp* buildwin7* obj/*
	rm -rf buildwin8x/obj

PHONY: all copy_sources copy_includes gen_sources prep_sources build_sources	\
	build_driver		build_xp_driver		build_win7_driver	\
build_win8x_driver		build_win10_driver	\
				clean_all clean $(BUILD_VARIANTS) release_sign_driver sign_driver help

# Display a list of available targets
help:
	@echo -e "\n\
To build xp only use:       'make build_xp_driver' \n\
To build win7 driver: 'make build_win7_driver' \n\
To build noccx win7 only:  'make BCMCCX=0 build_win7_driver' \n\
To build win8x driver:      'make build_win8x_driver' \n\
To get xp sources only:     'make BRAND=1 WINOS=WINXP gen_sources' \n\
To get win7 sources:		'make BRAND=1 WINOS=WIN7 gen_sources'\n\
To use WDK ver NNNN:        'make WDK_VER=NNNN' \n\
To use OLD DDK:             'make USEDDK=1' \n\
To clean built objects:     'make clean' \n\
To clean generated files:   'make clean_all' \n\
To track dependencies:      'make WDKFLAGS=\'\'' \n\
To regenerate buildscripts: 'make SKIP_BUILD_SOURCES=1' \n\
\n\
For more specific build tasks: \n\n\
To build xp x86 checked: \n\
 'make BUILD_TYPES=checked BUILD_ARCHS=x86 build_xp_driver' \n\n\
To build win7 x86 checked without ccx: \n\
 'make BCMCCX=0 BUILD_TYPES=checked BUILD_ARCHS=x86 build_win7_driver' \n\n\
To build win7 x86 checked: \n\
 'make BUILD_TYPES=checked BUILD_ARCHS=x86 build_win7_driver' \n\n\
To build win7 x86 checked with Microsoft post-build Code Review enabled: \n\
 'make BUILD_TYPES=checked BUILD_ARCHS=x86 WDK_OACR=oacr build_win7_driver' \n\n\
To build win8x x86 checked: \n\
 'make BUILD_TYPES=checked BUILD_ARCHS=x86 build_win8x_driver' \n\n\
To build win8x x64 free: \n\
 'make BUILD_TYPES=free BUILD_ARCHS=x64 build_win8x_driver' \n\n\
To sign built driver: \n\n\
[NOTE: Copy non driver files (inf,dll etc.,) from nightly build for signing] \n\
To sign xp 64bit driver:    \n\
 'make WINOS=WINXP SIGN_DIR=buildxp/objchk_wnet_amd64/amd64 sign_driver' \n\n\
For further details refer to HowToCompileThings twiki \n\
"

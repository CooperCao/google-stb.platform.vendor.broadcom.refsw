#
# Build the windows wireless drivers, app, tools, installer and package them
#
# Contact: hnd-software-scm-list
#
# $Id$
#
# HOW THIS MAKEFILE WORKS
#
# The release consists of 5 stages:
#
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
# Besides build target [all], there are two conenient targets [trayapp], [driver]
# to save time for testing build. They SHOULD not be released.

empty:=
space:= $(empty) $(empty)
comma:= $(empty),$(empty)

PARENT_MAKEFILE :=
DEFAULT_TARGET  := default
ALL_TARGET      := all
WLAN_WINPFX     ?= Z:


BUILD_WINALL_KIT=false
BUILD_WITH_WINALL_RELEASE=release_win7 release_winall
TARGET_BUILD_LIST=release_win7 release_winall



$(DEFAULT_TARGET): $(ALL_TARGET)

DATE                = $(shell date -I)
BUILD_BASE          = $(shell pwd)
RELEASEDIR          = $(BUILD_BASE)/release
SHELL               = bash.exe
FIND                = /bin/find
INSTALL            := install -p
MAKE_MODE          := unix
NULL               := /dev/null
BRAND              ?=win_external_wl
RETRYCMD       	   ?= $(firstword \
                      $(wildcard \
		       C:/tools/build/bcmretrycmd.exe \
                       $(WLAN_WINPFX)/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                   ))

export MOGRIFY_RULES     = mogrify_common_rules.mk
export BRAND_RULES       = brand_common_rules.mk

ifeq ($(origin VERISIGN_ROOT), undefined)
    VERISIGN_ROOT =  z:/projects/hnd/tools/win/verisign
endif
CABARC         := $(VERISIGN_ROOT)/cabarc.exe

OVFILE              = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
export CCX_SDK_VER := 1.1.13
CCX_SDK_DIR        ?= $(WLAN_WINPFX)/projects/hnd/restrictedTools/CCX_SDK/$(CCX_SDK_VER)
CCX_MUI_DIR        ?= $(WLAN_WINPFX)/projects/hnd/software/work/HelpFiles/vista/Cisco_Plug-Ins/$(CCX_SDK_VER)
X86_WPCAP41_DIR	   := src/8021x/win32/bin/wpcap_4_1
X64_WPCAP41_DIR	   := src/8021x/win64/bin/wpcap_4_1
export WDK_VER     := 7600
WDK_DIR            := $(firstword $(wildcard c:/tools/msdev/$(WDK_VER)wdk d:/tools/msdev/$(WDK_VER)wdk $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(WDK_VER)wdk))
CHKINF             := $(WDK_DIR)/tools/Chkinf/chkinf.bat
# Preserve intermediate files to debug any build issues
# export ECADDIN_DONT_RM_TMP_MAKEFILES=1

# By default both driver and applications (like trayapp) are built and
# packaged. On certain branches (media), trayapp may not build as it is
# not supported or due to incompatible dependent components. Media
# occasionally needs only driver package. So following provides a
# flag that can be set to true or false. Default is true (to build tray)
BUILD_TRAYAPP ?= false

WinXP_TRAYAPP ?= false

SIGN_OS             = $(if $(findstring WinXP,$@),WINXP,$(if $(findstring WinVista,$@),WINVISTA,$(if $(findstring Win7,$@),WIN7,UNKNOWN)))

# WinXP wl drivers (monolithic)
XP_WDK_BUILDDIR     = buildxp
XP_WDK_OS_x86       = wxp
XP_WDK_OS_x64       = wnet
XP_x86_WLDRIVER     = src/wl/sys/wdm/$(XP_WDK_BUILDDIR)/objfre_$(XP_WDK_OS_x86)_x86/i386/bcmwl5.sys
XP_x64_WLDRIVER     = src/wl/sys/wdm/$(XP_WDK_BUILDDIR)/objfre_$(XP_WDK_OS_x64)_amd64/amd64/bcmwl564.sys
XP_WLINF            = bcmwl5.inf
XP_WLINF_COI        = bcmwlcoi5.inf
X86_WLCAT           = bcm43xx.cat
X64_WLCAT           = bcm43xx64.cat

# WinVista and Win7 wl drivers (monolithic)
VISTA_WDK_BUILDDIR  = $(if $(findstring win7,$@)$(findstring Win7,$@)$(findstring WIN7,$@),buildwin7,buildvista)
VISTA_WDK_OS        = $(if $(findstring win7,$@)$(findstring Win7,$@)$(findstring WIN7,$@),win7,wlh)
VISTA_x86_WLDRIVER  = src/wl/sys/wdm/$(VISTA_WDK_BUILDDIR)/objfre_$(VISTA_WDK_OS)_x86/i386/bcmwl6.sys
VISTA_x64_WLDRIVER  = src/wl/sys/wdm/$(VISTA_WDK_BUILDDIR)/objfre_$(VISTA_WDK_OS)_amd64/amd64/bcmwl664.sys
VISTA_WLINF         = bcmwl6.inf

# NDIS virtual wireless driver
VIRTUAL_x86_DRIVER := components/ndis/ndisvwl/sys/buildxp/objfre_wxp_x86/i386/bcmvwl32.sys
VIRTUAL_x64_DRIVER := components/ndis/ndisvwl/sys/buildxp/objfre_wnet_amd64/amd64/bcmvwl64.sys

# WinVista and Win7 wdf dlls
VISTA_x86_WDFDLL   := $(WDK_DIR)/redist/wdf/x86/WdfCoInstaller01009.dll
VISTA_x64_WDFDLL   := $(WDK_DIR)/redist/wdf/amd64/WdfCoInstaller01009.dll

# WinVista and Win7 protocol drivers
VISTA_x86_PROTOCOLDRIVER := src/epiprot/sys/objfre_wlh_x86/i386/bcm42rly.sys
VISTA_x64_PROTOCOLDRIVER := src/epiprot/sys/objfre_wlh_amd64/amd64/bcm42rly.sys

# NDIS 5.0 or 5.1, Checked or Free
NDIS50_F           := ndis50/free/bcmwl5.sys

# NPF files
XP_x86_NPF         := bcmwlnpf.sys
XP_x64_NPF         := Bcmnpf64.sys

# App brands and variants to build and package
ifeq ($(BUILD_TRAYAPP),true)
  OEM_LIST                   ?=  bcm
  PROJECT_CONFIGS_GENERIC    := 'Release|win32'   'Release|x64'
  PROJECT_CONFIGS_VISTA      := 'Releasev|win32'  'Releasev|x64'
  PROJECT_CONFIGS_UNICODE    := 'ReleaseUv|win32' 'ReleaseUv|x64'
  PROJECT_CONFIGS_XP_APP     := $(PROJECT_CONFIGS_GENERIC)
  PROJECT_CONFIGS_VISTA_TTLS := $(PROJECT_CONFIGS_GENERIC)
  PROJECT_CONFIGS_VISTA_IHV  := $(PROJECT_CONFIGS_GENERIC)
  PROJECT_CONFIGS_VISTA_APP  := $(PROJECT_CONFIGS_VISTA)
  PROJECT_CONFIGS_VISTA_APP  += $(PROJECT_CONFIGS_UNICODE)
  OEM_LIST_XP_RLSPKG         ?=  Bcm
  OEM_LIST_VISTA_RLSPKG      ?=
  OEM_LIST_WIN7_RLSPKG       ?=  Bcm
  OEM_LIST_WINALL_RLSPKG     ?=  Bcm
  OEM_LIST_PE_RLSPKG         ?=  Bcm
else
  OEM_LIST                   ?=  bcm
  PROJECT_CONFIGS_GENERIC    := 'Release|win32'
  PROJECT_CONFIGS_GENERIC_ALL:= 'Release|win32' 'Release|x64'
  PROJECT_CONFIGS_VISTA      := 'Releasev|win32'
  PROJECT_CONFIGS_UNICODE    := 'ReleaseUv|win32'
  PROJECT_CONFIGS_XP_APP     := $(PROJECT_CONFIGS_GENERIC)
  PROJECT_CONFIGS_VISTA_TTLS := $(PROJECT_CONFIGS_GENERIC)
  PROJECT_CONFIGS_VISTA_IHV  := $(PROJECT_CONFIGS_GENERIC_ALL)
  PROJECT_CONFIGS_VISTA_APP  := $(PROJECT_CONFIGS_VISTA)
  PROJECT_CONFIGS_VISTA_APP  += $(PROJECT_CONFIGS_UNICODE)
  ifneq ($(BCM_MFGTEST),)
    OEM_LIST_XP_RLSPKG       ?=  Bcm Nokia
  else  # BCM_MFGTEST
    OEM_LIST_XP_RLSPKG       ?=  Bcm
  endif # BCM_MFGTEST
  OEM_LIST_VISTA_RLSPKG      ?=
  OEM_LIST_WIN7_RLSPKG       ?=  Bcm
 #OEM_LIST_WINALL_RLSPKG     ?=  Bcm
  OEM_LIST_PE_RLSPKG         ?=  Bcm
endif # BUILD_TRAYAPP
export OEM_LIST

ifeq ($(BUILD_WINALL_KIT),false)
	TARGET_BUILD_LIST := $(filter-out release_winall,$(BUILD_WITH_WINALL_RELEASE))
endif

# Restrict dotfuscation only to release builds
override DOTFUSCATE_APP := false
DOTFUSCATOR_DIR     ?= components/apps/windows/tray/winx/Obfuscator/Dotfuscator
# dotfuscator.exe cmd line options can be passed via DOTFUSCATE_OPTS
DOTFUSCATOR_OPTS    :=
export TRAY_POST_BUILD_DISABLED=1
export DOTFUSCATE_APP
export DOTFUSCATE_OPTS

# Following pre_release dependencies are built differently
PRE_RELEASE_DEPS_COMMON  := pre_release

PRE_RELEASE_DEPS_XP      := $(PRE_RELEASE_DEPS_COMMON)
PRE_RELEASE_DEPS_XP      += build_install

PRE_RELEASE_DEPS_VISTA   := $(PRE_RELEASE_DEPS_COMMON)
PRE_RELEASE_DEPS_VISTA   += build_install
# PRE_RELEASE_DEPS_VISTA   += build_trayapp_vista_dotfuscate

PRE_RELEASE_DEPS         := $(PRE_RELEASE_DEPS_COMMON)
PRE_RELEASE_DEPS         += $(PRE_RELEASE_DEPS_XP)
PRE_RELEASE_DEPS         += $(PRE_RELEASE_DEPS_VISTA)

# From command line, one flag can override packaging pre-requisites
ifeq ($(PRE_RELEASE_DEPS),)
  PRE_RELEASE_DEPS_COMMON  :=
  PRE_RELEASE_DEPS_XP      :=
  PRE_RELEASE_DEPS_VISTA   :=
  PRE_RELEASE_DEPS         :=
endif

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

## This is defined by calling win_mfgtest build
## Exclude BCMSDIO if not mfgtest
ifeq ($(BCM_MFGTEST),)
   BLDTYPE=EXTERNAL
else
   BLDTYPE=MFGTEST
   UNDEFS=
endif

## Pegged (fixed) versions of certain build/release dependent objects.
## If VER_PEGGED is set, that dostool is used to package
## If not set, current $(TAG) (or TOB or TOT) dostool
## binaries are built and then later used in release packaging.

#  NOTE: dostool version is fixed (no rebuild needed)
   DOSTOOL_VER_PEGGED = BUILD_DELL_DOSTOOL

BCMWLHLP_DIR               := $(WLAN_WINPFX)/projects/hnd/software/work/HelpFiles
BCMWLHLP_REV_xp_bcm        := 2010.10.15
BCMWLHLP_REV_xp_dell       := 2010.10.15
BCMWLHLP_REV_xp_dellsm     := 2010.10.15
BCMWLHLP_REV_xp_hp         := 2010.10.15
BCMWLHLP_REV_vista_bcm     := 2010.10.15
BCMWLHLP_REV_vista_dell    := 2010.10.15
BCMWLHLP_REV_vista_dellsm  := 2010.10.15
BCMWLHLP_REV_vista_hp      := 2010.10.15

ifeq ($(BUILD_TRAYAPP),true)

  dummy                = $(shell mkdir -p $(BUILD_LINUX_TEMP_WIN))

  ## DOSTOOL_VER; If DOSTOOL_VER_PEGGED defined use it
  DOSTOOL_VER         := $(if $(DOSTOOL_VER_PEGGED),$(DOSTOOL_VER_PEGGED),$(if $(TAG),$(TAG),"NIGHTLY"))
  DOSTOOL_WORK_DIR     = $(shell ls -1trd $(BUILD_LINUX_TEMP_WIN)/$(DOSTOOL_VER)/dostool/$(if $(TAG),,$(if $(DOSTOOL_VER_PEGGED),,$(TODAY).))* 2> $(NULL) | grep -v current | tail -1)
  DOSTOOL_SERVER_DIR   = $(shell ls -1trd $(BUILD_LINUX_DIR_WIN)/$(DOSTOOL_VER)/dostool/$(if $(TAG),,$(if $(DOSTOOL_VER_PEGGED),,$(TODAY).))* 2> $(NULL) | grep -v current | tail -1)
  DOSTOOL_DIR          = $(shell if [ -d "$(DOSTOOL_WORK_DIR)" ]; then echo $(DOSTOOL_WORK_DIR); else echo $(DOSTOOL_SERVER_DIR); fi)

endif

# Release package folders whose build status need to be marked as pass/fail
# in build_status.txt file
# NOTE: Some of the directory names are only specific to certain OEM brands
RLSPKG_DIRS_WIN7    := $(foreach oem,$(OEM_LIST_WIN7_RLSPKG),\
			   release/Win7/$(oem)/$(oem)_InstallShield \
			   release/Win7/$(oem)/$(oem)_InstallShield_Driver \
			   release/Win7/$(oem)/$(oem)_DriverOnly \
			   release/Win7/$(oem)/$(oem)_DriverOnly_CAB \
			)

RLSPKG_DIRS_XP      := $(foreach oem,$(OEM_LIST_XP_RLSPKG),\
			   release/WinXP/$(oem)/$(oem)_InstallShield \
			   release/WinXP/$(oem)/$(oem)_InstallShield_Driver \
			   release/WinXP/$(oem)/$(oem)_DriverOnly \
			   release/WinXP/$(oem)/$(oem)_DriverOnly_CAB \
			)

RLSPKG_DIRS_WINALL   := $(foreach oem,$(OEM_LIST_WINALL_RLSPKG),\
			   $(foreach os,WinXP Win7 WinVista,\
			     release/WinALL/$(oem)/$(oem)_InstallShield/Disk1/$(os) \
			     release/WinALL/$(oem)/$(oem)_InstallShield_Driver/Disk1/$(os) \
			     release/WinALL/$(oem)/$(oem)_DriverOnly/$(os) \
			  ) \
			)

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
UNDEFS+= CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ VX_BSD4_3 ROUTER BCMENET BCM4710A0 CONFIG_BCM4710 CONFIG_MIPS_BRCM POCKET_PC BCMINTERNAL DSLCPE LINUXSIM BCMSIM BCMSIMUNIFIED WLNINTENDO WLNINTENDO2

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS=BCM47XX BCM47XX_CHOPS

# this is a special addition of BCMDBG to force ASSERTS to be enabled
# in our code, regardless of whether we are compiling for debug or not.
# DEFS += BCMDBG

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

SIGNTOOL_TIME  := "http://timestamp.verisign.com/scripts/timstamp.dll"
SIGNTOOL_OPTS  ?= /a /v /s my /n "Broadcom Corporation" /t $(SIGNTOOL_TIME)
SIGNTOOL_CMND_32  := $(RETRYCMD) $(WDK_DIR)/bin/x86/signtool.exe sign $(SIGNTOOL_OPTS)
SIGNTOOL_CMND_64  := $(RETRYCMD) $(firstword $(wildcard $(WDK_DIR)/bin/amd64/signtool.exe $(WDK_DIR)/bin/x64/signtool.exe)) sign $(SIGNTOOL_OPTS)
# 7600 has 64bit signtool in amd64 dir, later WDk's use x64 dir.
export SIGNTOOL_OPTS

# Default installshield is used for all modules is still IS 12, but main
# installer upgraded to IS 2009
ifeq ($(origin ISSAB_ROOT), undefined)
    ISSAB_ROOT = $(firstword $(wildcard c:/tools/InstallShield12SAB d:/tools/InstallShield12SAB $(WLAN_WINPFX)/projects/hnd/tools/win/InstallShield12SAB))
    ISSAB12_ROOT = $(ISSAB_ROOT)
endif
ifeq ($(origin ISSAB2009_ROOT), undefined)
    ISSAB2009_ROOT = $(firstword $(wildcard c:/tools/InstallShield2009SAB d:/tools/InstallShield2009SAB $(WLAN_WINPFX)/projects/hnd/tools/win/InstallShield2009SAB))
endif
export IS12_ROOT  := $(ISSAB_ROOT)
export IS2009_ROOT:= $(ISSAB2009_ROOT)
TREECMD        := $(firstword $(wildcard c:/tools/utils/tree.exe d:/tools/utils/tree.exe $(WLAN_WINPFX)/projects/hnd/tools/win/utils/tree.exe)) -a -s -D -f
INS_DIR        := components/apps/windows/install/app
INF_DIR        := $(INS_DIR)/installshield/BCMAppInstaller/is_bin
BIN_DIR        := components/shared/resources/tools/bin
WARN_FILE      := _WARNING_PARTIAL_BUILD_DO_NOT_USE

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $@/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $@/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# Run check inf tool
define CHECK_INF
	@echo "#- $0"
	cd $1; \
	perl $(BUILD_BASE)/src/tools/inftools/check_inf.pl \
		-inf $2 -chkinf $(CHKINF)
endef  # CHECK_INF

# Copy and timestamp inf files
define INSTALL_INF
	@echo "#- $0"
	$(INSTALL) $1 $2
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d $(RELDATE) "`cygpath -w $2`"
endef  # INSTALL_INF

# VS05 Redistributables for tray/gui xp+vista app
define INSTALL_VS0532BIT_VCREDIST
	@echo "#- $0"
	$(INSTALL) $(BIN_DIR)/vs05/$1/vcredist_x86.exe  $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/vcredist_x86.bat  $2
endef  # INSTALL_VS0532BIT_VCREDIST

define INSTALL_VS0564BIT_VCREDIST
	@echo "#- $0"
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/vcredist_x64.exe  $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/vcredist_x64.bat  $2
endef  # INSTALL_VS0564BIT_VCREDIST

# VS08 Redistributables for tray/gui xp+vista app
define INSTALL_VS0832BIT_VCREDIST
	@echo "#- $0"
	$(INSTALL) -d  $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/vcredist_x86.exe  $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/vcredist_x86.bat  $2
endef  # INSTALL_VS0832BIT_VCREDIST

define INSTALL_VS0864BIT_VCREDIST
	@echo "#- $0"
	$(INSTALL) -d  $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/vcredist_x64.exe  $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/vcredist_x64.bat  $2
endef  # INSTALL_VS0864BIT_VCREDIST

# Dependent library for xp tray/gui
define INSTALL_VENDORLIBS_DLLS
	@echo "#- $0"
	$(INSTALL) src/vendorlibs/BCGSoft/BCGControlBarPro/v731/libs/VS2005/$1 $2
endef  # INSTALL_VENDORLIBS_DLLS

define INSTALL_BRCM_SUPPLICANT
	@echo "#- $0"
	$(INSTALL) src/8021x/win32/bin/$(XP_x86_NPF)              $2
	$(INSTALL) src/8021x/win32/bin/bcmwlpkt.dll               $2
	$(INSTALL) src/8021x/win64/bin/$(XP_x64_NPF)              $2
	-$(INSTALL) src/8021x/xsupplicant/bcm1xsup/$1             $2
endef # INSTALL_BRCM_SUPPLICANT

# WinXP/2K List only common symbol files (across all brands) here
define INSTALL_XP_APP_DEBUG_SYMBOLS
	@echo "#- $0"
	$(INSTALL) src/doc/READMESymbols.txt $@/README.txt
	$(INSTALL) src/wl/cpl/TrayApp/Release/$1/bcmwltry.pdb $@/
	$(INSTALL) src/wl/cpl/wltray/Release/$1/wltray.pdb $@/
	$(INSTALL) src/wl/cpl/preflib/Release/preflib.pdb $@/
endef # INSTALL_XP_APP_DEBUG_SYMBOLS

# WinVista and Win7 symbol files
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_VISTA_APP_DEBUG_SYMBOLS
	@echo "#- $0"
	$(INSTALL) src/doc/READMESymbols.txt                     				  $@/README.txt
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/$1/bcmwltry.pdb    $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/$1/wltray.pdb      $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/$1/bcmwlcpl.pdb $@/$2
	$(INSTALL) components/apps/windows/tray/winx/Remoting/$1/bcmwlrmt.pdb     $@/$2
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/$1/WLTryNT.pdb     $@/$2
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/$1/wltrysvc.pdb   $@/$2
	$(INSTALL) components/apps/windows/tray/winx/Peernet/$1/bcmpeerapi.pdb    $@/$2
endef # INSTALL_VISTA_APP_DEBUG_SYMBOLS

# WinXP original built files
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_XP32BIT_APP_ORIGINAL_OBJECTS
	@echo "#- $0"
	$(INSTALL) src/wl/cpl/TrayApp/$1/bcmwltry.exe              $@/$2
	$(INSTALL) src/components/BCMLogon/$1/BCMLogon.dll         $@/$2
	$(INSTALL) src/wl/cpl/preflib/$(dir $1)preflib.dll         $@/$2
	$(INSTALL) src/wl/cpl/wltray/$1/wltray.exe                 $@/$2
	$(INSTALL) src/wl/cpl/neptune/cpl/$1/bcmwlcpl.cpl          $@/$2
	$(INSTALL) src/wl/cpl/wltrynt/$1/wltrynt.dll               $@/$2
	$(INSTALL) src/wl/cpl/wltrysvc/$1/wltrysvc.exe             $@/$2
	$(INSTALL) src/wl/cpl/lib/WlAdapter/DLL/$(dir $1)bcmwlapi.dll $@/$2
endef # INSTALL_XP32BIT_APP_ORIGINAL_OBJECTS

# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_XP64BIT_APP_ORIGINAL_OBJECTS
	@echo "#- $0"
	$(INSTALL) src/components/BCMLogon/$1/BCMLogon.dll         $@/$2
	$(INSTALL) src/wl/cpl/preflib/$(dir $1)preflib.dll         $@/$2
	$(INSTALL) src/wl/cpl/neptune/cpl/$1/bcmwlcpl.cpl          $@/$2
	$(INSTALL) src/wl/cpl/lib/WlAdapter/DLL/$(dir $1)bcmwlapi.dll $@/$2
endef # INSTALL_XP64BIT_APP_ORIGINAL_OBJECTS

define INSTALL_XP32BIT_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_XP32BIT_APP_ORIGINAL_OBJECTS,$1,$2)
endef
define INSTALL_XP64BIT_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_XP64BIT_APP_ORIGINAL_OBJECTS,$1,$2)
endef

# WinVista and Win7 original built files (non-dotfuscated)
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_VISTA_APP_ORIGINAL_OBJECTS
	@echo "#- $0"
	$(INSTALL) components/apps/windows/tray/winx/Remoting/$1/bcmwlrmt.dll        		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/Peernet/$1/bcmpeerapi.dll       		$@/$2
	$(INSTALL) components/apps/windows/tray/shared/BCMLogon/BCMLogon/$1/BCMLogon.dll    $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/$1/bcmwlcpl.cpl    		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/$1/bcmwltry.exe       		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/$1/wltray.exe         		$@/$2
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/$(dir $1)bcmwlapiu.dll 	$@/$2
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/$(dir $(subst ReleaseUv,Releasev,$1))bcmwlapi.dll \
								    $@/$2
endef # INSTALL_VISTA_APP_ORIGINAL_OBJECTS

# WinVista and Win7 dotfuscated files
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_VISTA_APP_DOTFUSCATED_OBJECTS
	@echo "#- $0"
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwlrmt.dll     $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmpeerapi.dll   $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/BCMLogon.dll     $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwlcpl.cpl     $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwltry.exe     $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/wltray.exe       $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwlapiu.dll $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst ReleaseUv,Releasev,$(subst /,_,$1))/bcmwlapi.dll \
								      $@/$2
endef # INSTALL_VISTA_APP_DOTFUSCATED_OBJECTS

ifeq ($(strip $(DOTFUSCATE_APP)),true)
define INSTALL_VISTA_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_VISTA_APP_DOTFUSCATED_OBJECTS,$1,$2)
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/$1/wltrysvc.exe       $@/$2
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/$1/WLTryNT.dll         $@/$2
endef
else # NO DOTFUSCATE_APP
define INSTALL_VISTA_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_VISTA_APP_ORIGINAL_OBJECTS,$1,$2)
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/$1/wltrysvc.exe       $@/$2
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/$1/WLTryNT.dll         $@/$2
endef
endif # DOTFUSCATE_APP

define INSTALL_VISTA_PEERNET_LIB
	@echo "#- $0"
	$(INSTALL) components/apps/windows/tray/winx/Peernet/$1/bcmpeerapi.lib      $@/$2
endef # INSTALL_VISTA_PEERNET_LIB

define INSTALL_VISTA_PEERTEST_BIN
	@echo "#- $0"
	$(INSTALL) components/apps/windows/tray/winx/Peernet/Sample/PeerTest/$1/Peertest.exe \
								   $@/$2
endef # INSTALL_VISTA_PEERTEST_BIN

define INSTALL_VISTA_MAESTROEXT_BIN
	@echo "#- $0"
	$(INSTALL) components/apps/windows/tray/winx/Peernet/Sample/MaestroExtTest/$1/MaestroExtTest.exe \
								   $@/$2
endef # INSTALL_VISTA_MAESTROEXT_BIN

define INSTALL_VISTA_BUILT_TTLS
	@echo "#- $0"
	$(INSTALL) components/apps/windows/ihvfrm/eaphost/ttls/$1/bcmttls.dll $@/$2
endef # INSTALL_VISTA_BUILT_TTLS

# IHV files have to reside next to .inf and .sys files
define INSTALL_VISTA32BIT_BUILT_IHV
	@echo "#- $0"
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvui.dll    $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvsrv.dll   $@/$2
endef # INSTALL_VISTA32BIT_BUILT_IHV

# IHV files have to reside next to .inf and .sys files
define INSTALL_VISTA64BIT_BUILT_IHV
	@echo "#- $0"
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvui64.dll  $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvsrv64.dll $@/$2
endef # INSTALL_VISTA64BIT_BUILT_IHV

# IHV files have to reside next to .inf and .sys files
define INSTALL_VISTA_REDIST_IHV
	@echo "#- $0"
	mkdir -pv $@/EAP_Plugin_Installer
#disabled#	$(INSTALL) src/wps/win32/wps_enr/WinPcap_4_0_2.exe  $@/EAP_Plugin_Installer
	$(INSTALL) $(BIN_DIR)/ihv/free/x86/Inst_EAPModules.bat      $@/EAP_Plugin_Installer
	$(INSTALL) $(BIN_DIR)/ihv/free/x86/Uninst_EAPModules.bat    $@/EAP_Plugin_Installer
	$(INSTALL) $(CCX_SDK_DIR)/EAP/EAP-Fast/EAP-FAST.msi         $@/EAP_Plugin_Installer
	$(INSTALL) $(CCX_SDK_DIR)/EAP/LEAP/EAP-LEAP.msi             $@/EAP_Plugin_Installer
	$(INSTALL) $(CCX_SDK_DIR)/EAP/PEAP/EAP-PEAP.msi             $@/EAP_Plugin_Installer
endef # INSTALL_VISTA_REDIST_IHV

define INSTALL_VISTA_HSM_BIN
	@echo "#- $0"
	$(INSTALL) src/apps/fsh/win32/bin/$1/bcmfshapi.dll		$@/$2
endef # INSTALL_VISTA_HSM_BIN

define INSTALL_VISTA_FS_ICONS
	@echo "#- $0"
	$(INSTALL) -d  $1
	$(INSTALL) src/apps/fsh/FshSvrAdmin/blank.gif			$1
	$(INSTALL) src/apps/fsh/FshSvrAdmin/folder.gif			$1
	$(INSTALL) src/apps/fsh/FshSvrAdmin/file.gif			$1
	$(INSTALL) src/apps/fsh/FshSvrAdmin/prev.gif			$1
endef # INSTALL_VISTA_FS_ICONS

# Co-Installer files have to reside next to .inf and .sys files
# Co-Installer files have to be signed
define INSTALL_32BIT_BUILT_COINSTALLER
	@echo "#- $0"
	$(INSTALL) $(dir $(INS_DIR))Co-Installer/bcmwlcoi/$1/bcmwlcoi.dll $@/$2
	@$(SIGNTOOL_CMND_32) $$(cygpath -m $@/$2/bcmwlcoi.dll)
endef # INSTALL_32BIT_BUILT_COINSTALLER

define INSTALL_64BIT_BUILT_COINSTALLER
	@echo "#- $0"
	$(INSTALL) $(dir $(INS_DIR))Co-Installer/bcmwlcoi/$1/bcmwlcoi64.dll $@/$2
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/$2/bcmwlcoi64.dll)
endef # INSTALL_64BIT_BUILT_COINSTALLER

# copy over unsigned driver catalog files
# Arg listing: $1=DEST_DIR
define INSTALL_UNSIGNED_DRIVER_CAT
	@echo "#- $0"
	$(INSTALL) src/wl/sys/wdm/bcm43xx.cat $1/$(X86_WLCAT)
	$(INSTALL) src/wl/sys/wdm/bcm43xx.cat $1/$(X64_WLCAT)
endef # INSTALL_UNSIGNED_DRIVER_CAT

# Arg listing: $1=Driver-folder $2=CPU $3=os-type
define RELEASE_SIGN_WL_DRIVER
	@echo "#- $0"
	$(MAKE) -C src/wl/sys/wdm release_sign_driver \
		SIGN_DIR=$1 SIGN_ARCH=$2 RELNUM="$(RELNUM)" \
		$(if $3,WINOS=$3)
endef # RELEASE_SIGN_WL_DRIVER

define RELEASE_SIGN_VISTA_DRIVER
	@echo "#- $0"
	$(call RELEASE_SIGN_WL_DRIVER,$1,$2,$(SIGN_OS))
endef # RELEASE_SIGN_VISTA_DRIVER

define RELEASE_SIGN_XP_DRIVER
	@echo "#- $0"
	$(call RELEASE_SIGN_WL_DRIVER,$1,$2,$(SIGN_OS))
endef # RELEASE_SIGN_XP_DRIVER

# Arg listing: $1=Driver-folder $2=CPU $3=os-type
define GEN_CAT_WL_DRIVER
	@echo "#- $0"
	$(MAKE) -C src/wl/sys/wdm gen_cat_driver \
		SIGN_DIR=$1 SIGN_ARCH="$2" RELNUM="$(RELNUM)" $(if $3,WINOS=$3)
endef # GEN_CAT_WL_DRIVER

define GEN_CAT_WIN7_DRIVER
	@echo "#- $0"
	$(call GEN_CAT_WL_DRIVER,$1,$2,WIN7)
endef # GEN_CAT_WIN7_DRIVER

define GEN_CAT_XP_DRIVER
	@echo "#- $0"
	$(call GEN_CAT_WL_DRIVER,$1,$2,WINXP)
endef # GEN_CAT_XP_DRIVER

# Arg listing: $1=Virtual-Driver-folder $2=CPU $3=os-type
define RELEASE_SIGN_VWL_DRIVER
	@echo "#- $0"
	$(MAKE) -C components/ndis/ndisvwl/sys release_sign_driver \
		SIGN_DIR=$1 SIGN_ARCH=$2 RELNUM="$(RELNUM)" \
		$(if $3,WINOS=$3)
endef # RELEASE_SIGN_VWL_DRIVER

# Install ndis virtual wl driver bits
# Arg listing: $1=dest_dir, $s=os-type (WINXP OR WINVISTA OR WIN7)
define INSTALL_VWL_DRIVER
	@echo "#- $0"
	$(INSTALL) -d $1
	$(INSTALL) $(VIRTUAL_x86_DRIVER)   $1
	$(INSTALL) $(VIRTUAL_x64_DRIVER)   $1
	$(INSTALL) components/ndis/ndisvwl/README.txt $1
	$(call CHECK_INF,components/ndis/ndisvwl,bcmvwl.inf)
	$(call INSTALL_INF,components/ndis/ndisvwl/bcmvwl.inf,$1)
	$(call RELEASE_SIGN_VWL_DRIVER,$1,X86,$2)
	$(call RELEASE_SIGN_VWL_DRIVER,$1,X64,$2)
endef # INSTALL_VWL_DRIVER

define INSTALL_PEERNET_INSTALLER
	@echo "Creating softap installer batchfile"
	@echo "Setup.exe /peernet" > $(@D)/Setup_PeerNet.bat
	chmod +x $(@D)/Setup_PeerNet.bat
endef # INSTALL_PEERNET_INSTALLER


define INSTALL_DOSTOOL
	@echo "#- $0"
	@echo -e "\nSearching in DOSTOOL_DIR = $(DOSTOOL_DIR)\n";
	-$(INSTALL) -D $(DOSTOOL_DIR)/,release.log misc/,dostool_image_release.log
	@dostool=$(DOSTOOL_DIR)/release/dostool/dost*.exe; \
	cwsdpmi=$(DOSTOOL_DIR)/release/dostool/cwsdpmi*.exe; \
	dtdest=$(if $(DOSTOOL_VER_PEGGED),,dostool_$(subst .,_,$(RELNUM)).exe); \
	cwdest=$(if $(DOSTOOL_VER_PEGGED),,cwdest_$(subst .,_,$(RELNUM)).exe); \
	if [ -f "$(DOSTOOL_DIR)/,succeeded" ]; then \
		$(INSTALL) -v $${dostool} $@/$${dtdest}; \
		$(INSTALL) -v $${cwsdpmi} $@/$${cwdest}; \
	else \
		echo -e "\nWARNING: Failed <DOSTOOL_DIR> prebuild FOUND!!"; \
		echo -e "WARNING: dostools not copied\n"; \
	fi
endef # INSTALL_DOSTOOL

# Common steps to create a single installer package
# Installer setup.exe and packaged setup.exe are both signed
define PACKAGE_RELEASE
	@echo "#- $0"
	@if [ -f "$$(cygpath -m $@/setup.exe)" ]; then \
		echo "$(SIGNTOOL_CMND_64) $$(cygpath -m $@/setup.exe)"; \
		$(SIGNTOOL_CMND_64) $$(cygpath -m $@/setup.exe); \
	fi
	$(ISSAB2009_ROOT)/System/ReleasePackager.exe $(subst \,\\,$$(cygpath -w $(@) $(@D)/Setup.exe))
	$(INS_DIR)/installshield/utils/IsReMan.exe /manifest=$(INS_DIR)/installshield/utils/SetupExe.Admin.manifest $$(cygpath -m $(@D)/Setup.exe)
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $(@D)/Setup.exe)
	-@rm -fv $(@D)/Setup.exe.bak
	@if [ -f "$(@D)/Setup.exe" ]; then \
	    echo "Generating folder content list at $(@D)/$(1)_tree.txt"; \
	    pushd $(@D); \
	    $(TREECMD) -o $(1)_tree.txt $(@F); \
	    echo -e "\nNOTE: To unpack with no install run:" >> $(1)_tree.txt; \
	    echo -e "        'Setup.exe -extract_all:<path>'">> $(1)_tree.txt; \
	    popd; \
	    echo "Packaging was successful. Removing $@"; \
	    rm -rf $@; \
	fi

endef # PACKAGE_RELEASE

# Common steps to create a single installer package for WinALL
# Installer setup.exe and packaged SetupBWL.exe are both signed
define PACKAGE_RELEASE_MASTER
	@echo "#- $0"
	@if [ -f "$$(cygpath -m $@/setup.exe)" ]; then \
		echo "$(SIGNTOOL_CMND_64) $$(cygpath -m $@/setup.exe)"; \
		$(SIGNTOOL_CMND_64) $$(cygpath -m $@/setup.exe); \
	fi
	$(ISSAB2009_ROOT)/System/ReleasePackager.exe $(subst \,\\,$$(cygpath -w $@/Disk1 $@/SetupBWL.exe))
	$(INS_DIR)/installshield/utils/IsReMan.exe /manifest=$(INS_DIR)/installshield/utils/SetupExe.Admin.manifest $$(cygpath -m $@/SetupBWL.exe)
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/SetupBWL.exe)
	-@rm -fv $@/SetupBWL.exe.bak
	@if [ -f "$@/SetupBWL.exe" ]; then \
	    echo "Generating folder content list at $@/$(1)_tree.txt"; \
	    pushd $@; \
	    $(TREECMD) -o $@/$(1)_tree.txt; \
	    echo -e "\nNOTE: To unpack with no install run:" >> $@/$(1)_tree.txt; \
	    echo -e "        'SetupBWL.exe -extract_all:<path>'">> $@/$(1)_tree.txt; \
	    popd; \
	    echo "Packaging was successful. Removing $@/Disk1"; \
	    rm -rf $@/Disk1; \
	fi
endef # PACKAGE_RELEASE_MASTER

# $1 is <oem> $2 is <free or checked>
define 	INSTALL_BCMWLS
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/bcmwls/Release/bcmwls32.exe  $@/
	$(INSTALL) $(INS_DIR)/bcmwls/x64/Release/bcmwls64.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst32.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst64.exe $@/
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/bcmwls*.exe)
endef # INSTALL_BCMWLS

# $1 is <oem>
define 	INSTALL_MUIDLLS
	@echo "#- $0"
	mkdir -p $@/MUI
	$(INSTALL) $(INS_DIR)/MUI/ln/Release/$1/en-US/bcmwlrc.dll $@/MUI
	cd $(INS_DIR)/MUI/build/$1; cp -prv --parents */bcmwlrc.dll.mui $@/MUI
	cd $(CCX_MUI_DIR); $(FIND) . -print | cpio -pmud $@/MUI
endef # INSTALL_MUIDLLS

# $1 is <oem>
define 	INSTALL_BCM_MUIS
	@echo "#- $0"
	mkdir -p $@/MUI
	$(INSTALL) $(INS_DIR)/MUI/ln/Release/$1/en-US/bcmwlrc.dll $@/MUI
	cd $(INS_DIR)/MUI/build/$1; cp -prv --parents */bcmwlrc.dll.mui $@/MUI
endef # INSTALL_BCM_MUIS

# $1 is <oem>; $2 <free or checked>; $3 <type of installshield>
define INSTALL_INSTALLSHIELD_FILES
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data1.cab   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data1.hdr   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data2.cab   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/ISSetup.dll $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/_Setup.dll  $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/layout.bin  $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/setup.inx   $@/
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/Setup.exe   $@/IS.exe
	$(INSTALL) $(INS_DIR)/installshield/$1/setup.iss             $@/Setup.iss
	$(INSTALL) $(INS_DIR)/launcher/Release/Launcher.exe          $@/Setup.exe
	$(INSTALL) $(INS_DIR)/launcher/Launcher.ini          	  $@/
	$(INSTALL) $(INS_DIR)/uninstall/Release/$1/bcmwlu00.exe           $@/
	$(call INSTALL_BCMWLS,$1,$2)
endef # INSTALL_INSTALLSHIELD_FILES

# $1 is <oem>; $2 <free or checked>; $3 <type of installshield>
define INSTALL_BCMWLMASTER_IS_FILES
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data1.cab   $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data1.hdr   $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/data2.cab   $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/ISSetup.dll $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/_Setup.dll  $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/layout.bin  $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/setup.inx   $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/Setup.exe   $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/Setup.iss   $@/Disk1
	$(INSTALL) $(INS_DIR)/installshield/$3/is_bin/$1/Setup.ini   $@/Disk1
	unix2dos $@/Disk1/Setup.ini
endef # INSTALL_BCMWLMASTER_IS_FILES

# $1 is <winos-platform>/<oem>
define INSTALL_HELP_FILES
	@echo "#- $0"
	# WL Help are mandatory.
	@curhelpdir=$(BCMWLHLP_DIR)/$1/$(BCMWLHLP_REV_$(subst /,_,$1)); \
		if [ -s "$${curhelpdir}/english/bcmwlhlp.chm" ]; then \
			$(INSTALL) -v $${curhelpdir}/english/bcmwlhlp.chm $@/; \
		else \
			echo "ERROR: Missing $${curhelpdir}/english/bcmwlhlp.chm"; \
			exit 1; \
		fi; \
		if [ -s "$${curhelpdir}/bcmwlhlp.cab" ]; then \
			$(INSTALL) -v $${curhelpdir}/bcmwlhlp.cab $@/; \
		else \
			echo "ERROR: Missing $${curhelpdir}/bcmwlhlp.cab"; \
			exit 1; \
		fi
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/bcmwlhlp.cab)
endef # INSTALL_HELP_FILES

all: build_start
all: mogrify pre_release
all: build_driver
#all: build_trayapp_vista
all: build_install
#all: build_trayapp_vista_dotfuscate
all: release build_status
all: build_end

#disabled# include $(PARENT_MAKEFILE)

include $(MOGRIFY_RULES)
include $(BRAND_RULES)
include unreleased-chiplist.mk

build_start:
	@$(MARKSTART_BRAND)

# All exported MOGRIFY_xxx variables are referenced by following step
mogrify:
	@$(MARKSTART)
	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)	

build_include: mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)	

build_trayapp: build_trayapp_vista

build_trayapp_vista: mogrify build_include
	@$(MARKSTART)
	$(MAKE) -C src/epiprot
	$(MAKE) -C components/apps/windows/ihvfrm/ihv \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV)"
	$(MAKE) -C $(dir $(INS_DIR))Co-Installer/bcmwlcoi \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV)"
ifeq ($(BUILD_TRAYAPP),true)
	$(MAKE) -C components/apps/windows/tray/winx \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_APP)"
	$(MAKE) -C components/apps/windows/tray/winx/Samples \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_UNICODE)" \
		   OEM_LIST=bcm
	$(MAKE) -C components/apps/windows/tray/winx/Tools/WLVTT \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA)"
	$(MAKE) -C components/apps/windows/ihvfrm/eaphost/ttls \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_TTLS)"
	$(MAKE) -C $(dir $(INS_DIR))BcmSetupUtil \
		   OEM_LIST="bcm" \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_XP_APP)"
	$(MAKE) -C $(INS_DIR)/MUI \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV)"
endif # BUILD_TRAYAPP
	@$(MARKEND)

build_trayapp_vista_dotfuscate:
ifeq ($(BUILD_TRAYAPP),true)
	@$(MARKSTART)
ifeq ($(strip $(DOTFUSCATE_APP)),true)
	# Dotfuscate all vista objects
	$(MAKE) -C components/apps/windows/tray/winx/Obfuscator/Dotfuscator \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_UNICODE)"
	# Dotfuscate just bcmwlapi
	$(MAKE) -C components/apps/windows/tray/winx/Obfuscator/Dotfuscator \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA)" \
                   WLVISTA_TEMPLATE=WLVista_File.template \
                   SIGN_APP=false
	# Dotfuscate vista wlvtt tool (hp)
	$(MAKE) -C components/apps/windows/tray/winx/Tools/WLVTT/Dotfuscator \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA)"
else # DOTFUSCATE_APP
	@echo "WARN: Dotfuscation $(DOTFUSCATOR_DIR) not found or"
	@echo "WARN: DOTFUSCATE_APP has been set to false"
endif # DOTFUSCATE_APP
	@$(MARKEND)
endif # BUILD_TRAYAPP

build_trayapp_xp2k: mogrify build_include
	@$(MARKSTART)
	$(MAKE) -C src/epiprot
ifeq ($(BUILD_TRAYAPP),true)
	$(MAKE) -C src/wl/cpl PROJECT_CONFIGS="$(PROJECT_CONFIGS_XP_APP)"
	$(MAKE) -C src/wl/cpl/preflib/wlconfig PROJCFG=Release package
	$(MAKE) -C src/wl/cpl/preflib/preflibcl
	$(MAKE) -C src/wl/cpl/preflib/preflibcl/ex1
	$(MAKE) -C src/wl/cpl/preflib/preflibcl/prefdump
	$(MAKE) -C src/wl/cpl/preflib/preflibcl package
endif # BUILD_TRAYAPP
	@$(MARKEND)

build_driver: mogrify build_include
	@$(MARKSTART)
	@echo " -- MARK build driver $(BLDTYPE) --"
ifeq ($(BCM_MFGTEST),)
	$(MAKE) -C src/wl/sys/wdm BUILD_TYPES=free \
		build_win7_driver
	$(MAKE) -C components/ndis/ndisvwl/sys BUILD_TYPES=free
	$(MAKE) -C src/wl/exe
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlu_dll
else # MFGTEST
	$(MAKE) -C src/wl/sys/wdm WLTEST=1 \
		build_win7_driver
	$(MAKE) -C components/ndis/ndisvwl/sys BUILD_TYPES=free WLTEST=1
	$(MAKE) -C src/wl/exe WLTEST=1
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll WLTEST=1
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlu_dll WLTEST=1
	# Vista or Win7 version of wlu dll for mfgtest
	$(MAKE) -C src/wl/exe/win7 WLTEST=1 \
		PROJECT_CONFIGS="$(PROJECT_CONFIGS_GENERIC_ALL)"
#	$(MAKE) -C src/tools/mfgc WLTEST=1 nokia
	# Winxp version of wlu dll for mfgtest is built by wl_base
#	$(MAKE) -C src/tools/mfgc wl_base
#	$(MAKE) -C src/tools/mfgc nokmfg
	$(MAKE) -C src/tools/misc nvserial.exe
endif # BCM_MFGTEST
	@$(MARKEND)

build_install: export VS_PRE_BUILD_ENABLED=0
build_install: mogrify
	@$(MARKSTART)
	@echo " -- MARK $@ --"
ifeq ($(BCM_MFGTEST),)
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND)
else  # BCM_MFGTEST
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND) WLTEST=1
endif # BCM_MFGTEST
	@$(MARKEND)

clean:
	@$(MARKSTART)
	$(MAKE) -C src/include clean
	$(MAKE) -C $(INS_DIR) clean
	$(MAKE) -C src/wl/exe clean
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlm_dll clean
	$(MAKE) -C src/wl/exe -f GNUmakefile.wlu_dll clean
	$(MAKE) -C src/wl/sys/wdm clean
	$(MAKE) -C components/ndis/ndisvwl/sys clean
	$(MAKE) -C components/apps/windows/tray/winx clean_all
	$(MAKE) -C components/apps/windows/ihvfrm/eaphost/ttls clean_all
	$(MAKE) -C components/apps/windows/ihvfrm/ihv clean_all
	$(MAKE) -C $(dir $(INS_DIR))Co-Installer/bcmwlcoi clean_all
	@$(MARKEND)

# pre_release is required to ensure that individual <oem> specific
# release/<oem> make targets run independently in parallel builds
pre_release:
	@$(MARKSTART)
	@[ -d release ]          || mkdir -pv release
	# Creating XP release folders
#	@for oem in $(OEM_LIST_XP_RLSPKG); do \
#	  [ -d release/WinXP/$${oem} ] || mkdir -pv release/WinXP/$${oem}; \
#	done
#	@[ -d release/WinXP/Internal ] || mkdir -pv release/WinXP/Internal
	@for oem in $(OEM_LIST_WIN7_RLSPKG); do \
	  [ -d release/Win7/$${oem} ] || mkdir -pv release/Win7/$${oem}; \
	done
	@[ -d release/Win7/Internal ] || mkdir -pv release/Win7/Internal
ifneq ($(BUILD_WINALL_KIT),false)
	@for oem in $(OEM_LIST_WINALL_RLSPKG); do \
	  [ -d release/WinALL/$${oem} ] || mkdir -pv release/WinALL/$${oem}; \
	done
endif
	# Creating PE release folders
#	@for oem in $(OEM_LIST_PE_RLSPKG); do \
#	  [ -d release/WinPE/$${oem} ] || mkdir -pv release/WinPE/$${oem}; \
#	done
	@$(MARKEND)

release: $(TARGET_BUILD_LIST)

## WinXP/2K release packaging after everything is built
release_xp: pre_release
release_xp: $(OEM_LIST_XP_RLSPKG:%=$(RELEASEDIR)/WinXP/%)
	@$(MARKSTART)
	-$(INSTALL) src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

## WinVista and Win7 release packaging after everything is built
release_win7: pre_release
release_win7: $(OEM_LIST_WIN7_RLSPKG:%=$(RELEASEDIR)/Win7/%)
	@$(MARKSTART)
	-$(INSTALL) src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

## WinAll packaging
release_winall: pre_release
release_winall: $(OEM_LIST_WINALL_RLSPKG:%=$(RELEASEDIR)/WinALL/%)
	@$(MARKSTART)
	-$(INSTALL) src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

## WinPE release packaging after everything is built
release_pe: pre_release
release_pe: $(OEM_LIST_PE_RLSPKG:%=$(RELEASEDIR)/WinPE/%)
	@$(MARKSTART)
	-$(INSTALL) src/doc/ReadmeReleaseDir_$(BRAND).txt release/README.txt
	@$(MARKEND)

# release_peernet_sdk is not used or called directly by any build step.
# instead individual OEMs call their own peernet packaging.
# This target is here for test purposes only
release_peernet_sdk: $(RELEASEDIR)/Win7/Bcm/Bcm_Peernet_SDK \
		     $(RELEASEDIR)/Win7/HP/HP_Peernet_SDK   \
		     $(RELEASEDIR)/Win7/Dell/Dell_Peernet_SDK

#
# Release the BCM branded files.  These are really 'unbranded'
# by definition, but some customers may get these files and just
# live with that fact that they have Broadcom logos and copyrights.
#

# WinXP/2K
$(RELEASEDIR)/WinXP/Bcm: \
		   $(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly
#		   $(RELEASEDIR)/WinXP/Internal

ifeq ($(BUILD_TRAYAPP),true)
ifeq ($(WinXP_TRAYAPP),true)
$(RELEASEDIR)/WinXP/Bcm: \
		   $(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly_PreWHQL \
		   $(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly_CAB \
		   $(RELEASEDIR)/WinXP/Bcm/Bcm_InstallShield/Disk1 \
		   $(RELEASEDIR)/WinXP/Bcm/Bcm_Docs \
		   $(RELEASEDIR)/WinXP/DebugSymbols/Bcm

# WinALL All Packages
$(RELEASEDIR)/WinALL/Bcm: \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield_Driver \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_DriverOnly

# WinALL InstallShield Package
$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield: \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield/Disk1/WinXP \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield/Disk1/Win7
	# Install Master installer files
	$(call INSTALL_BCMWLMASTER_IS_FILES,bcm,free,BcmWLMaster)
	# Generate master installer
	$(call PACKAGE_RELEASE_MASTER,Bcm_App)

# WinALL InstallShield_Driver Package
$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield_Driver: \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield_Driver/Disk1/Win7
	# Install Master installer files
	$(call INSTALL_BCMWLMASTER_IS_FILES,bcm,free,BcmWLMaster)
	# Generate master installer
	$(call PACKAGE_RELEASE_MASTER,Bcm_Driver)

# WinALL DriverOnly Package
$(RELEASEDIR)/WinALL/Bcm/Bcm_DriverOnly: \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_DriverOnly/WinXP \
	$(RELEASEDIR)/WinALL/Bcm/Bcm_DriverOnly/Win7

endif # WinXP_TRAYAPP
else # BUILD_TRAYAPP
$(RELEASEDIR)/WinXP/Bcm: \
		   $(RELEASEDIR)/WinXP/Bcm/Bcm_Apps

$(RELEASEDIR)/Win7/Bcm: \
		   $(RELEASEDIR)/Win7/Bcm/Bcm_Apps

# Dummy WinALL Bcm package for mfgtest
$(RELEASEDIR)/WinALL/Bcm:

endif # BUILD_TRAYAPP

# WinXP/2K
$(RELEASEDIR)/WinXP/Bcm/Bcm_Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	@cp -pv $(BCMWLHLP_DIR)/xp/bcm/$(BCMWLHLP_REV_xp_bcm)/*.cab $@
	@$(MARKEND)

# WinXP/2K
$(RELEASEDIR)/WinXP/DebugSymbols/Bcm: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call INSTALL_XP_APP_DEBUG_SYMBOLS,bcm)
	$(INSTALL) src/wl/cpl/neptune/cpl/Release/bcm/bcmwlcpl.pdb $@/
	@$(MARKEND)

# WinXP/2K
$(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly \
$(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly_PreWHQL \
$(RELEASEDIR)/WinALL/Bcm/Bcm_DriverOnly/WinXP: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER)  $@/
	$(INSTALL) $(XP_x64_WLDRIVER)  $@/
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_INF,$(INF_DIR)/bcm/$(XP_WLINF_COI),$@/$(XP_WLINF)),\
	     $(call INSTALL_INF,$(INF_DIR)/bcm/$(XP_WLINF),$@/) \
	)
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_32BIT_BUILT_COINSTALLER,release))
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	# End of $(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly

# WinXP/2K
# Bcm driver-only cab package (for BCM/OEM/ODM test labs)
$(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly_CAB: FORCE
	@$(MARKSTART)
	rm -rf $@
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER) $@/
	$(INSTALL) $(XP_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(XP_WLINF),$@/)
	$(call GEN_CAT_XP_DRIVER,$@,XP_X86 XP_X64 2000)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	cd $@; $(CABARC) -r -p N $(subst _CAB,.CAB,$(@F)) *.cat
	cd $@; $(FIND) . -type f \! -name $(subst _CAB,.CAB,$(@F)) -print | \
		xargs rm -fv
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/*.CAB)
	@$(MARKEND)
#       End of $(RELEASEDIR)/WinXP/Bcm/Bcm_DriverOnly_CAB

# WinXP/2K
$(RELEASEDIR)/WinXP/Bcm/Bcm_InstallShield/Disk1 \
$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield/Disk1/WinXP: FORCE
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@/X64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/WinXP/WL
	# install driver and its dependent files
	$(INSTALL) $(XP_x86_WLDRIVER) $@/Drivers/WinXP/WL
	$(INSTALL) $(XP_x64_WLDRIVER) $@/Drivers/WinXP/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(XP_WLINF),$@/Drivers/WinXP/WL)
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/Drivers/WinXP/WL)
	# Install tray/gui app
	$(call INSTALL_BRCM_SUPPLICANT,Release/bcm1xsup.dll,$@/)
	$(call INSTALL_VENDORLIBS_DLLS,WLBCGCBPRO731.dll,$@/)
	$(call INSTALL_VS0532BIT_VCREDIST,free,$@/)
	$(call INSTALL_XP32BIT_BUILT_APPS,Release/bcm)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/WinXP/WL)
	$(call INSTALL_VENDORLIBS_DLLS,win64/WLBCGCBPRO731.dll,$@/x64)
	$(call INSTALL_VS0564BIT_VCREDIST,free,$@/x64)
	$(call INSTALL_XP64BIT_BUILT_APPS,x64/Release/bcm,x64)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/WinXP/WL)
	# install help
	$(call INSTALL_HELP_FILES,xp/bcm)
	# Install installer
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,free,BCMAppInstaller)
	$(INSTALL) $(INS_DIR)/ini/bcm/free/external.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini/bcm/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup.ini $@/
	unix2dos $@/Setup.ini
	# Check Bios info
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	# install misc
	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,Bcm_XP))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/WinXP/Bcm/Bcm_InstallShield/Disk1

# WinXP
$(RELEASEDIR)/WinXP/Bcm/Bcm_Apps: FORCE
ifneq ($(BCM_MFGTEST),)
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL) src/wl/exe/windows/winxp/obj/mfg_dll/free/brcm_wlu.dll   $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/wlm/free/wlm.lib $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/wlm/free/wlm.dll $@/
	$(INSTALL) src/wl/exe/windows/winxp/obj/external/free/wl.exe $@/
	$(INSTALL) src/tools/misc/nvserial.exe $@/
	@$(MARKEND)
endif # BCM_MFGTEST

# WinVISTA and Win7
$(RELEASEDIR)/Win7/Bcm: \
	$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly \
	$(RELEASEDIR)/Win7/Internal \
	$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly_VWL
#	$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly_IS

# WinVISTA and Win7 for non-mfgtest
ifeq ($(BUILD_TRAYAPP),true)
$(RELEASEDIR)/Win7/Bcm: \
	$(RELEASEDIR)/Win7/Bcm/Bcm_InstallShield/Disk1 \
	$(RELEASEDIR)/Win7/Bcm/Bcm_InstallShield_Driver/Disk1 \
	$(RELEASEDIR)/Win7/DebugSymbols/Bcm \
	$(RELEASEDIR)/Win7/Bcm/Bcm_Peernet_SDK
#	$(RELEASEDIR)/Win7/Bcm/Bcm_MaestroExt_SDK
#	$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly_CAB
#	$(RELEASEDIR)/Win7/Bcm/Bcm_Docs

endif # BUILD_TRAYAPP

# WinVISTA and Win7 for mfgtest
$(RELEASEDIR)/Win7/Bcm/Bcm_Apps: FORCE
ifneq ($(BCM_MFGTEST),)
	@$(MARKSTART)
	mkdir -p $@ $@/x64
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/free/wl.exe        $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/mfg_dll/Release/brcm_wlu.dll      $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/wlm/free/wlm.dll     $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/wlm/free/wlm.lib     $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/x64/mfg_dll/Release/brcm_wlu.dll  $@/x64
	@$(MARKEND)
endif # BCM_MFGTEST

# Common PeerNet SDK packaging target all OEMs
# WinVISTA and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_Peernet_SDK \
$(RELEASEDIR)/Win7/HP/HP_Peernet_SDK   \
$(RELEASEDIR)/Win7/Dell/Dell_Peernet_SDK: OEM=$(word 1,$(subst /, ,$(subst $(RELEASEDIR)/Win7/,,$@)))
$(RELEASEDIR)/Win7/Bcm/Bcm_Peernet_SDK \
$(RELEASEDIR)/Win7/HP/HP_Peernet_SDK   \
$(RELEASEDIR)/Win7/Dell/Dell_Peernet_SDK: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@date +"START: $@, %D %T"
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -p $@/common $@/Sample/PeerTest $@/Docs
	mkdir -p $@/Sample/Lib/{Release,Debug}/{win32,x64}
	mkdir -p $@/Sample/Bin/{Release,Debug}/{win32,x64}
	# Copy documents
	$(INSTALL) components/apps/windows/tray/winx/Peernet/Docs/PeernetSDK.doc $@/Docs
	# Copy common and sample peernet sdk and test files
	tar cpf - -C components/apps/windows/tray/winx/Peernet \
		--exclude=*/CVS \
		--exclude=*/.svn \
		--exclude=*/ReleaseUv \
		--exclude=*/Releasev \
		--exclude=*/DebugUv \
		--exclude=*/Debugv \
		common Sample/PeerTest | \
	tar xvpf - -C $@/
	# Copy peernet library in Release/{win32,x64} folders
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/$(OEM),Sample/Lib/Release/win32)
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/x64/$(OEM),Sample/Lib/Release/x64)
	# Copy peernet release library in Debug/{win32,x64} folders
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/$(OEM),Sample/Lib/Debug/win32)
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/x64/$(OEM),Sample/Lib/Debug/x64)
	# Copy peertest release binary in Release/{win32,x64} folders
	$(call INSTALL_VISTA_PEERTEST_BIN,ReleaseUv,Sample/Bin/Release/win32)
	$(call INSTALL_VISTA_PEERTEST_BIN,ReleaseUv/x64,Sample/Bin/Release/x64)
	# Copy peertest release binary in Debug/{win32,x64} folders
	$(call INSTALL_VISTA_PEERTEST_BIN,ReleaseUv,Sample/Bin/Debug/win32)
	$(call INSTALL_VISTA_PEERTEST_BIN,ReleaseUv/x64,Sample/Bin/Debug/x64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@date +"END: $@, %D %T"
	@$(MARKEND)
#	# End of $(RELEASEDIR)/Win7/Bcm/Bcm_Peernet_SDK

# Common Maestro External SDK packaging target all OEMs (derivative of peernet)
# WinVISTA and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_MaestroExt_SDK \
$(RELEASEDIR)/Win7/HP/HP_MaestroExt_SDK   \
$(RELEASEDIR)/Win7/Dell/Dell_MaestroExt_SDK: OEM=$(word 1,$(subst /, ,$(subst $(RELEASEDIR)/Win7/,,$@)))
$(RELEASEDIR)/Win7/Bcm/Bcm_MaestroExt_SDK \
$(RELEASEDIR)/Win7/HP/HP_MaestroExt_SDK   \
$(RELEASEDIR)/Win7/Dell/Dell_MaestroExt_SDK: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@date +"START: $@, %D %T"
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -p $@/common $@/Docs
	mkdir -p $@/Sample/Lib/{Release,Debug}/{win32,x64}
	mkdir -p $@/Sample/Bin/{Release,Debug}/{win32,x64}
	mkdir -p $@/Sample/MaestroExtTest
	# Copy common sample peernet files
	install -p components/apps/windows/tray/winx/Peernet/common/PeernetInt.h $@/common
	install -p components/apps/windows/tray/winx/Peernet/common/PeernetInt.cpp $@/common
	install -p components/apps/windows/tray/winx/Peernet/common/Peernetapi.h $@/common
	# Copy documents
	$(INSTALL) components/apps/windows/tray/winx/Peernet/Docs/MaestroExtSDK.doc $@/Docs
	# Copy sample MaestroExtSDK test files
	tar cpf - -C components/apps/windows/tray/winx/Peernet \
		--exclude=*/CVS \
		--exclude=*/.svn \
		--exclude=*/ReleaseUv \
		--exclude=*/Releasev \
		--exclude=*/DebugUv \
		--exclude=*/Debugv \
		Sample/MaestroExtTest | \
	tar xvpf - -C $@/
	# Copy peernet library in Release/{win32,x64} folders
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/$(OEM),Sample/Lib/Release/win32)
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/x64/$(OEM),Sample/Lib/Release/x64)
	# Copy peernet release library in Debug/{win32,x64} folders
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/$(OEM),Sample/Lib/Debug/win32)
	$(call INSTALL_VISTA_PEERNET_LIB,ReleaseUv/x64/$(OEM),Sample/Lib/Debug/x64)
	# Copy MaestroExtTest release binary in Release/{win32,x64} folders
	$(call INSTALL_VISTA_MAESTROEXT_BIN,ReleaseUv,Sample/Bin/Release/win32)
	$(call INSTALL_VISTA_MAESTROEXT_BIN,ReleaseUv/x64,Sample/Bin/Release/x64)
	# Copy MaestroExtTest release binary in Debug/{win32,x64} folders
	$(call INSTALL_VISTA_MAESTROEXT_BIN,ReleaseUv,Sample/Bin/Debug/win32)
	$(call INSTALL_VISTA_MAESTROEXT_BIN,ReleaseUv/x64,Sample/Bin/Debug/x64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@date +"END: $@, %D %T"
	@$(MARKEND)
#	# End of $(RELEASEDIR)/Win7/Bcm/Bcm_MaestroExt_SDK

# WinVISTA and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly \
$(RELEASEDIR)/WinALL/Bcm/Bcm_DriverOnly/Win7: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(VISTA_WLINF),$@/)
#	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
#	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X86)
#	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
#	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	# End of $(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly

# WinVista and Win7
# BCM driver-only cab package (for BCM/OEM/ODM test labs)
$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly_CAB: FORCE
	@$(MARKSTART)
	rm -rf $@
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(VISTA_WLINF),$@/)
#	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
#	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
#	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
#	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call GEN_CAT_WIN7_DRIVER,$@,VISTA_X86 VISTA_X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	cd $@; $(CABARC) -r -p N $(subst _CAB,.CAB,$(@F)) *.cat
	cd $@; $(FIND) . -type f \! -name $(subst _CAB,.CAB,$(@F)) -print | \
		xargs rm -fv
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/*.CAB)
	@$(MARKEND)
#       End of $(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly_CAB

# WinVista and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_DriverOnly_VWL: FORCE
	@$(MARKSTART)
	$(call INSTALL_VWL_DRIVER,$@,WIN7)
	@$(MARKEND)

# WinVista and Win7
$(RELEASEDIR)/Win7/DebugSymbols/Bcm: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	mkdir -p $@/x64
	$(call INSTALL_VISTA_APP_DEBUG_SYMBOLS,ReleaseUv/bcm)
	$(call INSTALL_VISTA_APP_DEBUG_SYMBOLS,ReleaseUv/x64/bcm,x64)
	@$(MARKEND)

# WinVista and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	@cp -pv $(BCMWLHLP_DIR)/vista/bcm/$(BCMWLHLP_REV_vista_bcm)/*.cab $@
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/*.cab)
	@$(MARKEND)

# WinVista and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_InstallShield/Disk1 \
$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield/Disk1/Win7: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@/x64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(VISTA_WLINF),$@/Drivers/Win7/WL)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm,Drivers/Win7/WL)
	$(call INSTALL_VISTA_REDIST_IHV)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/Win7/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/Win7/WL)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	# Install tray/gui app
	$(call INSTALL_VS0532BIT_VCREDIST,free,$@/)
	$(call INSTALL_VS0832BIT_VCREDIST,free,$@/vs08)
	$(call INSTALL_VISTA_BUILT_APPS,ReleaseUv/bcm)
	$(call INSTALL_VISTA_BUILT_TTLS,Release/bcm)
	$(call INSTALL_VISTA_HSM_BIN,Release)
	# install 64bit driver and app
	$(call INSTALL_VS0564BIT_VCREDIST,free,$@/x64)
	$(call INSTALL_VS0864BIT_VCREDIST,free,$@/vs08/x64)
	$(call INSTALL_VISTA_BUILT_APPS,ReleaseUv/x64/bcm,x64)
	$(call INSTALL_VISTA_BUILT_TTLS,x64/Release/bcm,x64)
	$(call INSTALL_VISTA_HSM_BIN,Release/x64,x64)
	# install File Sharing icons
	$(call INSTALL_VISTA_FS_ICONS,$@/FshSvrAdmin)
	# install help
	$(call INSTALL_HELP_FILES,vista/bcm)
	# Installshield files
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,free,BCMAppInstaller)
	$(call INSTALL_MUIDLLS,bcm)
	$(INSTALL) $(INS_DIR)/ini_vista/bcm/free/external.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini_vista/bcm/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	# Check Bios info
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	# install misc and package full installer
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/Release/BcmSetupUtil.exe $@/
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	$(INSTALL) $(VISTA_x86_PROTOCOLDRIVER) $@/
	# install WinPcap driver and dlls
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/npf.sys $@/
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/Packet.dll $@/
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/npf.sys $@/x64
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/Packet.dll $@/x64
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/x64/Release/BcmSetupUtil.exe $@/x64
	$(INSTALL) $(VISTA_x64_PROTOCOLDRIVER) $@/x64
#vista>	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(INSTALL) src/doc/READMEBCMCli_Vista.rtf $@/README.rtf
	$(INSTALL) src/doc/BCMLogo.gif $@/
	# Create peernet installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call INSTALL_PEERNET_INSTALLER,$@))
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,Bcm_Win7))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/Bcm/Bcm_InstallShield/Disk1

# WinVista and Win7
$(RELEASEDIR)/Win7/Bcm/Bcm_InstallShield_Driver/Disk1 \
$(RELEASEDIR)/WinALL/Bcm/Bcm_InstallShield_Driver/Disk1/Win7: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(VISTA_WLINF),$@/Drivers/Win7/WL)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm,Drivers/Win7/WL)
	$(call INSTALL_VISTA_REDIST_IHV)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/Win7/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/Win7/WL)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,free,BCMAppInstaller)
	$(INSTALL) $(INS_DIR)/ini_vista/bcm/free/driver.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini_vista/bcm/free/driver.ini $@/bcmwlsd.ini
	$(call INSTALL_MUIDLLS,bcm)
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setupd_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	# Create peernet installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call INSTALL_PEERNET_INSTALLER,$@))
	# Check Bios info
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,Bcm_Driver_Win7))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/Bcm/Bcm_InstallShield_Driver/Disk1

################################################################################
# Release the HP branded files.
################################################################################

# WinXP/2K
$(RELEASEDIR)/WinXP/HP: \
		  $(RELEASEDIR)/WinXP/HP/HP_InstallShield \
		  $(RELEASEDIR)/WinXP/HP/HP_InstallShield/Disk1 \
		  $(RELEASEDIR)/WinXP/HP/HP_InstallShield_Driver/Disk1 \
		  $(RELEASEDIR)/WinXP/HP/HP_DriverOnly \
		  $(RELEASEDIR)/WinXP/HP/HP_DriverOnly_CAB \
		  $(RELEASEDIR)/WinXP/HP/HP_Docs \
		  $(RELEASEDIR)/WinXP/DebugSymbols/HP

# WinALL All Packages
$(RELEASEDIR)/WinALL/HP: \
	$(RELEASEDIR)/WinALL/HP/HP_InstallShield \
	$(RELEASEDIR)/WinALL/HP/HP_InstallShield_Driver \
	$(RELEASEDIR)/WinALL/HP/HP_DriverOnly

# WinALL InstallShield Package
$(RELEASEDIR)/WinALL/HP/HP_InstallShield: \
	$(RELEASEDIR)/WinALL/HP/HP_InstallShield/Disk1/WinXP \
	$(RELEASEDIR)/WinALL/HP/HP_InstallShield/Disk1/Win7 \
	# Install Master installer files
	@$(MARKSTART)
	$(call INSTALL_BCMWLMASTER_IS_FILES,hp,free,BcmWLMaster)
	# Generate master installer
	$(call PACKAGE_RELEASE_MASTER,HP_App)
	@$(MARKEND)

# WinALL InstallShield_Driver Package
$(RELEASEDIR)/WinALL/HP/HP_InstallShield_Driver: \
	$(RELEASEDIR)/WinALL/HP/HP_InstallShield_Driver/Disk1/WinXP \
	$(RELEASEDIR)/WinALL/HP/HP_InstallShield_Driver/Disk1/Win7
	# Install Master installer files
	@$(MARKSTART)
	$(call INSTALL_BCMWLMASTER_IS_FILES,hp,free,BcmWLMaster)
	# Generate master installer
	$(call PACKAGE_RELEASE_MASTER,HP_Driver)
	@$(MARKEND)

# WinALL DriverOnly Package
$(RELEASEDIR)/WinALL/HP/HP_DriverOnly: \
	$(RELEASEDIR)/WinALL/HP/HP_DriverOnly/WinXP \
	$(RELEASEDIR)/WinALL/HP/HP_DriverOnly/Win7

# WinXP/2K
$(RELEASEDIR)/WinXP/HP/HP_InstallShield: FORCE $(PRE_RELEASE_DEPS_XP)
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/doc/ReleaseNotesHP.html $@/ReleaseNotes.html
	@$(MARKEND)

# WinXP/2K - App
$(RELEASEDIR)/WinXP/HP/HP_InstallShield/Disk1 \
$(RELEASEDIR)/WinALL/HP/HP_InstallShield/Disk1/WinXP: FORCE $(PRE_RELEASE_DEPS_XP)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@/X64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/WinXP/WL
	# install driver and its dependent files
	$(INSTALL) $(XP_x86_WLDRIVER) $@/Drivers/WinXP/WL
	$(INSTALL) $(XP_x64_WLDRIVER) $@/Drivers/WinXP/WL
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/Drivers/WinXP/WL)
	$(call INSTALL_INF,$(INF_DIR)/hp/$(XP_WLINF),$@/Drivers/WinXP/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/WinXP/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/WinXP/WL)
	# install tray/gui app
	$(call INSTALL_BRCM_SUPPLICANT,Release/bcm1xsup.dll,$@/)
	$(call INSTALL_VENDORLIBS_DLLS,WLBCGCBPRO731.dll,$@)
	$(call INSTALL_VS0532BIT_VCREDIST,free,$@/)
	$(call INSTALL_XP32BIT_BUILT_APPS,Release/hp)
	$(call INSTALL_VENDORLIBS_DLLS,win64/WLBCGCBPRO731.dll,$@/x64)
	$(call INSTALL_VS0564BIT_VCREDIST,free,$@/x64)
	$(call INSTALL_XP64BIT_BUILT_APPS,x64/Release/hp,x64)
	# install help
	$(call INSTALL_HELP_FILES,xp/hp)
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,hp,free,BCMAppInstaller)
	$(INSTALL) $(INS_DIR)/ini/hp/free/external.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini/hp/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/hp/Setupa.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/wladmin/hp/free/wladmin.exe $@/
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	# install misc and package full installer
	$(INSTALL) src/doc/READMEHPCli.rtf $@/README.rtf
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,HP_XP_App))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/WinXP/HP/HP_InstallShield/Disk1

# WinXP/2K - Driver
# Driver Only InstallShield package
$(RELEASEDIR)/WinXP/HP/HP_InstallShield_Driver/Disk1 \
$(RELEASEDIR)/WinALL/HP/HP_InstallShield_Driver/Disk1/WinXP: FORCE $(PRE_RELEASE_DEPS_XP)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/WinXP/WL
	# install driver and its dependent files
	$(INSTALL) $(XP_x86_WLDRIVER) $@/Drivers/WinXP/WL
	$(INSTALL) $(XP_x64_WLDRIVER) $@/Drivers/WinXP/WL
	$(call INSTALL_INF,$(INF_DIR)/hp/$(XP_WLINF),$@/Drivers/WinXP/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/WinXP/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/WinXP/WL)
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/Drivers/WinXP/WL)
	$(call INSTALL_BRCM_SUPPLICANT,Release/bcm1xsup.dll,$@/Drivers/WinXP/WL)
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,hp,free,BCMAppInstaller)
	$(INSTALL) $(INS_DIR)/ini/hp/free/driver.ini $@/bcmwls.ini
	# Need this to support -justdriver switch
	$(INSTALL) $(INS_DIR)/ini/hp/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/hp/Setupd.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,HP_XP_Driver))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/WinXP/HP/HP_InstallShield_Driver/Disk1

# WinXP/2K
# HP driver-only binaries
$(RELEASEDIR)/WinXP/HP/HP_DriverOnly \
$(RELEASEDIR)/WinALL/HP/HP_DriverOnly/WinXP: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER) $@/
	$(INSTALL) $(XP_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/hp/$(XP_WLINF),$@/)
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	# End of $(RELEASEDIR)/WinXP/HP/HP_DriverOnly

# WinXP/2K
# HP driver-only cab package (for HP test labs)
$(RELEASEDIR)/WinXP/HP/HP_DriverOnly_CAB: FORCE
	@$(MARKSTART)
	rm -rf $@
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER) $@/
	$(INSTALL) $(XP_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/hp/$(XP_WLINF),$@/)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call GEN_CAT_XP_DRIVER,$@,XP_X86 XP_X64 2000)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	cd $@; $(CABARC) -r -p N $(subst _CAB,.CAB,$(@F)) *.cat
	cd $@; $(FIND) . -type f \! -name $(subst _CAB,.CAB,$(@F)) -print | \
	       xargs rm -fv
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/*.CAB)
	@$(MARKEND)
#	End of $(RELEASEDIR)/WinXP/HP/HP_DriverOnly_CAB

# WinXP/2K
# HP docs.
$(RELEASEDIR)/WinXP/HP/HP_Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	@cp -pv $(BCMWLHLP_DIR)/xp/hp/$(BCMWLHLP_REV_xp_hp)/*.cab $@
	@$(MARKEND)

# WinXP/2K
# HP debug symbols
$(RELEASEDIR)/WinXP/DebugSymbols/HP: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call INSTALL_XP_APP_DEBUG_SYMBOLS,hp)
	$(INSTALL) src/wl/cpl/neptune/cpl/Release/hp/bcmwlcpl.pdb $@/
	@$(MARKEND)

# -------------------------------------------------------------------------
# HP Driver Only InstallShield for VISTA
# WinVista and Win7 (NOTE: Manual copy items are marked with #vista>)

# HP Driver Only InstallShield for WinVista and Win7
$(RELEASEDIR)/Win7/HP: \
		       $(RELEASEDIR)/Win7/HP/HP_DriverOnly \
		       $(RELEASEDIR)/Win7/HP/HP_DriverOnly_CAB \
		       $(RELEASEDIR)/Win7/HP/HP_InstallShield/Disk1 \
		       $(RELEASEDIR)/Win7/HP/HP_InstallShield_Driver/Disk1 \
		       $(RELEASEDIR)/Win7/HP/HP_Peernet_SDK \
		       $(RELEASEDIR)/Win7/HP/HP_MaestroExt_SDK

# WinVista and Win7
$(RELEASEDIR)/Win7/HP/HP_DriverOnly \
$(RELEASEDIR)/WinALL/HP/HP_DriverOnly/Win7: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/hp/$(VISTA_WLINF),$@/)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/HP/HP_DriverOnly

# WinVista and Win7
# HP driver-only cab package (for HP test labs)
$(RELEASEDIR)/Win7/HP/HP_DriverOnly_CAB: FORCE
	@$(MARKSTART)
	rm -rf $@
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/hp/$(VISTA_WLINF),$@/)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call GEN_CAT_WIN7_DRIVER,$@,VISTA_X86 VISTA_X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	cd $@; $(CABARC) -r -p N $(subst _CAB,.CAB,$(@F)) *.cat
	cd $@; $(FIND) . -type f \! -name $(subst _CAB,.CAB,$(@F)) -print | \
	       xargs rm -fv
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/*.CAB)
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/HP/HP_DriverOnly_CAB

# WinVista and Win7
$(RELEASEDIR)/Win7/HP/HP_InstallShield/Disk1 \
$(RELEASEDIR)/WinALL/HP/HP_InstallShield/Disk1/Win7: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@/x64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/hp/$(VISTA_WLINF),$@/Drivers/Win7/WL)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm,Drivers/Win7/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm,Drivers/Win7/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/Win7/WL)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	$(call INSTALL_VISTA_REDIST_IHV)
	# Install tray/gui app
	$(call INSTALL_VS0532BIT_VCREDIST,free,$@/)
	$(call INSTALL_VS0832BIT_VCREDIST,free,$@/vs08)
	$(call INSTALL_VISTA_BUILT_APPS,ReleaseUv/hp)
	$(call INSTALL_VISTA_BUILT_TTLS,Release/hp)
	$(call INSTALL_VISTA_HSM_BIN,Release)
	# install 64bit driver and app
	$(call INSTALL_VS0564BIT_VCREDIST,free,$@/x64)
	$(call INSTALL_VS0864BIT_VCREDIST,free,$@/vs08/x64)
	$(call INSTALL_VISTA_BUILT_APPS,ReleaseUv/x64/hp,x64)
	$(call INSTALL_VISTA_BUILT_TTLS,x64/Release/hp,x64)
	$(call INSTALL_VISTA_HSM_BIN,Release/x64,x64)
	# install File Sharing icons
	$(call INSTALL_VISTA_FS_ICONS,$@/FshSvrAdmin)
	# install help
	$(call INSTALL_HELP_FILES,vista/hp)
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,hp,free,BCMAppInstaller)
	$(call INSTALL_MUIDLLS,hp)
	$(INSTALL) $(INS_DIR)/ini_vista/hp/free/external.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini_vista/hp/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/hp/Setup_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/Release/BcmSetupUtil.exe $@/
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/x64/Release/BcmSetupUtil.exe $@/x64
	$(INSTALL) $(VISTA_x86_PROTOCOLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_PROTOCOLDRIVER) $@/x64
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	# install WinPcap driver and dlls
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/npf.sys $@/
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/Packet.dll $@/
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/npf.sys $@/x64
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/Packet.dll $@/x64
	# Create peernet installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call INSTALL_PEERNET_INSTALLER,$@))
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
#vista>	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,HP_Win7_App))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/HP/HP_InstallShield/Disk1

# WinVista and Win7
$(RELEASEDIR)/Win7/HP/HP_InstallShield_Driver/Disk1 \
$(RELEASEDIR)/WinALL/HP/HP_InstallShield_Driver/Disk1/Win7: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/hp/$(VISTA_WLINF),$@/Drivers/Win7/WL)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm,Drivers/Win7/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm,Drivers/Win7/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/Win7/WL)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	$(call INSTALL_VISTA_REDIST_IHV)
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,hp,free,BCMAppInstaller)
	$(call INSTALL_MUIDLLS,hp)
	$(INSTALL) $(INS_DIR)/ini_vista/hp/free/driver.ini $@/bcmwls.ini
	# Need this to support -justdriver switch
	$(INSTALL) $(INS_DIR)/ini_vista/hp/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/hp/Setupd_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	# Create peernet installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call INSTALL_PEERNET_INSTALLER,$@))
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
	# Create single installer (for non-WinALL packages)
	$(if $(findstring WinALL,$@),,$(call PACKAGE_RELEASE,HP_Win7_Driver))
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/HP/HP_InstallShield_Driver/Disk1
# WinPE
$(RELEASEDIR)/WinPE/Bcm: \
	$(RELEASEDIR)/WinPE/Bcm/Bcm_Apps

$(RELEASEDIR)/WinPE/Bcm/Bcm_Apps: FORCE
ifneq ($(BCM_MFGTEST),)
	@$(MARKSTART)
	mkdir -p $(RELEASEDIR)/WinPE
	$(INSTALL) src/wl/exe/windows/winpe/obj/winpe/checked/pet.exe $(RELEASEDIR)/WinPE/
	$(INSTALL) release/WinXP/Bcm/Bcm_DriverOnly/bcmwl5.sys $(RELEASEDIR)/WinPE/
	$(INSTALL) release/WinXP/Bcm/Bcm_DriverOnly/bcmwl5.inf $(RELEASEDIR)/WinPE/
	@$(MARKEND)
endif # BCM_MFGTEST

################################################################################
# Release the DELL branded files.
################################################################################

# WinXP/2K
$(RELEASEDIR)/WinXP/Dell: $(RELEASEDIR)/WinXP/Dell/Dell_Docs \
		    $(RELEASEDIR)/WinXP/DebugSymbols/Dell \
		    $(RELEASEDIR)/WinXP/Dell/Dell_DriverOnly \
		    $(RELEASEDIR)/WinXP/Dell/Dell_DriverOnly_PreWHQL \
		    $(RELEASEDIR)/WinXP/Dell/Dell_InstallShield/Disk1 \
		    $(RELEASEDIR)/WinXP/Dell/Dell_Tools

# WinXP/2K
# Dell docs.
$(RELEASEDIR)/WinXP/Dell/Dell_Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	@cp -pv $(BCMWLHLP_DIR)/xp/dell/$(BCMWLHLP_REV_xp_dell)/*.cab $@
	@$(MARKEND)

# WinXP/2K
$(RELEASEDIR)/WinXP/DebugSymbols/Dell: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call INSTALL_XP_APP_DEBUG_SYMBOLS,dell)
	$(INSTALL) src/wl/cpl/neptune/cpl/Release/dell/bcmwlcpl.pdb $@/
	@$(MARKEND)

# WinXP/2K
# Dell driver-only binaries - default to USA locale
$(RELEASEDIR)/WinXP/Dell/Dell_DriverOnly_PreWHQL \
$(RELEASEDIR)/WinXP/Dell/Dell_DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER) $@
	$(INSTALL) $(XP_x64_WLDRIVER) $@
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_INF,$(INF_DIR)/dell/$(XP_WLINF_COI),$@/$(XP_WLINF)), \
	     $(call INSTALL_INF,$(INF_DIR)/dell/$(XP_WLINF),$@/$(XP_WLINF)) \
	)
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_32BIT_BUILT_COINSTALLER,release))
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	# End of $(RELEASEDIR)/WinXP/Dell/Dell_DriverOnly

# WinXP/2K
# Common installshield for all three DELL regions (US, JPN and ROW)
$(RELEASEDIR)/WinXP/Dell/Dell_InstallShield/Disk1: FORCE $(PRE_RELEASE_DEPS_XP)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@/X64
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/WinXP/WL
	# install driver and its dependent files
	$(INSTALL) $(XP_x86_WLDRIVER) $@/Drivers/WinXP/WL
	$(INSTALL) $(XP_x64_WLDRIVER) $@/Drivers/WinXP/WL
	$(call INSTALL_INF,$(INF_DIR)/dell/bcmwl5.inf,$@/Drivers/WinXP/WL/$(XP_WLINF))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/Drivers/WinXP/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/WinXP/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/WinXP/WL)
	# install tray/gui app
	$(call INSTALL_BRCM_SUPPLICANT,Release/bcm1xsup.dll,$@/)
	$(call INSTALL_VENDORLIBS_DLLS,WLBCGCBPRO731.dll,$@)
	$(call INSTALL_VS0532BIT_VCREDIST,free,$@/)
	$(call INSTALL_XP32BIT_BUILT_APPS,Release/dell)
	$(call INSTALL_VENDORLIBS_DLLS,win64/WLBCGCBPRO731.dll,$@/x64)
	$(call INSTALL_VS0564BIT_VCREDIST,free,$@/x64)
	$(call INSTALL_XP64BIT_BUILT_APPS,x64/Release/dell,x64)
	# install help
	$(call INSTALL_HELP_FILES,xp/dell)
	$(INSTALL) -v $(BCMWLHLP_DIR)/xp/dell/smithmicro/$(BCMWLHLP_REV_xp_dellsm)/bcmwlhlp.cab $@/bcmwlhlpsm.cab
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/bcmwlhlpsm.cab)
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,dell,free,BCMAppInstaller)
	$(INSTALL) $(INS_DIR)/ini/dell/free/external.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini/dell/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/dell/Setup.ini $@/
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInfo.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInfo64.exe $@/
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/BcmCrypt.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInst.enc $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	# install misc and package full installer
	$(INSTALL) src/doc/READMECli.rtf $@/README.rtf
	$(call REMOVE_WARN_PARTIAL_BUILD)
	$(call PACKAGE_RELEASE,Dell_XP)
##	$(INSTALL) $(INS_DIR)/installshield/dell/DellInst.enc $(@D)
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/WinXP/Dell/Dell_InstallShield/Disk1

# WinXP/2K
$(RELEASEDIR)/WinXP/Dell/Dell_Tools: $(RELEASEDIR)/WinXP/Dell/Dell_Tools/Docs \
			       $(RELEASEDIR)/WinXP/Dell/Dell_Tools/DosTool \
			       $(RELEASEDIR)/WinXP/Dell/Dell_Tools/Preflib \
			       $(RELEASEDIR)/WinXP/Dell/Dell_Tools/Preflibcl \
			       $(RELEASEDIR)/WinXP/Dell/Dell_Tools/SmithMicro

# WinXP/2K
$(RELEASEDIR)/WinXP/Dell/Dell_Tools/Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	@$(MARKEND)

# WinXP/2K
$(RELEASEDIR)/WinXP/Dell/Dell_Tools/DosTool: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL_DOSTOOL)
	@$(MARKEND)

# WinXP/2K
$(RELEASEDIR)/WinXP/Dell/Dell_Tools/Preflib: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL) src/wl/cpl/preflib/wlconfig/package/ReadMe.txt $@/
	$(INSTALL) src/wl/cpl/preflib/wlconfig/package/wlconfig.exe $@/
	$(INSTALL) src/wl/cpl/preflib/wlconfig/package/source/preflib.h $@/
	$(INSTALL) src/wl/cpl/preflib/wlconfig/package/source/preflib.lib $@/
	$(INSTALL) src/wl/cpl/preflib/wlconfig/package/source/PreferredNetworkAPI.doc $@/
	@$(MARKEND)

# WinXP/2K
$(RELEASEDIR)/WinXP/Dell/Dell_Tools/SmithMicro: FORCE
	@$(MARKSTART)
	mkdir -p $@/include
	mkdir -p $@/lib/x64
	mkdir -p $@/samples/ex1
	# install smithmicro docs
	$(INSTALL) src/wl/cpl/preflib/PreferredNetworkAPI_SM.doc      $@/
	$(INSTALL) src/wl/cpl/lib/WlAdapter/docs/WlAdapterAPI.doc     $@/
	$(INSTALL) src/wl/cpl/preflib/wlconfig/package/ReadMe.txt     $@/
	# install smithmicro include files
	$(INSTALL) src/wl/cpl/preflib/PrefNWInt.h                     $@/include
	$(INSTALL) src/wl/cpl/preflib/preflib.h                       $@/include
	$(INSTALL) src/wl/cpl/include/traystatus.h                    $@/include
	$(INSTALL) src/wl/cpl/lib/WlAdapter/include/WlAdapterAPI.h    $@/include
	# install smithmicro library files
	$(INSTALL) src/wl/cpl/preflib/Release/preflib.lib             $@/lib
	$(INSTALL) src/wl/cpl/lib/WlAdapter/DLL/Release/bcmwlapi.lib  $@/lib
	$(INSTALL) src/wl/cpl/preflib/x64/Release/preflib.lib         $@/lib/x64
	$(INSTALL) src/wl/cpl/lib/WlAdapter/DLL/x64/Release/bcmwlapi.lib \
								   $@/lib/x64
	# install smithmicro sample build files
	$(INSTALL) src/wl/cpl/preflib/preflibcl/package/samples/BUILD.txt  \
							$@/samples/ex1
	$(INSTALL) src/wl/cpl/preflib/preflibcl/package/samples/ex1/ex1.cpp \
							$@/samples/ex1
	$(INSTALL) src/wl/cpl/preflib/preflibcl/package/samples/ex1/ex1.dsp \
							$@/samples/ex1
	$(INSTALL) src/wl/cpl/preflib/preflibcl/package/samples/ex1/ex1.dsw \
							$@/samples/ex1
	$(INSTALL) src/wl/cpl/preflib/preflibcl/package/samples/ex1/print.cpp \
							$@/samples/ex1
	@$(MARKEND)
#	# End of $(RELEASEDIR)/WinXP/Dell/Dell_Tools/SmithMicro

# -------------------------------------------------------------------------
# Dell WinVista and Win7 Specific
$(RELEASEDIR)/Win7/Dell:     $(RELEASEDIR)/Win7/Dell/Dell_DriverOnly \
			     $(RELEASEDIR)/Win7/DebugSymbols/Dell \
			     $(RELEASEDIR)/Win7/Dell/Dell_Docs \
			     $(RELEASEDIR)/Win7/Dell/Dell_InstallShield/Disk1 \
			     $(RELEASEDIR)/Win7/Dell/Dell_InstallShield_Driver/Disk1 \
			     $(RELEASEDIR)/Win7/Dell/Dell_Tools \
			     $(RELEASEDIR)/Win7/Dell/Dell_Peernet_SDK \
			     $(RELEASEDIR)/Win7/Dell/Dell_MaestroExt_SDK

# WinVista and Win7
$(RELEASEDIR)/Win7/Dell/Dell_DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -p $@
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@
	$(call INSTALL_INF,$(INF_DIR)/dell/bcmwl6.inf,$@/$(VISTA_WLINF))
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
	$(call INSTALL_VISTA_REDIST_IHV)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	@$(call RELEASE_SIGN_VISTA_DRIVER,$@/,X86)
	@$(call RELEASE_SIGN_VISTA_DRIVER,$@/,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	# End of $(RELEASEDIR)/WEin7/Dell/Dell_DriverOnly

# WinVista and Win7
$(RELEASEDIR)/Win7/DebugSymbols/Dell: FORCE
	@$(MARKSTART)
	mkdir -p $@/x64
	$(call INSTALL_VISTA_APP_DEBUG_SYMBOLS,ReleaseUv/dell)
	$(call INSTALL_VISTA_APP_DEBUG_SYMBOLS,ReleaseUv/x64/dell,x64)
	@$(MARKEND)

# WinVista and Win7
$(RELEASEDIR)/Win7/Dell/Dell_Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	@cp -pv $(BCMWLHLP_DIR)/vista/dell/$(BCMWLHLP_REV_vista_dell)/*.cab $@
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/*.cab)
	@$(MARKEND)

# Dell InstallShield for WinVista and Win7
# Common installshield for all three DELL regions (US, JPN and ROW)
$(RELEASEDIR)/Win7/Dell/Dell_InstallShield/Disk1: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@
	mkdir -p $@/x64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/dell/bcmwl6.inf,$@/Drivers/Win7/WL/$(VISTA_WLINF))
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm,Drivers/Win7/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm,Drivers/Win7/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/Win7/WL)
	@$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	@$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	$(call INSTALL_VISTA_REDIST_IHV)
	# install File Sharing icons
	$(call INSTALL_VISTA_FS_ICONS,$@/FshSvrAdmin)
	# install help
	$(call INSTALL_HELP_FILES,vista/dell)
	$(INSTALL) -v $(BCMWLHLP_DIR)/vista/dell/smithmicro/$(BCMWLHLP_REV_vista_dellsm)/bcmwlhlp.cab $@/bcmwlhlpsm.cab
	@$(SIGNTOOL_CMND_64) $$(cygpath -m $@/bcmwlhlpsm.cab)
	# Install tray/gui app
	$(call INSTALL_VS0532BIT_VCREDIST,free,$@)
	$(call INSTALL_VS0832BIT_VCREDIST,free,$@/vs08)
	$(call INSTALL_VISTA_BUILT_APPS,ReleaseUv/dell)
	$(call INSTALL_VISTA_BUILT_TTLS,Release/dell)
	$(call INSTALL_VISTA_HSM_BIN,Release)
	$(call INSTALL_VS0564BIT_VCREDIST,free,$@/x64)
	$(call INSTALL_VS0864BIT_VCREDIST,free,$@/vs08/x64)
	$(call INSTALL_VISTA_BUILT_APPS,ReleaseUv/x64/dell,x64)
	$(call INSTALL_VISTA_BUILT_TTLS,x64/Release/dell,x64)
	$(call INSTALL_VISTA_HSM_BIN,Release/x64,x64)
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,dell,free,BCMAppInstaller)
	$(call INSTALL_MUIDLLS,Dell)
	$(INSTALL) $(INS_DIR)/ini_vista/dell/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/ini_vista/dell/free/external.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/dell/Setup_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInfo.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInfo64.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/BcmCrypt.exe $@/
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	$(INSTALL) $(VISTA_x86_PROTOCOLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_PROTOCOLDRIVER) $@/x64
	# install WinPcap driver and dlls
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/npf.sys $@/
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/Packet.dll $@/
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/npf.sys $@/x64
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/Packet.dll $@/x64
	# install misc and package full installer
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/Release/BcmSetupUtil.exe $@/
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/x64/Release/BcmSetupUtil.exe $@/x64
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInst.enc $@/
	# Create peernet installer
	$(call INSTALL_PEERNET_INSTALLER,$@)
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
#vista>	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(INSTALL) src/doc/READMECli_Vista.rtf $@/README.rtf
	$(call REMOVE_WARN_PARTIAL_BUILD)
	$(call PACKAGE_RELEASE,Dell_Win7)
##	$(INSTALL) $(INS_DIR)/installshield/dell/DellInst.enc $(@D)
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/Dell/Dell_InstallShield/Disk1

# Dell InstallShield DriverOnly for WinVista and Win7
# Common installshield for all three DELL regions (US, JPN and ROW)
$(RELEASEDIR)/Win7/Dell/Dell_InstallShield_Driver/Disk1: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	@echo "============================================================"
	@date +"START: $@, %D %T"
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/dell/bcmwl6.inf,$@/Drivers/Win7/WL/$(VISTA_WLINF))
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm,Drivers/Win7/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm,Drivers/Win7/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/Win7/WL)
	@$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	@$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,dell,free,BCMAppInstaller)
	$(call INSTALL_BCM_MUIS,Dell)
	$(INSTALL) $(INS_DIR)/ini_vista/dell/free/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/ini_vista/dell/free/driver.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/dell/Setup_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/sysinfo/Release/SysInfo.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInfo.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInfo64.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/BcmCrypt.exe $@/
	$(INSTALL) $(INS_DIR)/installshield/dell/DellInst.enc $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
#vista>	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(INSTALL) src/doc/READMECli_Vista.rtf $@/README.rtf
	$(call REMOVE_WARN_PARTIAL_BUILD)
	$(call PACKAGE_RELEASE,Dell_Win7)
##	$(INSTALL) $(INS_DIR)/installshield/dell/DellInst.enc $(@D)
	@date +"END: $@, %D %T"
	@echo "============================================================"
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/Dell/Dell_InstallShield_Driver/Disk1

# WinVista and Win7
$(RELEASEDIR)/Win7/Dell/Dell_Tools: $(RELEASEDIR)/Win7/Dell/Dell_Tools/Docs $(RELEASEDIR)/Win7/Dell/Dell_Tools/DosTool

# WinVista and Win7
$(RELEASEDIR)/Win7/Dell/Dell_Tools/Docs: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL) src/doc/BCMLogo.gif $@/
	@$(MARKEND)
#vista>	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html

# WinVista and Win7
$(RELEASEDIR)/Win7/Dell/Dell_Tools/DosTool: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(INSTALL_DOSTOOL)
	@$(MARKEND)

################################################################################
# Release the APPLE branded files.
################################################################################

# WinXP/2K
$(RELEASEDIR)/WinXP/Apple: $(RELEASEDIR)/WinXP/Apple/Apple_DriverOnly \
			   $(RELEASEDIR)/WinXP/Apple/Apple_DriverOnly_PreWHQL

# WinXP/2K
$(RELEASEDIR)/WinXP/Apple/Apple_DriverOnly_PreWHQL \
$(RELEASEDIR)/WinXP/Apple/Apple_DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER) $@/
	$(INSTALL) $(XP_x64_WLDRIVER) $@/
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_INF,$(INF_DIR)/apple/$(XP_WLINF_COI),$@/$(XP_WLINF)),\
	     $(call INSTALL_INF,$(INF_DIR)/apple/$(XP_WLINF),$@/) \
	)
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_32BIT_BUILT_COINSTALLER,release))
	$(if $(findstring PreWHQL,$@),\
	     $(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release))
	$(call INSTALL_INF,$(INF_DIR)/apple/$(XP_WLINF),$@/)
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)

# WinVista and Win7
$(RELEASEDIR)/Win7/Apple: $(RELEASEDIR)/Win7/Apple/Apple_DriverOnly

# WinVista and Win7
$(RELEASEDIR)/Win7/Apple/Apple_DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/
	$(call INSTALL_INF,$(INF_DIR)/apple/$(VISTA_WLINF),$@/)
	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)

################################################################################
# Release the Nokia branded files.
################################################################################
$(RELEASEDIR)/WinXP/Nokia: $(RELEASEDIR)/WinXP/Nokia/Nokia_Test

# WinXP/2K
$(RELEASEDIR)/WinXP/Nokia/Nokia_Test: FORCE
ifneq ($(BCM_MFGTEST),)
	@$(MARKSTART)
	@mkdir -p $@
#	$(INSTALL) src/tools/mfgc/release/brcm_wlu.dll    $@/
#	$(INSTALL) src/tools/mfgc/release/nok_cli.exe     $@/
#	$(INSTALL) src/tools/mfgc/release/vee_wrapper.dll $@/
#	$(INSTALL) src/tools/mfgc/release/vee_wrapper.h   $@/
#	$(INSTALL) src/tools/mfgc/release/wlu_nokia.dll   $@/
#	$(INSTALL) src/tools/mfgc/nokia/cli/free/nok_cli.exe $@/
#	$(INSTALL) src/tools/mfgc/nokia/dll/free/nok_mfg.dll   $@/
#	mkdir -p tmp
#	$(INSTALL) src/tools/mfgc/nokia/include/vee_wrapper.h  tmp/ignore_me.h
#	sed -e "/EXPORT/s///" -e "/^#/s/^#.*$$//" tmp/ignore_me.h > tmp/vee_wrapper.h
#	$(INSTALL) tmp/vee_wrapper.h   $@/
#	rm -rf tmp
	@$(MARKEND)
endif # BCM_MFGTEST
#	End of $(RELEASEDIR)/WinXP/Nokia/Nokia_Test

################################################################################
# Release the Bcm Internal misc folder
################################################################################
# WinXP/2K
$(RELEASEDIR)/WinXP/Internal: $(RELEASEDIR)/WinXP/Internal/preflib \
			      $(RELEASEDIR)/WinXP/Internal/preflibcl
	@$(MARKSTART)
	mkdir -p $@
	$(call INSTALL_BRCM_SUPPLICANT,Debug/bcm1xsupd.dll,$@/)
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/wl/exe/windows/winxp/obj/external/checked/wl.exe $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/wlm/free/wlm.lib $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/wlm/free/wlm.dll $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(XP_WLINF),$@/)
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	# Copy over 32bit free driver accessory files
	$(INSTALL) $(XP_x86_WLDRIVER) $@/
	$(INSTALL) $(subst .sys,.pdb,$(XP_x86_WLDRIVER)) \
		$@/free_$(basename $(notdir $(XP_x86_WLDRIVER))).pdb
	$(INSTALL) $(subst .sys,.map,$(XP_x86_WLDRIVER)) \
		$@/free_$(basename $(notdir $(XP_x86_WLDRIVER))).map
	# Copy over 64bit free driver accessory files
	$(INSTALL) $(XP_x64_WLDRIVER) $@/
	$(INSTALL) $(subst .sys,.pdb,$(XP_x64_WLDRIVER)) \
		$@/free_$(basename $(notdir $(XP_x64_WLDRIVER))).pdb
	$(INSTALL) $(subst .sys,.map,$(XP_x64_WLDRIVER)) \
		$@/free_$(basename $(notdir $(XP_x64_WLDRIVER))).map
ifneq ($(BCM_MFGTEST),)
	# Copy over 32bit checked driver accessory files for test groups
	$(INSTALL) $(subst objfre,objchk,$(XP_x86_WLDRIVER)) \
		$@/checked_$(basename $(notdir $(XP_x86_WLDRIVER))).sys
	$(INSTALL) $(subst objfre,objchk,$(subst .sys,.pdb,$(XP_x86_WLDRIVER))) \
		$@/checked_$(basename $(notdir $(XP_x86_WLDRIVER))).pdb
	$(INSTALL) $(subst objfre,objchk,$(subst .sys,.map,$(XP_x86_WLDRIVER))) \
		$@/checked_$(basename $(notdir $(XP_x86_WLDRIVER))).map
	# Copy over 64bit checked driver accessory files for test groups
	$(INSTALL) $(subst objfre,objchk,$(XP_x64_WLDRIVER)) \
		$@/checked_$(basename $(notdir $(XP_x64_WLDRIVER))).sys
	$(INSTALL) $(subst objfre,objchk,$(subst .sys,.pdb,$(XP_x64_WLDRIVER))) \
		$@/checked_$(basename $(notdir $(XP_x64_WLDRIVER))).pdb
	$(INSTALL) $(subst objfre,objchk,$(subst .sys,.map,$(XP_x64_WLDRIVER))) \
		$@/checked_$(basename $(notdir $(XP_x64_WLDRIVER))).map
endif # BCM_MFGTEST
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	@$(MARKEND)
#	# End of $(RELEASEDIR)/WinXP/Internal

# WinXP/2K
$(RELEASEDIR)/WinXP/Internal/preflib: FORCE
ifeq ($(BUILD_TRAYAPP),true)
	@$(MARKSTART)
	mkdir -p $@
	(cd src/wl/cpl/preflib/wlconfig/package && $(FIND) . -print | cpio -pmvud $@)
	@$(MARKEND)
endif # BUILD_TRAYAPP

# WinXP/2K
$(RELEASEDIR)/WinXP/Internal/preflibcl $(RELEASEDIR)/WinXP/Dell/Dell_Tools/Preflibcl: FORCE
ifeq ($(BUILD_TRAYAPP),true)
	@$(MARKSTART)
	mkdir -p $@
	(cd src/wl/cpl/preflib/preflibcl/package && $(FIND) . -print | cpio -pmvud $@)
	@$(MARKEND)
endif # BUILD_TRAYAPP

# WinVista and Win7
#----------------------------------
# Internal release for WinVista and Win7
$(RELEASEDIR)/Win7/Internal: FORCE
	@$(MARKSTART)
	mkdir -p $@/x64
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/checked/wl.exe $@/
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	$(INSTALL) $(VISTA_x86_WLDRIVER) $@/
	$(INSTALL) $(subst .sys,.pdb,$(VISTA_x86_WLDRIVER)) $@/free_$(basename $(notdir $(VISTA_x86_WLDRIVER))).pdb
	$(INSTALL) $(subst .sys,.map,$(VISTA_x86_WLDRIVER)) $@/free_$(basename $(notdir $(VISTA_x86_WLDRIVER))).map
#	$(call INSTALL_VISTA32BIT_BUILT_IHV,release/bcm)
#	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
ifneq ($(BCM_MFGTEST),)
	$(INSTALL) $(subst objfre,objchk,$(VISTA_x86_WLDRIVER)) $@/checked_$(basename $(notdir $(VISTA_x86_WLDRIVER))).sys
	$(INSTALL) $(subst objfre,objchk,$(subst .sys,.pdb,$(VISTA_x86_WLDRIVER))) $@/checked_$(basename $(notdir $(VISTA_x86_WLDRIVER))).pdb
	$(INSTALL) $(subst objfre,objchk,$(subst .sys,.map,$(VISTA_x86_WLDRIVER))) $@/checked_$(basename $(notdir $(VISTA_x86_WLDRIVER))).map
endif # BCM_MFGTEST
	$(INSTALL) $(VISTA_x64_WLDRIVER) $@/
	$(INSTALL) $(subst .sys,.pdb,$(VISTA_x64_WLDRIVER)) $@/free_$(basename $(notdir $(VISTA_x64_WLDRIVER))).pdb
	$(INSTALL) $(subst .sys,.map,$(VISTA_x64_WLDRIVER)) $@/free_$(basename $(notdir $(VISTA_x64_WLDRIVER))).map
#	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/release/bcm)
#	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(VISTA_WLINF),$@/)
#vista>	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	@$(MARKEND)
#	# End of $(RELEASEDIR)/Win7/Internal

package_for_web:
	@# make self-extracting zip files of the installshield directories
	@# for use by the upgrade server.
	@$(MARKSTART)
	@if type pftwwiz.exe >/dev/null 2>&1; then \
	   for dir in Bcm/Bcm_InstallShield Dell/Dell_InstallShield ; do \
		cd $(RELEASEDIR)/$${dir}/; \
		cp $(BUILD_BASE)/src/tools/release/802.11.pfw $(BUILD_BASE)/802.11.pfw; \
		echo Packaging $${dir}; \
		pftwwiz.exe $(BUILD_BASE)/802.11.pfw -a -s; \
	   done \
	else \
	   echo pftwwiz.exe not found - not packaging release.; \
	fi
	@$(MARKEND)


build_status:
	@$(MARKSTART)
	@mkdir -p misc
	@rm -f misc/build_status.txt
	@echo  "## BUILD STATUS FOR TAG=$(if $(TAG),$(TAG),NIGHTLY) BRAND=$(BRAND)" >> misc/build_status.txt; \
	date  "+## GENERATED AT %Y/%m/%d %H:%M:%S" >> misc/build_status.txt; \
	for isdir in $(RLSPKG_DIRS_XP) $(RLSPKG_DIRS_WIN7) $(RLSPKG_DIRS_WINALL); do \
	    if [ -d "$${isdir}" ]; then \
	       if [ -f "$${isdir}/$(WARN_FILE)" ]; then \
		  echo "$${isdir} : FAILED"; \
		  echo "$${isdir} : FAILED" >> misc/build_status.txt; \
	       else \
		  echo "$${isdir} : PASSED"; \
		  echo "$${isdir} : PASSED" >> misc/build_status.txt; \
	       fi; \
	    fi; \
	done;
	@$(MARKEND)

#pragma runlocal
build_clean: release
	@$(MARKSTART)
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)

# Usage: "$(MAKE) -f win_external_wl.mk SHELL=/bin/bash syntax_check"
syntax_check:
	@echo "Checking Makefile syntax only"

.PHONY: build_tryapp build_trayapp_vista build_trayapp_vista build_driver copy_dongle_images mogrify pre_release release_xp build_include build_status build_end FORCE

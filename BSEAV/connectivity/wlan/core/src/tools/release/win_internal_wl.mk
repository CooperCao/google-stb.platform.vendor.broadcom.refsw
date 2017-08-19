#
# Build the windows wireless drivers and tools
#
# Contact: hnd-software-scm-list
#
# $Id$
#
# HOW THIS MAKEFILE WORKS
#
# The release consists of 6 stages:
#
# 1. Checkout source from SVN.
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

$(DEFAULT_TARGET): $(ALL_TARGET)

DATE          = $(shell date -I)
BUILD_BASE   := $(shell pwd)
RETRYCMD     ?= $(firstword \
                $(wildcard \
		     C:/tools/build/bcmretrycmd.exe \
                     $(WLAN_WINPFX)/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                ))
RELEASEDIR   := $(BUILD_BASE)/release
SHELL        := bash.exe
FIND         := /bin/find
INSTALL      := install -p
NULL         := /dev/null
MAKE_MODE    := unix
export BRAND ?= win_internal_wl
OVFILE        = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
export TTYPE := DBG
INS_DIR      := components/apps/windows/install/app
INF_DIR      := $(INS_DIR)/installshield/BCMAppInstaller/is_bin
BIN_DIR      := components/shared/resources/tools/bin
WARN_FILE    := _WARNING_PARTIAL_BUILD_DO_NOT_USE

export CCX_SDK_VER := 1.1.13
CCX_SDK_DIR        ?= $(WLAN_WINPFX)/projects/hnd/restrictedTools/CCX_SDK/$(CCX_SDK_VER)
CCX_MUI_DIR        ?= $(WLAN_WINPFX)/projects/hnd/software/work/HelpFiles/vista/Cisco_Plug-Ins/$(CCX_SDK_VER)
X86_WPCAP41_DIR	   := src/8021x/win32/bin/wpcap_4_1
X64_WPCAP41_DIR	   := src/8021x/win64/bin/wpcap_4_1
export WDK_VER     := 7600
WDK_DIR            := $(firstword $(wildcard c:/tools/msdev/$(WDK_VER)wdk d:/tools/msdev/$(WDK_VER)wdk $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(WDK_VER)wdk))
CHKINF             := $(WDK_DIR)/tools/Chkinf/chkinf.bat

# For bcm internal build only bcm oem/brand
export OEM_LIST := bcm

SIGN_OS             = $(if $(findstring WinXP,$@),WINXP,$(if $(findstring WinVista,$@),WINVISTA,$(if $(findstring Win7,$@),WIN7,UNKNOWN)))

# WinXP wl drivers and inf
XP_WDK_BUILDDIR      = buildxp
XP_WDK_OS_x86        = wxp
XP_WDK_OS_x64        = wnet
XP_x86_WLDRIVER_C    = src/wl/sys/wdm/$(XP_WDK_BUILDDIR)/objchk_$(XP_WDK_OS_x86)_x86/i386/bcmwl5.sys
XP_x64_WLDRIVER_C    = src/wl/sys/wdm/$(XP_WDK_BUILDDIR)/objchk_$(XP_WDK_OS_x64)_amd64/amd64/bcmwl564.sys
XP_WLINF             = bcmwl5.inf

# WinVista wl drivers and inf
VISTA_WDK_BUILDDIR   = $(if $(findstring win7,$@)$(findstring Win7,$@)$(findstring WIN7,$@),buildwin7,buildvista)
VISTA_WDK_OS         = $(if $(findstring win7,$@)$(findstring Win7,$@)$(findstring WIN7,$@),win7,wlh)
VISTA_x86_WLDRIVER_C = src/wl/sys/wdm/$(VISTA_WDK_BUILDDIR)/objchk_$(VISTA_WDK_OS)_x86/i386/bcmwl6.sys
VISTA_x64_WLDRIVER_C = src/wl/sys/wdm/$(VISTA_WDK_BUILDDIR)/objchk_$(VISTA_WDK_OS)_amd64/amd64/bcmwl664.sys
VISTA_WLINF          = bcmwl6.inf

X86_WLCAT            = bcm43xx.cat
X64_WLCAT            = bcm43xx64.cat

# Win vista wdf free dlls (checked dlls support only w2k, wxp, w2k3 and not vista and w2k8)
VISTA_x86_WDFDLL   := $(WDK_DIR)/redist/wdf/x86/WdfCoInstaller01009.dll
VISTA_x64_WDFDLL   := $(WDK_DIR)/redist/wdf/amd64/WdfCoInstaller01009.dll

# Win vista wl drivers
VISTA_x86_PROTOCOLDRIVER_C := src/epiprot/sys/objchk_wlh_x86/i386/bcm42rly.sys
VISTA_x64_PROTOCOLDRIVER_C := src/epiprot/sys/objchk_wlh_amd64/amd64/bcm42rly.sys
# NDIS virtual wireless driver
VIRTUAL_x86_DRIVER:= components/ndis/ndisvwl/sys/buildxp/objchk_wxp_x86/i386/bcmvwl32.sys
VIRTUAL_x64_DRIVER:= components/ndis/ndisvwl/sys/buildxp/objchk_wnet_amd64/amd64/bcmvwl64.sys

# Disable dotfuscation for internal builds
override DOTFUSCATE_APP := false
DOTFUSCATOR_DIR      ?= components/apps/windows/tray/winx/Obfuscator/Dotfuscator
export TRAY_POST_BUILD_DISABLED=1
export DOTFUSCATE_APP

PROJECT_CONFIGS_GENERIC_DEBUG       := 'Debug|win32'     'Debug|x64'
PROJECT_CONFIGS_XP_APP_DEBUG        := 'Debug|win32'     'Debug|x64'
PROJECT_CONFIGS_VISTA_TTLS_DEBUG    := 'Debug|win32'     'Debug|x64'
PROJECT_CONFIGS_VISTA_IHV_DEBUG     := 'Debug|win32'     'Debug|x64'
PROJECT_CONFIGS_VISTA_DEBUG         := 'Debugv|win32'    'Debugv|x64'
PROJECT_CONFIGS_VISTA_UNICODE_DEBUG := 'DebugUv|win32'   'DebugUv|x64'
PROJECT_CONFIGS_VISTA_APP_DEBUG     := $(PROJECT_CONFIGS_VISTA_DEBUG)
PROJECT_CONFIGS_VISTA_APP_DEBUG     += $(PROJECT_CONFIGS_VISTA_UNICODE_DEBUG)

# Following pre_release dependencies are built differently
PRE_RELEASE_DEPS_COMMON  := pre_release

PRE_RELEASE_DEPS_XP      := $(PRE_RELEASE_DEPS_COMMON)
PRE_RELEASE_DEPS_XP      += build_install

PRE_RELEASE_DEPS_VISTA   := $(PRE_RELEASE_DEPS_COMMON)
PRE_RELEASE_DEPS_VISTA   += build_install
PRE_RELEASE_DEPS_VISTA   += build_trayapp_vista_dotfuscate

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

HNDSVN_BOM := wl-build.sparse

export MOGRIFY_RULES     = mogrify_common_rules.mk
export BRAND_RULES       = brand_common_rules.mk

# These symbols will be UNDEFINED in the source code by the transmogirifier
#
UNDEFS= CONFIG_BRCM_VJ CONFIG_BCRM_93725 CONFIG_BCM93725_VJ \
        CONFIG_BCM93725 BCM4413_CHOPS BCM42XX_CHOPS BCM33XX_CHOPS \
        CONFIG_BCM93725 BCM93725B BCM94100 BCM_USB NICMODE ETSIM \
        INTEL NETGEAR COMS BCM93352 BCM93350 BCM93310 PSOS __klsi__ \
        VX_BSD4_3 ROUTER BCMENET BCM4710A0 CONFIG_BCM4710 \
        CONFIG_MIPS_BRCM POCKET_PC DSLCPE LINUXSIM BCMSIM \
        BCMSIMUNIFIED WLNINTENDO WLNINTENDO2 WLNOKIA

# These symbols will be DEFINED in the source code by the transmogirifier
#
DEFS=BCM47XX BCM47XX_CHOPS BCMINTERNAL

# Mogrifier step pre-requisites
export MOGRIFY_FLAGS     = $(UNDEFS:%=-U%) $(DEFS:%=-D%)
export MOGRIFY_FILETYPES =
export MOGRIFY_EXCLUDE   =

# Place a warning file to indicate build failed
define WARN_PARTIAL_BUILD
	touch $@/$(WARN_FILE)
endef # WARN_PARTIAL_BUILD

# Remove warning file to indicate build step completed successfully
define REMOVE_WARN_PARTIAL_BUILD
	rm -f $@/$(WARN_FILE)
endef # REMOVE_WARN_PARTIAL_BUILD

# $1 is <Release or Debug>; $2 is destination
define INSTALL_BRCM_SUPPLICANT
	@echo "#- $0"
	$(INSTALL) src/8021x/win32/bin/bcmwlnpf.sys  $2
	$(INSTALL) src/8021x/win32/bin/bcmwlpkt.dll  $2
	$(INSTALL) src/8021x/win64/bin/Bcmnpf64.sys  $2
	$(INSTALL) src/8021x/xsupplicant/bcm1xsup/$1 $2
endef

# Dependent library for xp tray/gui
define INSTALL_VENDORLIBS_DLLS
	@echo "#- $0"
	$(INSTALL) src/vendorlibs/BCGSoft/BCGControlBarPro/v731/libs/VS2005/$1 $2
endef  # INSTALL_VENDORLIBS_DLLS

# $1 is <free or checked>; $2 is destination
define INSTALL_VS0532BIT_VCREDIST_DEBUG
	@echo "#- $0"
	$(INSTALL) $(BIN_DIR)/vs05/$1/Microsoft_VC80_DebugCRT_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/Microsoft_VC80_DebugMFC_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/policy_8_0_Microsoft_VC80_DebugCRT_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/policy_8_0_Microsoft_VC80_DebugMFC_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/vcredist_x86.bat $2
endef

# $1 is <free or checked>; $2 is destination
define INSTALL_VS0564BIT_VCREDIST_DEBUG
	@echo "#- $0"
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/Microsoft_VC80_DebugCRT_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/Microsoft_VC80_DebugMFC_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/policy_8_0_Microsoft_VC80_DebugCRT_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/policy_8_0_Microsoft_VC80_DebugMFC_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs05/$1/x64/vcredist_x64.bat $2
endef

# $1 is <free or checked>; $2 is destination
define INSTALL_VS0832BIT_VCREDIST_DEBUG
	@echo "#- $0"
	$(INSTALL) -d $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/Microsoft_VC90_DebugCRT_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/Microsoft_VC90_DebugMFC_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/policy_9_0_Microsoft_VC90_DebugCRT_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/policy_9_0_Microsoft_VC90_DebugMFC_x86.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/vcredist_x86.bat $2
endef

# $1 is <free or checked>; $2 is destination
define INSTALL_VS0864BIT_VCREDIST_DEBUG
	@echo "#- $0"
	$(INSTALL) -d $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/Microsoft_VC90_DebugCRT_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/Microsoft_VC90_DebugMFC_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/policy_9_0_Microsoft_VC90_DebugCRT_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/policy_9_0_Microsoft_VC90_DebugMFC_x86_x64.msi $2
	$(INSTALL) $(BIN_DIR)/vs08/$1/x64/vcredist_x64.bat $2
endef

# WinVista and Win7 original built files (non-dotfuscated)
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_VISTA_APP_ORIGINAL_OBJECTS
	@echo "#- $0"
	$(INSTALL) components/apps/windows/tray/winx/Remoting/$1/bcmwlrmt.dll        		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/Peernet/$1/bcmpeerapi.dll       		$@/$2
	$(INSTALL) components/apps/windows/tray/shared/BCMLogon/BCMLogon/$1/BCMLogon.dll    $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/$1/bcmwlcpl.cpl    		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/$1/bcmwlcpl.pdb    		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/$1/bcmwltry.exe       		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/$1/bcmwltry.pdb     			$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/$1/wltray.exe         		$@/$2
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/$1/wltray.pdb         		$@/$2
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/$(dir $1)bcmwlapiu.dll 	$@/$2
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/$(dir $(subst DebugUv,Debugv,$1))bcmwlapi.dll \
								    $@/$2
endef # INSTALL_VISTA_APP_ORIGINAL_OBJECTS


# WinVista and Win7 dotfuscated files
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_VISTA_APP_DOTFUSCATED_OBJECTS
	@echo "#- $0"
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwlrmt.dll    $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmpeerapi.dll  $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/BCMLogon.dll    $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwlcpl.cpl    $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwltry.exe    $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/wltray.exe      $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst /,_,$1)/bcmwlapiu.dll   $@/$2
	$(INSTALL) $(DOTFUSCATOR_DIR)/$(subst DebugUv,Debugv,$(subst /,_,$1))/bcmwlapi.dll    $@/$2
endef # INSTALL_VISTA_APP_DOTFUSCATED_OBJECTS

ifeq ($(strip $(DOTFUSCATE_APP)),true)
define INSTALL_VISTA_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_VISTA_APP_DOTFUSCATED_OBJECTS,$1,$2)
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/$1/wltrysvc.exe   $@/$2
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/$1/WLTryNT.dll     $@/$2
endef
else # NO DOTFUSCATE_APP
define INSTALL_VISTA_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_VISTA_APP_ORIGINAL_OBJECTS,$1,$2)
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/$1/wltrysvc.exe   $@/$2
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/$1/WLTryNT.dll     $@/$2
endef
endif # DOTFUSCATE_APP

# WinXP original built files
# $1 is <project_config>/<oem>; $2 is option <platformname>
define INSTALL_XP32BIT_APP_ORIGINAL_OBJECTS
	@echo "#- $0"
	$(INSTALL) src/wl/cpl/TrayApp/$1/bcmwltry.exe              $@/$2
	$(INSTALL) src/wl/cpl/TrayApp/$1/bcmwltry.pdb              $@/$2
	$(INSTALL) src/components/BCMLogon/$1/BCMLogon.dll         $@/$2
	$(INSTALL) src/components/BCMLogon/$1/BCMLogon.pdb         $@/$2
	$(INSTALL) src/wl/cpl/preflib/$(dir $1)preflib.dll         $@/$2
	$(INSTALL) src/wl/cpl/wltray/$1/wltray.exe                 $@/$2
	$(INSTALL) src/wl/cpl/wltray/$1/wltray.pdb                 $@/$2
	$(INSTALL) src/wl/cpl/wltrynt/$1/wltrynt.dll               $@/$2
	$(INSTALL) src/wl/cpl/neptune/cpl/$1/bcmwlcpl.cpl          $@/$2
	$(INSTALL) src/wl/cpl/neptune/cpl/$1/bcmwlcpl.pdb          $@/$2
	$(INSTALL) src/wl/cpl/wltrysvc/$1/wltrysvc.exe             $@/$2
endef  # INSTALL_XP32BIT_APP_ORIGINAL_OBJECTS

define INSTALL_XP64BIT_APP_ORIGINAL_OBJECTS
	@echo "#- $0"
	$(INSTALL) src/components/BCMLogon/$1/BCMLogon.dll         $@/$2
	$(INSTALL) src/components/BCMLogon/$1/BCMLogon.pdb         $@/$2
	$(INSTALL) src/wl/cpl/preflib/$(dir $1)preflib.dll         $@/$2
	$(INSTALL) src/wl/cpl/neptune/cpl/$1/bcmwlcpl.cpl          $@/$2
	$(INSTALL) src/wl/cpl/neptune/cpl/$1/bcmwlcpl.pdb          $@/$2
endef  # INSTALL_XP64BIT_APP_ORIGINAL_OBJECTS

define INSTALL_XP32BIT_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_XP32BIT_APP_ORIGINAL_OBJECTS,$1,$2)
endef
define INSTALL_XP64BIT_BUILT_APPS
	@echo "#- $0"
	$(call INSTALL_XP64BIT_APP_ORIGINAL_OBJECTS,$1,$2)
endef

define INSTALL_VISTA_BUILT_TTLS
	@echo "#- $0"
	$(INSTALL) components/apps/windows/ihvfrm/eaphost/ttls/$1/bcmttls.dll $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/eaphost/ttls/$1/bcmttls.pdb $@/$2
endef # INSTALL_VISTA_BUILT_TTLS

define INSTALL_VISTA32BIT_BUILT_IHV
	@echo "#- $0"
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvui.dll  $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvui.pdb  $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvsrv.dll $@/$2
	-$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvsrv.pdb $@/$2
endef # INSTALL_VISTA32BIT_BUILT_IHV

define INSTALL_VISTA64BIT_BUILT_IHV
	@echo "#- $0"
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvui64.dll  $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvui64.pdb  $@/$2
	$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvsrv64.dll $@/$2
	-$(INSTALL) components/apps/windows/ihvfrm/ihv/$1/bcmihvsrv64.pdb $@/$2
endef # INSTALL_VISTA64BIT_BUILT_IHV

define INSTALL_VISTA_REDIST_IHV
	@echo "#- $0"
	mkdir -pv $@/EAP_Plugin_Installer
#disabled#	$(INSTALL) src/wps/wpsapi/win32/WinPcap_4_0_2.exe 	$@/EAP_Plugin_Installer
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
define INSTALL_32BIT_BUILT_COINSTALLER
	@echo "#- $0"
	$(INSTALL) $(dir $(INS_DIR))Co-Installer/bcmwlcoi/$1/bcmwlcoi.dll   $@/$2
	$(SIGNTOOL_CMND_32) $$(cygpath -m $@/$2/bcmwlcoi.dll)
endef # INSTALL_32BIT_BUILT_COINSTALLER

define INSTALL_64BIT_BUILT_COINSTALLER
	@echo "#- $0"
	$(INSTALL) $(dir $(INS_DIR))Co-Installer/bcmwlcoi/$1/bcmwlcoi64.dll $@/$2
	$(SIGNTOOL_CMND_64) $$(cygpath -m $@/$2/bcmwlcoi64.dll)
endef # INSTALL_64BIT_BUILT_COINSTALLER

# Copy over unsigned driver catalog files
# Arg listing: $1=DEST_DIR
define INSTALL_UNSIGNED_DRIVER_CAT
	@echo "#- $0"
	$(INSTALL) src/wl/sys/wdm/bcm43xx.cat $1/$(X86_WLCAT)
	$(INSTALL) src/wl/sys/wdm/bcm43xx.cat $1/$(X64_WLCAT)
endef # INSTALL_UNSIGNED_DRIVER_CAT

# Arg listing: $1=Driver-folder $2=CPU $3=catalog-file $4=driver-name
# Arg listing: $3 and $4 are optional and are derived from $2
define RELEASE_SIGN_VISTA_DRIVER
	@echo "#- $0"
	$(MAKE) -C src/wl/sys/wdm release_sign_driver \
		SIGN_DIR=$1 SIGN_ARCH=$2 RELNUM="$(RELNUM)" \
		WINOS=$(SIGN_OS)
endef

# Arg listing: $1=Virtual-Driver-folder $2=CPU $3=os-type
define RELEASE_SIGN_VWL_DRIVER
	@echo "#- $0"
	$(MAKE) -C components/ndis/ndisvwl/sys release_sign_driver \
		SIGN_DIR=$1 SIGN_ARCH=$2 RELNUM="$(RELNUM)" \
		$(if $3,WINOS=$3)
endef # RELEASE_SIGN_VWL_DRIVER

# Install ndis virtual wl driver bits
# Arg listing: $1=dest_dir, $2=os-type (WINXP OR WINVISTA OR WIN7)
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
	# For WinVista/Win7 create softap/peernet installer
	# next to setup.exe
	@if [ "$(findstring /Win7/,$1)" != "" ]; then \
		if [ "$(findstring InstallShield,$1)" != "" ]; then \
			echo "Creating softap installer batchfile"; \
			echo "Setup.exe /peernet" > \
				$(subst /VWL,,$1)/Setup_PeerNet.bat; \
			chmod +x $(subst /VWL,,$1)/Setup_PeerNet.bat; \
		fi; \
	fi
endef # INSTALL_VWL_DRIVER

# $1 is <oem> $2 is <free or checked>
define  INSTALL_BCMWLS
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/bcmwls/Release/bcmwls32.exe  $@/
	$(INSTALL) $(INS_DIR)/bcmwls/x64/Release/bcmwls64.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst32.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst64.exe $@/
endef # INSTALL_BCMWLS

# $1 is <oem>
define 	INSTALL_MUIDLLS
	@echo "#- $0"
	mkdir -p $@/MUI
	$(INSTALL) $(INS_DIR)/MUI/ln/Release/$1/en-US/bcmwlrc.dll $@/MUI
	cd $(INS_DIR)/MUI/build/$1 && cp -prv --parents */bcmwlrc.dll.mui $@/MUI
	cd $(CCX_MUI_DIR) && $(FIND) . -print | cpio -pmud $@/MUI
endef # INSTALL_MUIDLLS

# $1 is <oem>; $2 is <free or checked>
define INSTALL_INSTALLSHIELD_FILES
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/data1.cab $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/data1.hdr $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/data2.cab $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/ISSetup.dll $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/_Setup.dll $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/layout.bin $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/setup.inx $@/
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/$1/Setup.exe $@/IS.exe
	$(INSTALL) $(INS_DIR)/installshield/$1/setup.iss $@/Setup.iss
	$(INSTALL) $(INS_DIR)/launcher/Release/Launcher.exe        $@/Setup.exe
	$(INSTALL) $(INS_DIR)/launcher/Launcher.ini          	  $@/
	$(INSTALL) $(INS_DIR)/uninstall/Debug/$1/bcmwlu00.exe $@/
	$(call INSTALL_BCMWLS,$1,$2)
endef # INSTALL_INSTALLSHIELD_FILES

# Run check inf tool
define CHECK_INF
	@echo "#- $0"
	cd $1 && perl $(BUILD_BASE)/src/tools/inftools/check_inf.pl -inf $2 -chkinf $(CHKINF)
endef  # CHECK_INF

# Copy and timestamp inf files
define INSTALL_INF
	@echo "#- $0"
	$(INSTALL) $1 $2
	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d $(RELDATE) "`cygpath -w $2`"
endef  # INSTALL_INF

#all: build_start mogrify pre_release build_driver build_apps build_trayapp build_install release release_apps build_infstamp build_end
all: build_start mogrify pre_release build_driver build_install release build_infstamp build_end

#disabled# include $(PARENT_MAKEFILE)

include $(MOGRIFY_RULES)
include $(BRAND_RULES)
include unreleased-chiplist.mk

build_start:
	@$(MARKSTART_BRAND)

# All exported MOGRIFY_xxx variables are referenced by following steps

mogrify:
	@$(MARKSTART)
#	$(MAKE) -f $(MOGRIFY_RULES)
	@$(MARKEND)

build_include: mogrify
	@$(MARKSTART)
	$(MAKE) -C src/include
	@$(MARKEND)

build_trayapp: build_trayapp_vista_dotfuscate build_trayapp_vista

build_trayapp_vista_dotfuscate: mogrify build_include
	@$(MARKSTART)
	$(MAKE) -C components/apps/windows/tray/winx \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_APP_DEBUG)"
	$(MAKE) -C components/apps/windows/tray/winx/Tools/WLVTT \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_DEBUG)"
ifeq ($(strip $(DOTFUSCATE_APP)),true)
	$(MAKE) -C components/apps/windows/tray/winx/Obfuscator/Dotfuscator \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_UNICODE_DEBUG)"
	$(MAKE) -C components/apps/windows/tray/winx/Obfuscator/Dotfuscator \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_DEBUG)" \
		   WLVISTA_TEMPLATE=WLVista_File.template \
		   SIGN_APP=false
	$(MAKE) -C components/apps/windows/tray/winx/Tools/WLVTT/Dotfuscator \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_DEBUG)"
else # DOTFUSCATE_APP
	@echo "WARN: Dotfuscation $(DOTFUSCATOR_DIR) not found or"
	@echo "WARN: DOTFUSCATE_APP has been set to false"
endif # DOTFUSCATE_APP
	@$(MARKEND)

build_trayapp_vista: mogrify build_include
	@$(MARKSTART)
	$(MAKE) -C src/epiprot BUILD_TYPES=checked
	$(MAKE) -C components/apps/windows/ihvfrm/eaphost/ttls \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_TTLS_DEBUG)"
	$(MAKE) -C components/apps/windows/ihvfrm/ihv \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV_DEBUG)"
	$(MAKE) -C $(dir $(INS_DIR))Co-Installer/bcmwlcoi \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV_DEBUG)"
	$(MAKE) -C $(dir $(INS_DIR))BcmSetupUtil \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_XP_APP_DEBUG)"
	$(MAKE) -C $(INS_DIR)/MUI \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV_DEBUG)"
	@$(MARKEND)

build_trayapp_xp2k: mogrify build_include
	@$(MARKSTART)
	@echo " -- MARK build trayapp --"
	$(MAKE) -C src/epiprot BUILD_TYPES=checked
	$(MAKE) -C src/wl/cpl \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_XP_APP_DEBUG)"
	$(MAKE) -C src/wl/cpl/preflib/wlconfig PROJCFG=Debug package
	$(MAKE) -C src/wl/cpl/preflib/preflibcl
	$(MAKE) -C src/wl/cpl/preflib/preflibcl/ex1
	$(MAKE) -C src/wl/cpl/preflib/preflibcl/prefdump
	$(MAKE) -C src/wl/cpl/preflib/preflibcl package
	@$(MARKEND)

build_apps: mogrify build_include
	@$(MARKSTART)
	@echo " -- MARK build apps --"
	$(MAKE) -C src/wl/exe/xp_server BUILD_TYPES=checked
	$(MAKE) -C src/wl/exe/vista_server BUILD_TYPES=checked
	@$(MARKEND)

build_driver: mogrify build_include
	@$(MARKSTART)
	@echo " -- MARK build driver --"
	$(MAKE) -C src/wl/sys/wdm BUILD_TYPES=checked \
		build_win7_driver
	$(MAKE) -C components/ndis/ndisvwl/sys BUILD_TYPES=checked
	$(MAKE) -C src/wl/exe
	@$(MARKEND)

ifeq ($(SKIP_BUILD_INSTALL),)
build_install: export VS_PRE_BUILD_ENABLED=0
build_install: mogrify
	@$(MARKSTART)
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND)
	@$(MARKEND)
else # SKIP_BUILD_INSTALL
build_install:
endif # SKIP_BUILD_INSTALL

clean:
	@$(MARKSTART)
	$(MAKE) -C src/include clean
	$(MAKE) -C $(INS_DIR) clean
	$(MAKE) -C src/wl/exe clean
	$(MAKE) -C src/wl/sys/wdm clean
	$(MAKE) -C components/ndis/ndisvwl/sys clean
	$(MAKE) -C components/apps/windows/tray/winx clean_all
	$(MAKE) -C components/apps/windows/ihvfrm/eaphost/ttls clean_all
	$(MAKE) -C components/apps/windows/ihvfrm/ihv clean_all
	$(MAKE) -C $(dir $(INS_DIR))Co-Installer/bcmwlcoi clean_all
	@$(MARKEND)

# For clustered builds, these release folders need to be pre-existing
pre_release:
	@$(MARKSTART)
	@[ -d release ] || mkdir -pv release
	# WinXP release dirs
	@[ -d release/WinXP/checked/Apps ] || \
		mkdir -pv release/WinXP/checked/Apps
#	@[ -d release/WinXP/checked/InstallShield ] || \
#		mkdir -pv release/WinXP/checked/InstallShield
#	@[ -d release/WinXP/checked/DriverOnly ] || \
#		mkdir -pv release/WinXP/checked/DriverOnly
	# WinVista and Win7 release dirs
	@[ -d release/Win7/checked/InstallShield ] || \
		mkdir -pv release/Win7/checked/InstallShield
	@[ -d release/Win7/checked/DriverOnly ] || \
		mkdir -pv release/Win7/checked/DriverOnly
	@$(MARKEND)

##############################################################################
# release xp and vista apps for servers
##############################################################################
release_apps: pre_release
	@$(MARKSTART)
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe  \
	$(RELEASEDIR)/WinXP/checked/Apps/
	$(INSTALL) src/wl/exe/xp_server/obj/server/dongle/checked/wl_server_dongle.exe $(RELEASEDIR)/WinXP/checked/Apps/
	$(INSTALL) src/wl/exe/xp_server/obj/server/socket/checked/wl_server_socket.exe $(RELEASEDIR)/WinXP/checked/Apps/
	$(INSTALL) src/wl/exe/xp_server/obj/server/802.11/checked/wl_server_wifi.exe $(RELEASEDIR)/WinXP/checked/Apps/
	@$(MARKEND)

release: release_win7

##############################################################################
# release_xp for WinXP/2K
##############################################################################
release_xp: \
	 $(RELEASEDIR)/WinXP/checked/DriverOnly \
	 $(RELEASEDIR)/WinXP/checked/DriverOnly_VWL
#	 $(RELEASEDIR)/WinXP/checked/InstallShield

# WinXP/2K - internal debug installshield
$(RELEASEDIR)/WinXP/checked/InstallShield: FORCE $(PRE_RELEASE_DEPS_XP)
	@$(MARKSTART)
	mkdir -p $@/X64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/WinXP/WL
	# install driver
	$(INSTALL) $(XP_x86_WLDRIVER_C) $@/Drivers/WinXP/WL
	$(INSTALL) $(XP_x64_WLDRIVER_C) $@/Drivers/WinXP/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl5,bcmwl5i,$(XP_WLINF)),$@/Drivers/WinXP/WL/$(XP_WLINF))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/Drivers/WinXP/WL)
#TODO#	$(call INSTALL_32BIT_BUILT_COINSTALLER,release,Drivers/WinXP/WL)
#TODO#	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release,Drivers/WinXP/WL)
	# install tray/gui app
	$(call INSTALL_BRCM_SUPPLICANT,Debug/bcm1xsupd.dll,$@)
	$(call INSTALL_VENDORLIBS_DLLS,WLBCGCBPRO731d.dll,$@/)
	$(call INSTALL_VS0532BIT_VCREDIST_DEBUG,checked,$@/)
	$(call INSTALL_XP32BIT_BUILT_APPS,Debug/bcm)
	$(call INSTALL_VENDORLIBS_DLLS,win64/WLBCGCBPRO731d.dll,$@/x64)
	$(call INSTALL_VS0564BIT_VCREDIST_DEBUG,checked,$@/x64)
	$(call INSTALL_XP64BIT_BUILT_APPS,x64/Debug/bcm,x64)
	# install help
	# install installer
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,checked)
	$(INSTALL) $(INS_DIR)/ini/bcm/checked/internal.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup.ini $@/
	unix2dos $@/Setup.ini
	# install tools
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe $(@D)/
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe $@/
	(cd src/wl/cpl/preflib/wlconfig/package && $(FIND) . -print | cpio -pmvud $(@D)/preflib)
	(cd src/wl/cpl/preflib/preflibcl/package && $(FIND) . -print | cpio -pmvud $(@D)/preflibcl)
	$(INSTALL) -D src/wl/cpl/preflib/preflibcl/prefdump/Debug/prefdump.exe $(@D)/preflibcl/bin/release/prefdump.exe
	# Check Bios info
	$(INSTALL) $(INS_DIR)/sysinfo/Debug/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(INSTALL) src/doc/BCMLogo.gif $(@D)/
	$(INSTALL) src/doc/mdc.jpg $(@D)/
	$(INSTALL) src/doc/ReleaseNotesWl.html $(@D)/ReleaseNotes.html
#	src/tools/release/wustamp -o -r -v "$(RELNUM)" -d `date +%m/%d/%Y` "`cygpath -w $@`"
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@date +"END: $@, %D %T"
	@echo "============================================================"
#	End of $(RELEASEDIR)/WinXP/checked/InstallShield
	@$(MARKEND)

# WinXP/2K - internal debug driver-only files
$(RELEASEDIR)/WinXP/checked/DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER_C) $@/
	$(INSTALL) $(XP_x64_WLDRIVER_C) $@/
	$(INSTALL) $(subst .sys,.pdb,$(XP_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.pdb,$(XP_x64_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(XP_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(XP_x64_WLDRIVER_C)) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl5,bcmwl5i,$(XP_WLINF)),$@/$(XP_WLINF))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe $@/
#TODO#	$(call INSTALL_32BIT_BUILT_COINSTALLER,release)
#TODO#	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/release)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	End of $(RELEASEDIR)/WinXP/checked/DriverOnly

# WinXP/2K
$(RELEASEDIR)/WinXP/checked/DriverOnly_VWL: FORCE
	@$(MARKSTART)
	$(call INSTALL_VWL_DRIVER,$@,WINXP)
	@$(MARKEND)

## WinVista and Win7
release_win7:  \
#		$(RELEASEDIR)/Win7/checked/InstallShield \
		$(RELEASEDIR)/Win7/checked/DriverOnly \
	 	$(RELEASEDIR)/Win7/checked/DriverOnly_VWL
#	 	$(RELEASEDIR)/Win7/checked/DriverOnly_IS

## WinVista and Win7 - internal debug installshield
$(RELEASEDIR)/Win7/checked/InstallShield: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	mkdir -p $@/X64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER_C) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER_C) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl6,bcmwl6i,$(VISTA_WLINF)),$@/Drivers/Win7/WL/$(VISTA_WLINF))
	$(call INSTALL_VISTA32BIT_BUILT_IHV,debug/bcm,Drivers/Win7/WL/)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/debug/bcm,Drivers/Win7/WL/)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,debug,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/debug,Drivers/Win7/WL)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	$(call INSTALL_VISTA_REDIST_IHV)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	$(INSTALL) $(VISTA_x86_PROTOCOLDRIVER_C) $@/
	$(INSTALL) $(VISTA_x64_PROTOCOLDRIVER_C) $@/x64
	# install WinPcap driver and dlls
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/npf.sys $@/
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/Packet.dll $@/
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/npf.sys $@/x64
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/Packet.dll $@/x64
	# install tray/gui app
	$(call INSTALL_VS0532BIT_VCREDIST_DEBUG,checked,$@)
	$(call INSTALL_VS0832BIT_VCREDIST_DEBUG,checked,$@/vs08)
	$(call INSTALL_VISTA_BUILT_APPS,DebugUv/bcm)
	$(call INSTALL_VISTA_BUILT_TTLS,Debug/bcm)
	$(call INSTALL_VISTA_HSM_BIN,Debug)
	$(call INSTALL_VS0564BIT_VCREDIST_DEBUG,checked,$@/x64)
	$(call INSTALL_VS0864BIT_VCREDIST_DEBUG,checked,$@/vs08/x64)
	$(call INSTALL_VISTA_BUILT_APPS,DebugUv/x64/bcm,x64)
	$(call INSTALL_VISTA_BUILT_TTLS,x64/Debug/bcm,x64)
	$(call INSTALL_VISTA_HSM_BIN,Debug/x64,x64)
	# install File Sharing icons
	$(call INSTALL_VISTA_FS_ICONS,$@/FshSvrAdmin)
	# install help
	# install installer
	## Install installshield files
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,checked)
	$(INSTALL) $(INS_DIR)/ini_vista/bcm/checked/internal.ini $@/bcmwls.ini
	$(call INSTALL_MUIDLLS,bcm)
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup_vista.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/Debug/BcmSetupUtil.exe $@/
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/x64/Debug/BcmSetupUtil.exe $@/x64
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	# install tools
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/checked/wl.exe $(@D)/
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/checked/wl.exe $@/
	# Check Bios info
	$(INSTALL) $(INS_DIR)/sysinfo/Debug/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/doc/mdc.jpg $@/
	$(INSTALL) src/doc/READMEBCMCli_Vista.rtf $@/README.rtf
	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	End of release/Win7/checked/InstallShield

## WinVista and Win7 - internal release driver-only
$(RELEASEDIR)/Win7/checked/DriverOnly: FORCE
	@$(MARKSTART)
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/checked/wl.exe $@/
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER_C) $@/
	$(INSTALL) $(VISTA_x64_WLDRIVER_C) $@/
	$(INSTALL) $(subst .sys,.pdb,$(VISTA_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.pdb,$(VISTA_x64_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(VISTA_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(VISTA_x64_WLDRIVER_C)) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl6,bcmwl6i,$(VISTA_WLINF)),$@/$(VISTA_WLINF))
	## Install utils
#	$(call INSTALL_VISTA32BIT_BUILT_IHV,debug/bcm)
#	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/debug/bcm)
#	$(call INSTALL_32BIT_BUILT_COINSTALLER,debug)
#	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/debug)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
#	End of $(RELEASEDIR)/Win7/checked/DriverOnly

# WinVista and Win7
$(RELEASEDIR)/Win7/checked/DriverOnly_VWL: FORCE
	@$(MARKSTART)
	$(call INSTALL_VWL_DRIVER,$@,WIN7)
	@$(MARKEND)

build_infstamp:
	@$(MARKSTART)
	# stamp all the infs
	@echo maj=$(maj)
	@echo min=$(min)
	@echo rcnum=$(rcnum)
	@echo incremental=$(incremental)
	src/tools/release/wustamp -a -o -r -v "$(maj).$(min).$(rcnum).$(incremental)" "`cygpath -w $(RELEASEDIR)`"
	@$(MARKEND)

# This package_for_web is deprecated and obsoleted since we have
# single installer from installshield
package_for_web:
	@# make self-extracting zip files of the installshield directories
	@# for use by the upgrade server.
	@$(MARKSTART)
	@if type pftwwiz.exe >/dev/null 2>&1; then \
	   for dir in checked/InstallShield; do \
		cd $(RELEASEDIR)/$${dir}/ && \
		cp $(BUILD_BASE)/src/tools/release/802.11.pfw $(BUILD_BASE)/802.11.pfw && \
		echo Packaging $${dir} && \
		pftwwiz.exe $(BUILD_BASE)/802.11.pfw -a -s; \
	   done \
	else \
	   echo pftwwiz.exe not found - not packaging release.; \
	fi
	@$(MARKEND)

$(RELEASEDIR)/% :
	@$(MARKSTART)
	mkdir -p $@
	@$(MARKEND)


#pragma runlocal
build_clean: release
	@$(MARKSTART)
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f
	@$(MARKEND)

build_end: build_clean
	@$(MARKEND_BRAND)	

# Usage: "$(MAKE) -f win_internal_wl.mk SHELL=/bin/bash syntax_check"
syntax_check:
	@echo "Checking Makefile syntax only"

.PHONY: src-top pre_release release_xp build_trayapp build_trayapp_vista build_trayapp_vista_dotfuscate build_driver mogrify build_install build_include FORCE

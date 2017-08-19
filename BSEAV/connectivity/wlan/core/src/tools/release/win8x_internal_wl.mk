#
# Build the windows wireless drivers and tools
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
# 3. Build binaries include both trayapp and driver
#
# 4. copy binary files to release folder
#
# Besides build target [all], there are two convenient targets [trayapp], [driver]
# to save time for testing build. They SHOULD NOT be released.

empty           :=
space           := $(empty) $(empty)
comma           := $(empty),$(empty)

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
BRAND              ?= win8x_internal_wl
RETRYCMD           ?= $(firstword \
                      $(wildcard \
                       Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe \
                   ))

export VS_VER      := 2013
VERSION_NAME       := Win8X
WIN8X_WDK_VERSION  ?= 9600
WIN8X_WDK_INSTALL   ?= $(WIN8X_WDK_VERSION)wdksdk
WIN8X_WDK_LOCAL     ?= c:/tools/msdev
WIN8X_WDK_GLOBAL    ?= z:/projects/hnd/tools/win/msdev
WIN8X_WDK_ROOT      := $(firstword \
                         $(wildcard $(WIN8X_WDK_LOCAL)/$(WIN8X_WDK_INSTALL) \
                                    $(WIN8X_WDK_GLOBAL)/$(WIN8X_WDK_INSTALL) ) \
                         Makefile-Did-Not-Find-$(WIN8X_WDK_INSTALL)-Error)

OVFILE        = $(if $(OVERRIDE),$(OVERRIDE)/tools/release/$@,)
export TTYPE := DBG
WARN_FILE    := _WARNING_PARTIAL_BUILD_DO_NOT_USE
export RETRYCMD := Z:/projects/hnd_software/gallery/src/tools/build/bcmretrycmd.exe

WIN8X_WDK_ROOTS :=c:/tools/msdev/$(WIN8X_WDK_VERSION)wdksdk
WIN8X_WDK_ROOT :=$(strip $(foreach r, $(WIN8X_WDK_ROOTS), $(if $(findstring $(WIN8X_WDK_VERSION), $r), $r)))

UPPER_VERSION_NAME = $(subst $(space),$(empty), $(call .toupper, $(VERSION_NAME)))
LOWER_VERSION_NAME = $(subst $(space),$(empty), $(call .tolower, $(VERSION_NAME)))

WINOS=$(UPPER_VERSION_NAME)

export CCX_SDK_VER := 1.2.4
CCX_SDK_DIR        ?= Z:/projects/hnd/restrictedTools/CCX_SDK/$(CCX_SDK_VER)
CCX_MUI_DIR        ?= Z:/projects/hnd/software/work/HelpFiles/vista/Cisco_Plug-Ins/$(CCX_SDK_VER)
X86_WPCAP41_DIR	   := src/8021x/win32/bin/wpcap_4_1
X64_WPCAP41_DIR	   := src/8021x/win64/bin/wpcap_4_1
export WDK_VER     := 7600
WDK_DIR            := $(firstword $(wildcard c:/tools/msdev/$(WDK_VER)wdk d:/tools/msdev/$(WDK_VER)wdk z:/projects/hnd/tools/win/msdev/$(WDK_VER)wdk))
CHKINF             := $(WDK_DIR)/tools/Chkinf/chkinf.bat

# For bcm internal build only bcm oem/brand
export OEM_LIST := bcm

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

# Win8X
WIN8X_x86_WDFDLL   := $(WIN8X_WDK_ROOT)/Redist/wdf/x86/WdfCoInstaller01011.dll
WIN8X_x64_WDFDLL   := $(WIN8X_WDK_ROOT)/Redist/wdf/x64/WdfCoInstaller01011.dll

# Win8X wl drivers and inf
WIN8X_x86_WLDRIVER_C = src/wl/sys/wdm/obj/win8x_nic/checked/x86/bcmwl63.sys
WIN8X_x64_WLDRIVER_C = src/wl/sys/wdm/obj/win8x_nic/checked/x64/bcmwl63a.sys
WIN8X_WLINF          = bcmwl63.inf

# Win vista wl drivers
VISTA_x86_PROTOCOLDRIVER := src/epiprot/sys/objchk_wlh_x86/i386/bcm42rly.sys
VISTA_x64_PROTOCOLDRIVER := src/epiprot/sys/objchk_wlh_amd64/amd64/bcm42rly.sys
# NDIS virtual wireless driver
VIRTUAL_x86_DRIVER:= components/ndis/ndisvwl/sys/buildxp/objchk_wxp_x86/i386/bcmvwl32.sys
VIRTUAL_x64_DRIVER:= components/ndis/ndisvwl/sys/buildxp/objchk_wnet_amd64/amd64/bcmvwl64.sys


# Disable dotfuscation for internal builds
override DOTFUSCATE_APP := false
DOTFUSCATOR_DIR      ?= components/apps/windows/tray/winx/Obfuscator/Dotfuscator
# Run dialog test on tray/gui app objects and report errors
DLGTEST_LOG          := src/tools/locale/dlgtest/rundlgtest_errors.log
DLGTEST_NOTIFY       ?= false
DLGTEST_NOTIFY_LIST  ?= hnd-software-windows-list@broadcom.com
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

SIGN_OS  ?=$(if $(findstring WinXP,$@),WINXP,$(if $(findstring WinVista,$@),WINVISTA,$(if $(findstring Win7,$@),WIN7,$(if $(findstring Win8X,$@),WIN8X,UNKNOWN))))

SIGNTOOL_TIME  := "http://timestamp.verisign.com/scripts/timstamp.dll"
SIGNTOOL_OPTS  ?= /a /v /s my /n "Broadcom Corporation" /t $(SIGNTOOL_TIME)
SIGNTOOL_CMND_32  := $(RETRYCMD) $(WIN8X_WDK_ROOT)/bin/x86/signtool sign $(SIGNTOOL_OPTS)
SIGNTOOL_CMND_64  := $(RETRYCMD) $(WIN8X_WDK_ROOT)/bin/x64/signtool sign $(SIGNTOOL_OPTS)
export SIGNTOOL_OPTS

INS_DIR        := components/apps/windows/install/app
INF_DIR        := $(INS_DIR)/installshield/BCMAppInstaller/is_bin
BIN_DIR        := components/shared/resources/tools/bin

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

# These symbols will be DEFINED in the source code by the transmogrifier
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
	$(INSTALL) components/apps/windows/tray/winx/Remoting/$1/bcmwlrmt.dll                 $@/$2
	$(INSTALL) components/apps/windows/tray/winx/Peernet/$1/bcmpeerapi.dll                $@/$2
	$(INSTALL) components/apps/windows/tray/shared/BCMLogon/BCMLogon/$1/BCMLogon.dll      $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/$1/bcmwlcpl.cpl             $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/$1/bcmwlcpl.pdb             $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/$1/bcmwltry.exe                $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/$1/bcmwltry.pdb                $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/$1/wltray.exe                  $@/$2
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/$1/wltray.pdb                  $@/$2
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/$(dir $1)bcmwlapiu.dll     $@/$2
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

define INSTALL_AUXAPP
	@echo "#- $0"
	@$(MARKSTART)
	# MARK-WIN8X-START copy trayapp files
	$(INSTALL) components/apps/windows/tray/winx/Peernet/Debugwb/$(1)/bcmpeerapi.{dll,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/winx/Peernet/Debugwb/x64/$(1)/bcmpeerapi.{dll,pdb,map} $@/x64
	$(INSTALL) components/apps/windows/tray/winx/Remoting/Debugwb/$(1)/bcmwlrmt.{dll,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/winx/Remoting/Debugwb/x64/$(1)/bcmwlrmt.{dll,pdb,map} $@/x64
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/Debugwb/$(1)/bcmwltry.{exe,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/winx/ServerApp/Debugwb/x64/$(1)/bcmwltry.{exe,pdb,map} $@/x64
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/Debugwb/$(1)/wltray.{exe,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/winx/ClientApp/Debugwb/x64/$(1)/wltray.{exe,pdb,map} $@/x64
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/Debugwb/$(1)/wltrysvc.{exe,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/shared/wltrysvc/Debugwb/x64/$(1)/wltrysvc.{exe,pdb,map} $@/x64
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/Debugwb/$(1)/WLTryNT.{dll,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/shared/wltrynt/Debugwb/x64/$(1)/WLTryNT.{dll,pdb,map} $@/x64
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/Debugwb/$(1)/bcmwlcpl.{cpl,pdb,map}    $@/
	$(INSTALL) components/apps/windows/tray/winx/ControlPanel/Debugwb/x64/$(1)/bcmwlcpl.{cpl,pdb,map}    $@/x64
	$(INSTALL) components/apps/windows/tray/shared/BCMLogon/BCMLogon/Debugwb/$(1)/BCMLogon.{dll,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/shared/BCMLogon/BCMLogon/Debugwb/$(1)/BCMLogon.{dll,pdb,map} $@/
	$(INSTALL) components/apps/windows/tray/shared/BCMLogon/BCMLogon/Debugwb/x64/$(1)/BCMLogon.{dll,pdb,map} $@/x64
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac_win8.exe $@/Inst2Gac.exe
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac_win8.exe $@/x64/Inst2Gac.exe
	# MARK-WIN8X-END copy trayapp files
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/Debugwb/bcmwlapiu.{dll,pdb,map} $@/
	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/Debugwb/x64/bcmwlapiu.{dll,pdb,map} $@/x64
	# Tools
	$(INSTALL) $(BIN_DIR)/vs13/checked/vcredist_x86.bat $@/vs13
	$(INSTALL) $(BIN_DIR)/vs13/checked/X64/vcredist_x64.bat $@/vs13/x64
	$(INSTALL) $(BIN_DIR)/vs13/checked/Microsoft_VC120_DebugCRT_x86.msi $@/vs13
	$(INSTALL) $(BIN_DIR)/vs13/checked/X64/Microsoft_VC120_DebugCRT_x64.msi $@/vs13/x64
	$(INSTALL) $(BIN_DIR)/vs13/checked/Microsoft_VC120_DebugMFC_x86.msi $@/vs13
	$(INSTALL) $(BIN_DIR)/vs13/checked/X64/Microsoft_VC120_DebugMFC_x64.msi $@/vs13/x64
	$(INSTALL) $(BIN_DIR)/vs13/checked/*.dll $@/vs13
	$(INSTALL) $(BIN_DIR)/vs13/checked/x64/*.dll $@/vs13/x64
	$(call FILES_COMPONENT_PROTOCOLDRIVER)
	$(call FILES_COMPONENT_PCAP)
	$(call NEW_INSTALL_VISTA_HSM_BIN,Debugwb)
	$(call INSTALL_PEERNET_INSTALLER,$@)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/$(UPPER_VERSION_NAME)/VWL,WIN7)
	@$(MARKEND)
endef # INSTALL_AUXAPP
define INSTALL_COMPONENT_SOFTAP
	$(call FILES_COMPONENT_SOFTAP)
endef

define FILES_COMPONENT_PROTOCOLDRIVER
	$(INSTALL) $(VISTA_x86_PROTOCOLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_PROTOCOLDRIVER) $@/x64
endef
define FILES_COMPONENT_PCAP
	# install WinPcap driver and dlls
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/npf.sys $@/
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/npf.sys $@/x64
	$(INSTALL) $(X86_WPCAP41_DIR)/Vista/Packet.dll $@/
	$(INSTALL) $(X64_WPCAP41_DIR)/Vista/Packet.dll $@/x64
endef
COMPONENT_SOFTAP_ROOT_PATH := src/wl/cpl/vista/lenovo/WPFLenovoSoftAP
define FILES_COMPONENT_SOFTAP
	$(INSTALL) -d $@/en-US
	$(INSTALL) -d $@/zh-Hans
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/win32/Debugwb/en-US/*      $@/en-US
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/win32/Debugwb/zh-Hans/*    $@/zh-Hans
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/win32/Debugwb/*.{dll,exe}  $@/

	$(INSTALL) -d $@/x64/en-US
	$(INSTALL) -d $@/x64/zh-Hans
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/x64/Debugwb/en-US/*     $@/x64/en-US
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/x64/Debugwb/zh-Hans/*   $@/x64/zh-Hans
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/x64/Debugwb/*.{dll,exe} $@/x64
	$(INSTALL) $(COMPONENT_SOFTAP_ROOT_PATH)/WPFSoftAPUI/Resources/SoftAP_App.ico $@/
endef
###---------BEGIN_SECTION_WINIEAPI---------###
COMPONENT_WINIEAPI_ROOT_PATH := components/apps/windows/libraries/WinIEapi
define INSTALL_COMPONENT_WINIEAPI
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapi/Debugwb/*.{dll,lib} $@/
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapp/Debugwb/bcmwlanapp.exe $@/
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapi/Debugwb/x64/*.{dll,lib} $@/x64
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapp/Debugwb/x64/bcmwlanapp.exe $@/x64
endef
define INSTALL_COMPONENT_WINIEAPI_DEBUGSYMBOLS
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapi/Debugwb/bcmwlanapi.pdb $@/
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapp/Debugwb/bcmwlanapp.pdb $@/
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapi/Debugwb/x64/bcmwlanapi.pdb $@/x64
	$(INSTALL) $(COMPONENT_WINIEAPI_ROOT_PATH)/bcmwlanapp/Debugwb/x64/bcmwlanapp.pdb $@/x64
endef
###---------END_SECTION_WINIEAPI---------###

#COMPONENT_MIRACAST_NATIVE_ROOT_PATH := src/wl/cpl/vista/WinIEapi/Miracast
#define INSTALL_COMPONENT_MIRACAST_NATIVE
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapi/Debugwb/*.{dll,lib} $@/
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapi/Debugwb/x64/*.{dll,lib} $@/x64
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapp/Debugwb/miracastnativeapp.exe $@/
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapp/Debugwb/x64/miracastnativeapp.exe $@/x64
#endef

#define INSTALL_COMPONENT_MIRACAST_NATIVE_DEBUGSYMBOLS
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapi/Debugwb/*.pdb $@/
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapi/Debugwb/x64/*.pdb $@/x64
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapp/Debugwb/*.pdb $@/
#	$(INSTALL) $(COMPONENT_MIRACAST_NATIVE_ROOT_PATH)/miracastnativeapp/Debugwb/x64/*.pdb $@/x64
#endef

define INSTALL_BLUEWIRE
	@echo "#- $0"
	$(INSTALL) src/wl/cpl/vista/Bluewire/$1/WFDNamespaceExt.dll $@/$2
	$(INSTALL) src/wl/cpl/vista/Bluewire/$1/WFDNamespaceExt.propdesc $@/$2
	$(INSTALL) src/wl/cpl/vista/Bluewire/$1/WFDSendToExplorer.exe $@/$2
	$(INSTALL) src/wl/cpl/vista/Bluewire/$1/WFDTray.exe         $@/$2
	$(INSTALL) src/wl/cpl/vista/Bluewire/$1/WifiDirectRez.dll   $@/$2
	mkdir -p $@/$2/MUI/en-US && $(INSTALL) src/wl/cpl/vista/Bluewire/$1/en-US/WifiDirectRez.dll.mui $@/$2/MUI/en-US
endef # INSTALL_BLUEWIRE

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
	$(INSTALL) src/wl/cpl/components/sup/eaphost/ttls/$1/bcmttls.dll $@/$2
	$(INSTALL) src/wl/cpl/components/sup/eaphost/ttls/$1/bcmttls.pdb $@/$2
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
#disabled#	$(INSTALL) src/wps/wpsapi/win32/WinPcap_4_0_2.exe         $@/$1
	$(INSTALL) $(BIN_DIR)/ihv/free/x86/Inst_EAPModules.bat      $@/$1
	$(INSTALL) $(BIN_DIR)/ihv/free/x86/Uninst_EAPModules.bat    $@/$1
	$(INSTALL) $(CCX_SDK_DIR)/EAP/EAP-Fast/EAP-FAST.msi         $@/$1
	$(INSTALL) $(CCX_SDK_DIR)/EAP/LEAP/EAP-LEAP.msi             $@/$1
	$(INSTALL) $(CCX_SDK_DIR)/EAP/PEAP/EAP-PEAP.msi             $@/$1
endef # INSTALL_VISTA_REDIST_IHV

define INSTALL_VISTA_HSM_BIN
	@echo "#- $0"
	$(INSTALL) src/apps/fsh/win32/bin/$1/bcmfshapi.dll		$@/$2
endef # INSTALL_VISTA_HSM_BIN

define NEW_INSTALL_VISTA_HSM_BIN
	@echo "#- $0"
	-$(INSTALL) src/apps/fsh/win32/bin/$1/bcmfshapi{.dll,.pdb}	$@/
	-$(INSTALL) src/apps/fsh/win32/bin/$1/x64/bcmfshapi{.dll,.pdb}	$@/x64
endef # NEW_INSTALL_VISTA_HSM_BIN

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
		WINOS=$(SIGN_OS) WIN8_WDK_VER=$(WIN8X_WDK_VERSION)
endef

define RELEASE_SIGN_WIN8X_DRIVER
	@echo "#- $0($1,$2,$3)"
	$(call RELEASE_SIGN_VISTA_DRIVER,$1,$2,$(SIGN_OS))
endef # RELEASE_SIGN_WIN8X_DRIVER

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
endef # INSTALL_VWL_DRIVER

define INSTALL_PEERNET_INSTALLER
        @echo "Creating softap installer batchfile"
        @echo "Setup.exe /peernet" > $(@D)/Setup_PeerNet.bat
        chmod +x $(@D)/Setup_PeerNet.bat
endef # INSTALL_PEERNET_INSTALLER

# $1 is <oem> $2 is <free or checked>
define INSTALL_BCMWLS
	@echo "#- $0"
	$(INSTALL) $(INS_DIR)/bcmwls/Release/bcmwls32.exe  $@/
	$(INSTALL) $(INS_DIR)/bcmwls/x64/Release/bcmwls64.exe $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst32.exe  $@/
	$(INSTALL) $(INS_DIR)/Utils/dpinst64.exe  $@/
endef # INSTALL_BCMWLS

# $1 is <oem>
define	INSTALL_MUIDLLS
	@echo "#- $0"
	mkdir -p $@/MUI
	$(INSTALL) $(INS_DIR)/MUI/ln/Release/$1/en-US/bcmwlrc.dll $@/MUI
	cd $(INS_DIR)/MUI/build/$1; cp -prv --parents */bcmwlrc.dll.mui $@/MUI
	cd $(CCX_MUI_DIR); $(FIND) . -print | cpio -pmud $@/MUI
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
	$(INSTALL) $(INS_DIR)/launcher/Release/Launcher.exe $@/Setup.exe
	$(INSTALL) $(INS_DIR)/launcher/Launcher.ini $@/
	$(INSTALL) $(INS_DIR)/uninstall/Debug/$1/bcmwlu00.exe $@/
	$(call INSTALL_BCMWLS,$1,$2)
endef # INSTALL_INSTALLSHIELD_FILES

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

#disabled# all: profile.log mogrify pre_release copy_dongle_images build_driver build_apps build_trayapp build_bluewire_win7 build_install release release_apps build_infstamp build_dlgtest build_end

all: profile.log mogrify pre_release build_driver build_install release build_infstamp build_end

#disabled# include $(PARENT_MAKEFILE)

include $(MOGRIFY_RULES)
include $(BRAND_RULES)
include unreleased-chiplist.mk

unexport VSINSTALLDIR

# All exported MOGRIFY_xxx variables are referenced by following steps

mogrify:
	$(MAKE) -f $(MOGRIFY_RULES)

build_include: mogrify
	@date +"START: $@, %D %T" | tee -a profile.log
	$(MAKE) -C src/include
	@date +"END:   $@, %D %T" | tee -a profile.log

build_trayapp: build_trayapp_vista_dotfuscate build_trayapp_vista build_trayapp_xp2k

build_trayapp_vista_dotfuscate: mogrify build_include
ifeq ($(strip $(DOTFUSCATE_APP)),true)
#disabled#		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_DEBUG)"
else # DOTFUSCATE_APP
#disabled#	@echo "WARN: Dotfuscation $(DOTFUSCATOR_DIR) not found or"
#disabled#	@echo "WARN: DOTFUSCATE_APP has been set to false"
#disabled#	$(MAKE) -C components/apps/windows/tray/winx/Obfuscator/Dotfuscator \
#disabled#		PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_UNICODE_DEBUG)"
endif # DOTFUSCATE_APP
#disabled#	@date +"END:   $@, %D %T" | tee -a profile.log

build_trayapp_vista: mogrify build_include
	@date +"START: $@, %D %T" | tee -a profile.log
#disabled#	$(MAKE) -C src/wl/cpl/components/sup/eaphost/ttls \
#disabled#		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_TTLS_DEBUG)"
	$(MAKE) -C components/apps/windows/ihvfrm/ihv -f build_vs13.mk \
	        BUILD_CONFS=Debugwb BUILD_ARCHS="x86 x64"
#disabled#	$(MAKE) -C $(dir $(INS_DIR))Co-Installer/bcmwlcoi \
#disabled#		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV_DEBUG)"
#disabled#	$(MAKE) -C $(dir $(INS_DIR))BcmSetupUtil \
#disabled#		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_XP_APP_DEBUG)"
	$(MAKE) -C $(INS_DIR)/MUI \
		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_VISTA_IHV_DEBUG)"
#disabled#	@date +"END:   $@, %D %T" | tee -a profile.log

build_trayapp_xp2k: mogrify build_include
#disabled#	@date +"START: $@, %D %T" | tee -a profile.log
#disabled#	@echo " -- MARK build trayapp --"
	$(MAKE) -C src/epiprot BUILD_TYPES=checked
#disabled#	$(MAKE) -C src/wl/cpl \
#disabled#		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_XP_APP_DEBUG)"
#disabled#	$(MAKE) -C src/wl/cpl/preflib/wlconfig PROJCFG=Debug package
#disabled#	$(MAKE) -C src/wl/cpl/preflib/preflibcl
#disabled#	$(MAKE) -C src/wl/cpl/preflib/preflibcl/ex1
#disabled#	$(MAKE) -C src/wl/cpl/preflib/preflibcl/prefdump
#disabled#	$(MAKE) -C src/wl/cpl/preflib/preflibcl package
#disabled#	@date +"END:   $@, %D %T" | tee -a profile.log

build_bluewire_win7: build_include
#disabled#	@date +"START: $@, %D %T" | tee -a profile.log
#disabled#	$(MAKE) -C src/wl/cpl/vista/Bluewire/WFD \
#disabled#		   PROJECT_CONFIGS="$(PROJECT_CONFIGS_GENERIC_DEBUG)"
#disabled#	@date +"END:   $@, %D %T" | tee -a profile.log

build_apps: mogrify build_include
#disabled#	@date +"START: $@, %D %T" | tee -a profile.log
#disabled#	@echo " -- MARK build apps --"
#disabled#	$(MAKE) -C src/wl/exe/xp_server BUILD_TYPES=checked
#disabled#	$(MAKE) -C src/wl/exe/vista_server BUILD_TYPES=checked
#disabled#	@date +"END:   $@, %D %T" | tee -a profile.log

build_driver: mogrify build_include
	@date +"START: $@, %D %T" | tee -a profile.log
	@echo " -- MARK build driver --"
	@$(MARKSTART)
	$(MAKE) -C src/wl/sys/wdm WIN8_WDK_VER=$(WIN8X_WDK_VERSION) WINOS=$(UPPER_VERSION_NAME)     BUILD_TYPES=checked $(if $(strip ${SERVER_PATH}),SERVER_PATH=$(strip ${SERVER_PATH})) \
		build_win8x_driver
	$(MAKE) -C components/ndis/ndisvwl/sys BUILD_TYPES=checked
	$(MAKE) -C src/wl/exe -f wl_vs13.mk BUILD_CONFS=Debugwb all
	@$(MARKEND)

ifeq ($(SKIP_BUILD_INSTALL),)
build_install: export VS_PRE_BUILD_ENABLED=0
build_install: mogrify
	@date +"START: $@, %D %T" | tee -a profile.log
	$(MAKE) -C $(INS_DIR) $(if $(VERBOSE),VERBOSE=1) BRAND=$(BRAND)
	@date +"END:   $@, %D %T" | tee -a profile.log
else # SKIP_BUILD_INSTALL
build_install:
endif # SKIP_BUILD_INSTALL

clean: profile.log
	@date +"START: $@, %D %T" | tee -a profile.log
	$(MAKE) -C src/include clean
	$(MAKE) -C $(INS_DIR) clean
	$(MAKE) -C src/wl/exe -f wl_vs13.mk clean_all
	$(MAKE) -C src/wl/sys/wdm clean
	$(MAKE) -C components/ndis/ndisvwl/sys clean
	$(MAKE) -C components/apps/windows/tray/winx clean_all
	$(MAKE) -C src/wl/cpl/components/sup/eaphost/ttls clean_all
	$(MAKE) -C components/apps/windows/ihvfrm/ihv clean_all
	$(MAKE) -C $(dir $(INS_DIR))Co-Installer/bcmwlcoi clean_all
	@date +"END:   $@, %D %T" | tee -a profile.log

# For clustered builds, these release folders need to be pre-existing
pre_release:
	@[ -d release ] || mkdir -pv release
	@[ -d release/$(VERSION_NAME)/checked/DriverOnly ] || \
		mkdir -pv release/$(VERSION_NAME)/checked/DriverOnly
	@[ -d release/$(VERSION_NAME)/checked/InstallShield ] || \
	        mkdir -pv release/$(VERSION_NAME)/checked/InstallShield

MSBUILD_INTERNALARCH := x86
VISUALSTUDIOPATH :=$(firstword $(wildcard C:/Tools/msdev/VS2013/VC C:/Tools/msdev/VS2012/VC))
PRGCMD := vcvarsall.bat $(MSBUILD_INTERNALARCH)
MSBUILD_CMD := msbuild
export SN_CMD32 := \
       "C:\Program Files (x86)\Microsoft SDKs/Windows/v8.1A/bin/NETFX 4.5.1 Tools/x64/sn.exe" -R
export SN_CMD64 := \
       "C:\Program Files (x86)\Microsoft SDKs/Windows/v8.1A/bin/NETFX 4.5.1 Tools/x64/sn.exe" -R
define BUILD_MSBUILD_COMMANDS
	cmd /c "$(VISUALSTUDIOPATH)\$(PRGCMD) && cd $(3) && \
	  set OEM=$(1)&& \
	  $(MSBUILD_CMD) /t:$(2) $(4) /p:Configuration=$(5) /p:Platform=$(6) "
endef
define BUILD_MSBUILD_COMMANDS_WDK
	cmd /c "$(VISUALSTUDIOPATH)\$(PRGCMD) && cd $(3) && \
	  set OEM=$(1)&& \
	  $(MSBUILD_CMD) /t:$(2) $(4) /p:Configuration=$(5) /p:Platform=$(6) /p:WDKContentRoot=$(WIN8X_WDK_ROOT)/"
endef
define BUILD_CONTROLPANEL_APPS
	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),build,components\apps\windows\libraries,WLWin8_VS13.sln,Debugwb,Win32)
	$(call BUILD_MSBUILD_COMMANDS,$(1),build,components\apps\windows\tray\winx,WLWin8Tray_vs13.sln,Debugwb,Win32)
	cmd /c %SN_CMD32% components\\apps\\windows\\tray\\winx\\Remoting\\Debugwb\\$(1)\\bcmwlrmt.dll  \
		components\\apps\\windows\\tray\\winx\\Remoting\\bcmwlrmt.snk

	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),build,components\apps\windows\libraries,WLWin8_VS13.sln,Debugwb,x64)
	$(call BUILD_MSBUILD_COMMANDS,$(1),build,components\apps\windows\tray\winx,WLWin8Tray_vs13.sln,Debugwb,x64)
	cmd /c %SN_CMD64% components\\apps\\windows\\tray\\winx\\Remoting\\Debugwb\\x64\\$(1)\\bcmwlrmt.dll \
		components\\apps\\windows\\tray\\winx\\Remoting\\bcmwlrmt.snk

	cmd /c "$(VISUALSTUDIOPATH)\$(PRGCMD) && set OEM= "
endef
define BUILD_COMPONENT_SOFTAP
	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),build,src\wl\cpl\vista\lenovo\WPFLenovoSoftAP,LenovoSoftAP_VS13.sln,Debugwb,Win32)
	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),build,src\wl\cpl\vista\lenovo\WPFLenovoSoftAP,LenovoSoftAP_VS13.sln,Debugwb,x64)
endef
#define BUILD_COMPONENT_WINIEAPI
#	@echo "#- $0"
#	@$(MARKSTART)
#	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),build,src\wl\cpl\vista\WinIEapi,bcmwlanapi.sln,Debug,Win32)
#	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),build,src\wl\cpl\vista\WinIEapi,bcmwlanapi.sln,Debug,x64)
#	$(call BUILD_MSBUILD_COMMANDS,$(1),build,src\wl\cpl\vista\WinIEapi,bcmwlanapp.sln,Debug,Win32)
#	$(call BUILD_MSBUILD_COMMANDS,$(1),build,src\wl\cpl\vista\WinIEapi,bcmwlanapp.sln,Debug,x64)
#	@$(MARKEND)
#endef

#define BUILD_COMPONENT_WINIEAPI_AND_MIRACAST_NATIVE
#	@echo "#- $0"
#	@$(MARKSTART)
#	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),components\apps\windows\libraries,WLWin8_VS13.sln,Debugwb,Win32)
#	$(call BUILD_MSBUILD_COMMANDS_WDK,$(1),components\apps\windows\libraries,WLWin8_VS13.sln,Debugwb,x64)
#	@$(MARKEND)
#endef

.PHONY:  build_auxiliary_libraries_bcm build_auxiliary_libraries_softap
build_auxiliary_libraries: build_auxiliary_libraries_bcm build_auxiliary_libraries_softap

build_auxiliary_libraries_bcm:
	@$(MARKSTART)
	$(call BUILD_CONTROLPANEL_APPS,bcm)
#	$(call BUILD_COMPONENT_WINIEAPI_AND_MIRACAST_NATIVE,generic)
#	$(call BUILD_COMPONENT_WINIEAPI,generic)
	@$(MARKEND)


build_auxiliary_libraries_softap: export VS_PRE_BUILD_ENABLED=0
build_auxiliary_libraries_softap:
	@$(MARKSTART)
#	$(call BUILD_COMPONENT_SOFTAP,bcm)
	@$(MARKEND)

##############################################################################
# release xp and vista apps for servers
##############################################################################
release_apps: pre_release
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe  \
	$(RELEASEDIR)/WinXP/checked/Apps/
	$(INSTALL) src/wl/exe/xp_server/obj/server/dongle/checked/wl_server_dongle.exe $(RELEASEDIR)/WinXP/checked/Apps/
	$(INSTALL) src/wl/exe/xp_server/obj/server/socket/checked/wl_server_socket.exe $(RELEASEDIR)/WinXP/checked/Apps/
	$(INSTALL) src/wl/exe/xp_server/obj/server/802.11/checked/wl_server_wifi.exe $(RELEASEDIR)/WinXP/checked/Apps/

release: release_win8x

##############################################################################
# release_xp for WinXP/2K
##############################################################################
release_xp: \
	 $(RELEASEDIR)/WinXP/checked/InstallShield \
	 $(RELEASEDIR)/WinXP/checked/DriverOnly \
	 $(RELEASEDIR)/WinXP/checked/DriverOnly_VWL

# WinXP/2K - internal debug installshield
$(RELEASEDIR)/WinXP/checked/InstallShield: FORCE $(PRE_RELEASE_DEPS_XP)
	@date +"START: $@, %D %T" | tee -a profile.log
	mkdir -p $@/X64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/WinXP/WL
	# install driver
	$(INSTALL) $(XP_x86_WLDRIVER_C) $@/Drivers/WinXP/WL
	$(INSTALL) $(XP_x64_WLDRIVER_C) $@/Drivers/WinXP/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl5,bcmwl5i,$(XP_WLINF)),$@/Drivers/WinXP/WL/$(XP_WLINF))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@/Drivers/WinXP/WL)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,Debug,Drivers/WinXP/WL)^M
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/Debug,Drivers/WinXP/WL)^M

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
	# install tools
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe $(@D)/
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe $@/
	(cd src/wl/cpl/preflib/wlconfig/package && $(FIND) . -print | cpio -pmvud $(@D)/preflib)
	(cd src/wl/cpl/preflib/preflibcl/package && $(FIND) . -print | cpio -pmvud $(@D)/preflibcl)
	$(INSTALL) -D src/wl/cpl/preflib/preflibcl/prefdump/Debug/prefdump.exe $(@D)/preflibcl/bin/release/prefdump.exe
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

# WinXP/2K - internal debug driver-only files
$(RELEASEDIR)/WinXP/checked/DriverOnly: FORCE
	mkdir -p $@
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) $(XP_x86_WLDRIVER_C) $@/
	$(INSTALL) $(XP_x64_WLDRIVER_C) $@/
	$(call INSTALL_32BIT_BUILT_COINSTALLER,Debug)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/Debug)
	$(INSTALL) $(subst .sys,.pdb,$(XP_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.pdb,$(XP_x64_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(XP_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(XP_x64_WLDRIVER_C)) $@/
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl5,bcmwl5i,$(XP_WLINF)),$@/$(XP_WLINF))
	$(call INSTALL_UNSIGNED_DRIVER_CAT,$@)
	$(INSTALL) src/wl/exe/windows/winxp/obj/checked/wl.exe $@/
	$(call REMOVE_WARN_PARTIAL_BUILD)
#	End of $(RELEASEDIR)/WinXP/checked/DriverOnly

# WinXP/2K
$(RELEASEDIR)/WinXP/checked/DriverOnly_VWL: FORCE
	$(call INSTALL_VWL_DRIVER,$@,WINXP)

## WinVista and Win7
release_win7:  \
		$(RELEASEDIR)/Win7/checked/InstallShield \
		$(RELEASEDIR)/Win7/checked/DriverOnly \
		$(RELEASEDIR)/Win7/checked/DriverOnly_VWL \
		$(RELEASEDIR)/Win7/checked/DriverOnly_IS

## WinVista and Win7 - internal debug installshield
$(RELEASEDIR)/Win7/checked/InstallShield: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@date +"START: $@, %D %T" | tee -a profile.log
	mkdir -p $@/X64
	$(call WARN_PARTIAL_BUILD)
	mkdir -pv $@/Drivers/Win7/{WL,VWL}
	mkdir -pv $@/Drivers/WinVista/WL
	# install driver and its dependent files
	$(INSTALL) $(VISTA_x86_WLDRIVER_C) $@/Drivers/Win7/WL
	$(INSTALL) $(VISTA_x64_WLDRIVER_C) $@/Drivers/Win7/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl6,bcmwl6i,$(VISTA_WLINF)),$@/Drivers/Win7/WL/$(VISTA_WLINF))
	$(call INSTALL_VISTA32BIT_BUILT_IHV,Debugwb/bcm,Drivers/Win7/WL/)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/Debugwb/bcm,Drivers/Win7/WL/)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,Debug,Drivers/Win7/WL)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/Debug,Drivers/Win7/WL)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@/Drivers/Win7/WL,X64)
	$(call INSTALL_VISTA_REDIST_IHV)
	# Install and sign ndis virtual wl driver bits
	$(call INSTALL_VWL_DRIVER,$@/Drivers/Win7/VWL,WIN7)
	$(call INSTALL_PEERNET_INSTALLER,$@)
	# Copy over Win7 bits as Vista (and as Win8 in future)
	$(INSTALL) $@/Drivers/Win7/WL/*.cat $@/Drivers/WinVista/WL/
	$(INSTALL) $(VISTA_x86_PROTOCOLDRIVER) $@/
	$(INSTALL) $(VISTA_x64_PROTOCOLDRIVER) $@/x64
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
	$(call INSTALL_BLUEWIRE,debug/bcm)
	$(call INSTALL_VISTA_HSM_BIN,Debugwb)
	$(call INSTALL_VS0564BIT_VCREDIST_DEBUG,checked,$@/x64)
	$(call INSTALL_VS0864BIT_VCREDIST_DEBUG,checked,$@/vs08/x64)
	$(call INSTALL_VISTA_BUILT_APPS,DebugUv/x64/bcm,x64)
	$(call INSTALL_BLUEWIRE,debug/x64/bcm,x64)
	$(call INSTALL_VISTA_BUILT_TTLS,x64/Debug/bcm,x64)
	$(call INSTALL_VISTA_HSM_BIN,Debugwb/x64,x64)
	# install File Sharing icons
	$(call INSTALL_VISTA_FS_ICONS,$@/FshSvrAdmin)
	# install help
	# install installer
	## Install installshield files
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,checked)
	$(INSTALL) $(INS_DIR)/ini_vista/bcm/checked/internal.ini $@/bcmwls.ini
	$(call INSTALL_MUIDLLS,bcm)
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup_vista.ini $@/Setup.ini
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/Debug/BcmSetupUtil.exe $@/
	$(INSTALL) $(dir $(INS_DIR))BcmSetupUtil/x64/Debug/BcmSetupUtil.exe $@/x64
	# Copy utilities that install peernet dll into machine GAC
	$(INSTALL) $(INS_DIR)/Utils/Inst2Gac.exe $@/
	# install tools
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/checked/wl.exe $(@D)/
	$(INSTALL) src/wl/exe/windows/win7/obj/winvista/external/checked/wl.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/doc/mdc.jpg $@/
	$(INSTALL) src/doc/READMEBCMCli_Vista.rtf $@/README.rtf
	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(call REMOVE_WARN_PARTIAL_BUILD)
#	End of release/Win7/checked/InstallShield

## WinVista and Win7 - internal release driver-only
$(RELEASEDIR)/Win7/checked/DriverOnly: FORCE
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
	$(call INSTALL_VISTA32BIT_BUILT_IHV,Debugwb/bcm)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/Debugwb/bcm)
	$(call INSTALL_32BIT_BUILT_COINSTALLER,Debug)
	$(call INSTALL_64BIT_BUILT_COINSTALLER,x64/Debug)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X86)
	$(call RELEASE_SIGN_VISTA_DRIVER,$@,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
#	End of $(RELEASEDIR)/Win7/checked/DriverOnly

# WinVista and Win7
$(RELEASEDIR)/Win7/checked/DriverOnly_VWL: FORCE
	$(call INSTALL_VWL_DRIVER,$@,WIN7)
	$(call INSTALL_PEERNET_INSTALLER,$@)


##############################################################################
# release_win8x for Win8X
##############################################################################
release_win8x: $(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly
# release_win8x: $(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly \
#               $(RELEASEDIR)/$(VERSION_NAME)/checked/InstallShield

## Win8X - internal release driver-only
$(RELEASEDIR)/$(VERSION_NAME)/checked/DriverOnly: FORCE
	mkdir -p $@
	mkdir -pv $@/x64
	$(call WARN_PARTIAL_BUILD)
	$(INSTALL) src/wl/exe/Debugwb/wl.exe $@/
	$(INSTALL) $(WIN8X_x86_WLDRIVER_C) $(WIN8X_x64_WLDRIVER_C) $@/
	$(INSTALL) $(subst .sys,.pdb,$(WIN8X_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.pdb,$(WIN8X_x64_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(WIN8X_x86_WLDRIVER_C)) $@/
	$(INSTALL) $(subst .sys,.map,$(WIN8X_x64_WLDRIVER_C)) $@/
	# For driver restart use
#	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/Debugwb/bcmwlapiu.dll $@/
#	$(INSTALL) components/apps/windows/libraries/WlAdapter/DLL/Debugwb/x64/bcmwlapiu.dll $@/x64
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl63,bcmwl63i,$(WIN8X_WLINF)),$@/$(WIN8X_WLINF))
#	$(call INSTALL_VISTA32BIT_BUILT_IHV,Debugwb/bcm)
#	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/Debugwb/bcm)
	$(call RELEASE_SIGN_WIN8X_DRIVER,$@,X86)
	$(call RELEASE_SIGN_WIN8X_DRIVER,$@,X64)
	$(call REMOVE_WARN_PARTIAL_BUILD)
#	$(call INSTALL_COMPONENT_WINIEAPI,generic)
#	$(call INSTALL_COMPONENT_MIRACAST_NATIVE,bcm)
#	End of $(RELEASEDIR)/Win8X/checked/DriverOnly

#Win8X - checked InstallShield
$(RELEASEDIR)/$(VERSION_NAME)/checked/InstallShield: FORCE $(PRE_RELEASE_DEPS_VISTA)
	@$(MARKSTART)
	mkdir -pv $@/x64 $@/vs13/x64 $@/Drivers/$(VERSION_NAME)/{WL,VWL}
	mkdir -pv $@/EAP_Plugin_Installer
	mkdir -pv $@/MUI
	$(call WARN_PARTIAL_BUILD)
	$(call INSTALL_AUXAPP,bcm)
#	$(call INSTALL_COMPONENT_SOFTAP)
	$(call INSTALL_COMPONENT_WINIEAPI,generic)
	$(call INSTALL_COMPONENT_WINIEAPI_DEBUGSYMBOLS,generic)
#	$(call INSTALL_COMPONENT_MIRACAST_NATIVE,bcm)
#	$(call INSTALL_COMPONENT_MIRACAST_NATIVE_DEBUGSYMBOLS,bcm)
	# install tools
	$(INSTALL) src/wl/exe/Debugwb/wl.exe $(@D)/
	$(INSTALL) src/wl/exe/Debugwb/wl.exe $@/
	$(call INSTALL_VISTA32BIT_BUILT_IHV,Debugwb/bcm,Drivers/$(VERSION_NAME)/WL)
	$(call INSTALL_VISTA64BIT_BUILT_IHV,x64/Debugwb/bcm,Drivers/$(VERSION_NAME)/WL)
	$(INSTALL) components/apps/windows/ihvfrm/ihv/Debugwb/wapiutil.exe  $@/
	$(INSTALL) components/apps/windows/ihvfrm/ihv/x64/Debugwb/wapiutil.exe $@/x64
	# install driver and its dependent files
	$(INSTALL) $(WIN8X_x86_WLDRIVER_C) $(WIN8X_x64_WLDRIVER_C) $@/Drivers/$(VERSION_NAME)/WL
	$(call INSTALL_INF,$(INF_DIR)/bcm/$(subst bcmwl63,bcmwl63i,$(WIN8X_WLINF)),$@/Drivers/$(UPPER_VERSION_NAME)/WL/$(WIN8X_WLINF))

	# Temp duplicate .CAT file to Win8 folder for installshield
	mkdir -pv  $@/Drivers/Win8/VWL
	$(INSTALL) $@/Drivers/$(VERSION_NAME)/VWL/*.cat $@/Drivers/Win8/VWL/

	$(call RELEASE_SIGN_WIN8X_DRIVER,$@/Drivers/$(VERSION_NAME)/WL,X86)
	$(call RELEASE_SIGN_WIN8X_DRIVER,$@/Drivers/$(VERSION_NAME)/WL,X64)
	## Install installshield files
	$(call INSTALL_INSTALLSHIELD_FILES,bcm,checked)
	$(INSTALL) $(INS_DIR)/ini_win8x/bcm/checked/internal.ini $@/bcmwls.ini
	$(INSTALL) $(INS_DIR)/ini_win8x/bcm/checked/driver.ini $@/bcmwlsd.ini
	$(INSTALL) $(INS_DIR)/installshield/BCMAppInstaller/is_bin/bcm/Setup_win8x.ini $@/Setup.ini
	unix2dos $@/Setup.ini
	$(INSTALL) $(INS_DIR)/sysinfo/Debug/SysInfo.exe $@/
	# copy package version dll
	$(INSTALL) $(INS_DIR)/Utils/PackageVersion.dll $@/
	$(INSTALL) src/doc/BCMLogo.gif $@/
	$(INSTALL) src/doc/READMEBCMCli_Vista.rtf $@/README.rtf
	$(INSTALL) src/doc/ReleaseNotesWl.html $@/ReleaseNotes.html
	$(call REMOVE_WARN_PARTIAL_BUILD)
	@$(MARKEND)
	# End of release/Win8X/checked/InstallShield

build_infstamp:
	# stamp all the infs
	@echo maj=$(maj)
	@echo min=$(min)
	@echo rcnum=$(rcnum)
	@echo incremental=$(incremental)
	src/tools/release/wustamp -a -o -r -v "$(maj).$(min).$(rcnum).$(incremental)" "`cygpath -w $(RELEASEDIR)`"
	@date +"END:   $@, %D %T" | tee -a profile.log

# This package_for_web is deprecated and obsoleted since we have
# single installer from installshield
package_for_web:
	@# make self-extracting zip files of the installshield directories
	@# for use by the upgrade server.
	@date +"START: $@, %D %T" | tee -a profile.log
	@if type pftwwiz.exe >/dev/null 2>&1; then \
	   for dir in checked/InstallShield; do \
		cd $(RELEASEDIR)/$${dir}/; \
		cp $(BUILD_BASE)/src/tools/release/802.11.pfw $(BUILD_BASE)/802.11.pfw; \
		echo Packaging $${dir}; \
		pftwwiz.exe $(BUILD_BASE)/802.11.pfw -a -s; \
	   done \
	else \
	   echo pftwwiz.exe not found - not packaging release.; \
	fi
	@date +"END:   $@, %D %T" | tee -a profile.log

profile.log :
	@date +"START: win_internal_wl tag=${TAG} %D %T" | tee profile.log

$(RELEASEDIR)/% :
	mkdir -p $@

#pragma runlocal
build_dlgtest: rundlgtest email_dlgtest_errors

#pragma runlocal
rundlgtest:
	rm -f $(DLGTEST_LOG)
	-$(MAKE) -C src/tools/locale/dlgtest CONFIG_NAME=Debug $@
	-$(MAKE) -C src/tools/locale/dlgtest CONFIG_NAME=DebugUv $@
#vista>	# once dlgtest tool is ported to 64bit systems, uncomment these
#vista>	-$(MAKE) -C src/tools/locale/dlgtest CONFIG_NAME="DebugUv|x64" $@

#pragma runlocal
email_dlgtest_errors: rundlgtest
ifeq ($(DLGTEST_NOTIFY),true)
	@test ! -s "$(DLGTEST_LOG)" || \
	    (set -x; blat $(DLGTEST_LOG) -mailfrom hwnbuild -t $(DLGTEST_NOTIFY_LIST) \
		-s "DialogTest FAILED for $(if $(TAG),$(TAG),NIGHTLY) $(BRAND) build")
endif

#pragma runlocal
build_clean: release
	-@$(FIND) src components -type f -name "*\.obj" -o -name "*\.OBJ" -o -name "*\.o" -o -name "*\.O" | \
		xargs -P20 -n100 rm -f

build_end: build_clean
	@date +"BUILD_END: win_internal_wl TAG=${TAG} %D %T" | tee -a profile.log

# Usage: "$(MAKE) -f win_internal_wl.mk SHELL=/bin/bash syntax_check"
syntax_check:
	@echo "Checking Makefile syntax only"

.PHONY: src-top pre_release release_xp build_trayapp build_trayapp_xp2k build_auxiliary_libraries build_trayapp_vista build_trayapp_vista_dotfuscate build_driver mogrify build_install build_include email_dlgtest_errors FORCE



################################################################################
## Simple text substitution utilities to avoid excessive $(shell ) calls.
## Only usable for = and not := due to position at the end of this file
################################################################################

.tolower = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))
.toupper = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

## #########################################################################
## Set Environment Paths and variables for All Visual Studio 200X and
## Windows Mobile/WinCE projects
##
## Author: Prakash Dhavali
## Contact: hnd-software-scm-list
##
## $Id$
## #########################################################################

empty               :=
space               :=$(empty) $(empty)
SHELL               :=bash.exe
DEVENV_EXTRA_FLAGS  :=/useenv
DEVENV              :=devenv.com
UNICONV             :=$(SRCBASE)/tools/locale/free/uniconv.exe
VSVER               ?=VS2005
WDK_VER             ?=7600
CCX_SDK_VER         ?=1.1.13
WMARCH              ?=ARMV4I
WLAN_WINPFX         ?= Z:

## Vista/IHV CCX SDK Path
CCXSDKDIR   ?= $(WLAN_WINPFX)/projects/hnd/restrictedTools/CCX_SDK/$(CCX_SDK_VER)

## Dotfuscator dir
DOTFUSCATORDIR ?= C:/tools/dotfuscator3.0

## MSVS Command Line need following variables
VSINSTALLDIR ?= $(firstword $(wildcard C:/TOOLS/msdev/$(VSVER) D:/TOOLS/msdev/$(VSVER) $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(VSVER)/))

## For VS2008, override the SDK to 7.0 instead of in-built 6.0A version
ifeq ($(VSVER),VS2008)
  MSSDKDir := $(firstword $(wildcard C:/TOOLS/msdev/MSSdk70 D:/TOOLS/msdev/MSSdk70 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/MSSdk70))
endif

## Even for VS2005, override the SDK to 7.0 instead of platform sdk
ifeq ($(VSVER),VS2005)
  MSSDKDir := $(firstword $(wildcard C:/TOOLS/msdev/MSSdk70 D:/TOOLS/msdev/MSSdk70 Z:/projects/hnd/tools/win/msdev/MSSdk70))
endif

# TODO: RTMSDK60 needs to be obsoleted for VS08, as MSSDK70 should suffice
# TODO: Current some solutions fail if we don't have RTMSDK60 in path.
# TODO: That needs to be cleaned up and RTMSDK60 needs to be removed for VS08
RTMSDK60 ?= $(firstword $(wildcard C:/TOOLS/msdev/RtmSDK60 D:/TOOLS/msdev/RtmSDK60 $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/RtmSDK60))

## Cygpath path
ifdef CYGWIN_DIRECTORY
MYCYGPATH ?= C:/$(CYGWIN_DIRECTORY)/bin
else
MYCYGPATH ?= $(firstword $(wildcard C:/TOOLS/win32/bin D:/TOOLS/win32/bin $(WLAN_WINPFX)/projects/hnd/tools/win32/bin))
endif

## XP DDK path
WDM2600DDK ?= $(firstword $(wildcard C:/TOOLS/msdev/2600ddk D:/TOOLS/msdev/2600ddk $(WLAN_WINPFX)/TOOLS/msdev/2600ddk/))

## XP/Vista/Windows WDK path
WDM7600WDK ?= $(firstword $(wildcard C:/TOOLS/msdev/7600wdk D:/TOOLS/msdev/7600wdk Z:/Projects/hnd/tools/win/msdev/7600wdk/))

## Vista WDK version and path
WDK_DIR ?= $(firstword $(wildcard C:/TOOLS/msdev/$(WDK_VER)wdk D:/TOOLS/msdev/$(WDK_VER)wdk $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/$(WDK_VER)wdk))
WDKDIR  ?= $(WDK_DIR)

## $(WMARCH) tool for WINCE500STDSDK and WM6PSDK projects
ifdef REQUIRE_WM6PSDK
  # Standard SDK for WinCE 500
  WINCE500STDSDK ?= $(firstword $(wildcard C:/TOOLS/msdev/wince500_stdsdk D:/TOOLS/msdev/wince500_stdsdk $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/wince500_stdsdk/))/wce500/STANDARDSDK_500

  # Win Mobile 6 Platform SDK
  WM6PSDK ?= $(firstword $(wildcard C:/TOOLS/msdev/WM6PSDK D:/TOOLS/msdev/WM6PSDK $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/WM6PSDK/))/'Windows Mobile 6 SDK'

  WINCEVER   ?= 500
  _WINCEROOT  = $(firstword $(wildcard c:/tools/msdev/wince$(WINCEVER) d:/tools/msdev/wince$(WINCEVER) $(WLAN_WINPFX)/projects/hnd/tools/wince$(WINCEVER)))

endif # REQUIRE_WM6PSDK

## CNG SDK
CNGSDKDIR ?= $(firstword $(wildcard C:/TOOLS/msdev/CNGSDK D:/TOOLS/msdev/CNGSDK $(WLAN_WINPFX)/projects/hnd/tools/win/msdev/CNGSDK))

# SDK env variables referenced *inside* visual studio projects via include
# and lib paths
ifeq ($(VSVER),VS2008)
  export MYWINDOWSSDKDIR:=$(MSSDKDir)/
else
  ifeq ($(VSVER),VS2005)
    export MYWINDOWSSDKDIR:=$(MSSDKDir)/
  else
    export MYWINDOWSSDKDIR:=$(RTMSDK60)/
  endif
endif
export MYWINDOWSSDKPATCHESDIR:=$(MYWINDOWSSDKDIR)/

export DOTFUSCATORDIR

export MYCNGSDKDIR:=$(CNGSDKDIR)/

# DDK env variables referenced *inside* visual studio projects via include
# and lib paths
export MYWINDOWSDDKDIR:=$(WDK_DIR)/
export MYWINDOWSDDKPATCHESDIR:=$(WDK_DIR)/
export MYCCXSDKDIR:=$(CCXSDKDIR)/
export WDKDIR

VCINSTALLDIR   ?= $(VSINSTALLDIR)/VC
DevEnvDir      := $(VSINSTALLDIR)/Common7/IDE
MSVCDir        := $(VCINSTALLDIR)

# WARN: #########################################################
# WARN: DO NOT CHANGE THIS ORDER. PLEASE CONSULT TRAYAPP TEAM
# WARN: IF YOU NEED TO MAKE ANY CHANGES HERE
# WARN: SDK 70 needs to be ahead across PATH, INCLUDE and LIB
# WARN: #########################################################
# Derive env for i386 (32bit project configs)
ifeq ($(VSVER),VS2008)
  MYVSPATH       :=  $(DevEnvDir);$(MSVCDir)/bin;$(VSINSTALLDIR)/Common7/Tools;$(VSINSTALLDIR)/Common7/Tools/bin;$(MSVCDir)/PlatformSDK/bin;$(MSSDKDir)/bin;$(MYWINDOWSSDKDIR)/bin
  MYVSINCLUDE    := $(MSVCDir)/ATLMFC/INCLUDE;$(MSVCDir)/INCLUDE;$(MSSDKDir)/include;$(MSVCDir)/PlatformSDK/include;$(MYWINDOWSSDKDIR)/include
  MYVSLIB        := $(MYWINDOWSSDKDIR)/LIB;$(MSVCDir)/ATLMFC/LIB;$(MSVCDir)/LIB;$(MSSDKDir)/lib;$(MSVCDir)/PlatformSDK/lib
else # VS2005
  MYVSPATH    := $(MYWINDOWSSDKDIR)/bin;$(DevEnvDir);$(MSVCDir)/bin;$(VSINSTALLDIR)/Common7/Tools;$(VSINSTALLDIR)/Common7/Tools/bin;$(WDM7600WDK)/bin/x86
  MYVSINCLUDE := $(MYWINDOWSSDKDIR)/include;$(MSVCDir)/ATLMFC/INCLUDE;$(MSVCDir)/INCLUDE;$(MYWINDOWSSDKDIR)/include;$(WDM7600WDK)/inc/api
  MYVSLIB     := $(MYWINDOWSSDKDIR)/LIB;$(MSVCDir)/ATLMFC/LIB;$(MSVCDir)/LIB
endif
MYLIBPATH      := $(MSVCDir)/ATLMFC/LIB

ifdef REQUIRE_WDM2600
  MYVSINCLUDE  := $(MYVSINCLUDE);$(WDM2600DDK)/inc/wxp
  MYVSLIB      := $(MYVSLIB);$(WDM2600DDK)/lib/wxp/i386
endif # REQUIRE_WDM2600

ifdef REQUIRE_WDM7600
  # TO-DO: Move this customziation for IHV from this central
  # TO-DO: makefile to IHV module
  ifneq ($(findstring ihv.sln,$(SOLUTION)),)
    MYVSINCLUDE  := $(WDM7600WDK)/inc/api;$(MYVSINCLUDE)
    MYVSLIB      := $(WDM7600WDK)/lib/win7/i386;$(MYVSLIB)
  else
    MYVSINCLUDE  := $(MYVSINCLUDE);$(WDM7600WDK)/inc/api
    MYVSLIB      := $(MYVSLIB);$(WDM7600WDK)/lib/wxp/i386
  endif
endif # REQUIRE_WDM7600

# Drive $(WMARCH) env for winmobile
# NOTE: Leaving WMARCH value configurable
ifdef REQUIRE_WM6PSDK
  MYVSPATH_CE    := $(VCINSTALLDIR)/CE/bin/x86_arm;$(MYVSPATH)

  MYVSINCLUDE_CE := $(if $(VSINC_PFX),$(VSINC_PFX);)$(VCINSTALLDIR)/CE/include
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(WINCE500STDSDK)/include/$(WMARCH)
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(WINCE500STDSDK)/include
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(WM6PSDK)/PocketPC/include/$(WMARCH)
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(WM6PSDK)/PocketPC/include
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(VCINSTALLDIR)/CE/ATLMFC/include
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(VCINSTALLDIR)/SmartDevies/SDK/SQL Server/Mobile/v3.0
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE);$(VSINSTALLDIR)/SmartDevices/SDK/PocketPC2003/Include
  MYVSINCLUDE_CE := $(MYVSINCLUDE_CE)$(if $(VSINC_SFX),;$(VSINC_SFX))

  MYVSLIB_CE     := $(if $(VSLIB_PFX),$(VSLIB_PFX);)$(WINCE500STDSDK)/lib/$(WMARCH)
  MYVSLIB_CE     := $(MYVSLIB_CE);$(WM6PSDK)/PocketPC/lib/$(WMARCH)
  MYVSLIB_CE     := $(MYVSLIB_CE);$(VCINSTALLDIR)/CE/ATLMFC/lib/$(WMARCH);$(VCINSTALLDIR)/CE/lib/$(WMARCH)
  MYVSLIB_CE     := $(MYVSLIB_CE);$(VSINSTALLDIR)/SmartDevices/SDK/PocketPC2003/Lib/armv4
  MYVSLIB_CE     := $(MYVSLIB_CE)$(if $(VSLIB_SFX),;$(VSLIB_SFX))

  MYLIBPATH_CE   :=
endif # REQUIRE_WM6PSDK

# Derive env for x64 (64bit project configs)
ifeq ($(VSVER),VS2008)
  MYVSPATH_X64   := $(DevEnvDir);$(MSVCDir)/bin/x86_amd64;$(MSVCDir)/bin;$(VSINSTALLDIR)/Common7/Tools;$(VSINSTALLDIR)/Common7/Tools/bin;$(MSVCDir)/PlatformSDK/bin;$(MSSDKDir)/bin;$(MYWINDOWSSDKDIR)/bin
  MYVSLIB_X64    := $(MYWINDOWSSDKDIR)/LIB/x64;$(MSVCDir)/ATLMFC/LIB/amd64;$(MSVCDir)/LIB/amd64;$(MSSDKDir)/lib/x64;$(MSVCDir)/PlatformSDK/lib/amd64
else # VS2005
  MYVSPATH_X64   := $(DevEnvDir);$(MSVCDir)/bin/x86_amd64;$(MSVCDir)/bin;$(VSINSTALLDIR)/Common7/Tools;$(VSINSTALLDIR)/Common7/Tools/bin;$(MSVCDir)/PlatformSDK/bin;$(VSINSTALLDIR)/SDK/v2.0/bin;$(MYWINDOWSSDKDIR)/bin
  MYVSLIB_X64    := $(MYWINDOWSSDKDIR)/LIB/x64;$(MSVCDir)/ATLMFC/LIB/amd64;$(MSVCDir)/LIB/amd64;$(MSVCDir)/PlatformSDK/lib/amd64;$(VSINSTALLDIR)/SDK/v2.0/lib/amd64
endif
MYLIBPATH_X64  := $(MSVCDir)/ATLMFC/LIB/amd64

## HND Additions to Microsoft Visual Studio .NET 200x paths
NEWPATH        := $(MYCYGPATH);$(MYVSPATH)
NEWPATH        := $(shell cygpath -p "$(NEWPATH)")
NEWINCLUDE     := $(subst ; ,;,$(subst //,/,$(MYVSINCLUDE)))
NEWLIB         := $(subst ; ,;,$(subst //,/,$(MYVSLIB)))
NEWLIBPATH     := $(subst ; ,;,$(subst //,/,$(MYLIBPATH);$(LIBPATH)))

ifdef REQUIRE_WM6PSDK
  NEWPATH_CE   := $(MYCYGPATH);$(MYVSPATH_CE)
  NEWPATH_CE   := $(shell cygpath -p "$(NEWPATH_CE)")
  NEWINCLUDE_CE:= $(subst ; ,;,$(subst //,/,$(MYVSINCLUDE_CE)))
  NEWLIB_CE    := $(subst ; ,;,$(subst //,/,$(MYVSLIB_CE)))
  NEWLIBPATH_CE:=
endif # REQUIRE_WM6PSDK

# x64 configurations
NEWPATH_X64    := $(MYCYGPATH);$(MYVSPATH_X64)
NEWPATH_X64    := $(shell cygpath -p "$(NEWPATH_X64)")
NEWLIB_X64     := $(subst ; ,;,$(subst //,/,$(MYVSLIB_X64)))
NEWLIBPATH_X64 := $(subst ; ,;,$(subst //,/,$(MYLIBPATH_X64);$(LIBPATH)))

## Final i386 paths
PATH        := $(subst //,/,$(NEWPATH):$(PATH))
INCLUDE     := $(subst /,\,$(NEWINCLUDE))
LIB         := $(subst /,\,$(NEWLIB))
LIBPATH     := $(subst /,\,$(NEWLIBPATH))

PATH_WIN32     := $(PATH)
INCLUDE_WIN32  := $(INCLUDE)
LIB_WIN32      := $(LIB)
LIBPATH_WIN32  := $(LIBPATH)

export PATH INCLUDE LIB LIBPATH
export PATH_WIN32 INCLUDE_WIN32 LIB_WIN32 LIBPATH_WIN32

## Final $(WMARCH) paths
ifdef REQUIRE_WM6PSDK
  PATH_CE   := $(subst //,/,$(NEWPATH_CE):$(PATH))
  INCLUDE_CE:= $(subst /,\,$(NEWINCLUDE_CE))
  LIB_CE    := $(subst /,\,$(NEWLIB_CE))
  LIBPATH_CE:= $(subst /,\,$(NEWLIBPATH_CE))

  export PATH_CE INCLUDE_CE LIB_CE LIBPATH_CE
  export _WINCEROOT
endif # REQUIRE_WM6PSDK

## Final x64 paths; export to subsequent caller makefiles
PATH_X64    := $(subst //,/,$(NEWPATH_X64):$(PATH))
LIB_X64     := $(subst /,\,$(NEWLIB_X64))
LIBPATH_X64 := $(subst /,\,$(NEWLIBPATH_X64))

export PATH_X64 LIB_X64 LIBPATH_X64

## Export VS variables
export VCINSTALLDIR
export VSINSTALLDIR

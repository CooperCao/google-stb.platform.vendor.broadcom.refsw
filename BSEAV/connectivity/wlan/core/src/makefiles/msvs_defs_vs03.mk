## #########################################################################
## Set Environment Paths and variables for All Visual Studio 200X Builds
##
## $Id$
## #########################################################################

empty               :=
space               := $(empty) $(empty)
SHELL               := bash.exe
DEVENV_EXTRA_FLAGS  := /useenv
DEVENV              := devenv.com
UNICONV             := $(SRCBASE)/tools/locale/free/uniconv.exe
MAKEFILE_VSXX       := GNUmakefile
MAKEFILE_VS03       := GNUmakefile

## Modules or caller can set VSVER env var to indicate that they
## need either VS2003 or VS2005 toolchain, if not it is derived
## from solution file name.
ifndef VSVER
  ifeq ($(findstring VS03,$(SOLUTION)),VS03)
     VSVER := VS2003
  else
     VSVER := VS2005
  endif
endif

ifeq ($(origin MYCYGPATH), undefined)
  ifdef CYGWIN_DIRECTORY
    MYCYGPATH := C:/$(CYGWIN_DIRECTORY)/bin
  else
    MYCYGPATH := $(firstword $(wildcard C:/tools/win32/bin D:/tools/win32/bin Z:/projects/hnd/tools/win32/bin))
  endif
endif

ifeq ($(origin WDM2600DDK), undefined)
  WDM2600DDK := $(firstword $(wildcard C:/tools/msdev/2600ddk D:/tools/msdev/2600ddk Z:/tools/msdev/2600ddk/ ))
endif

## MSVS Command Line need following variables
ifeq ($(VSVER),VS2003)

  ifeq ($(origin VCINSTALLDIR), undefined)
       VCINSTALLDIR := $(firstword $(wildcard C:/tools/msdev/$(VSVER) D:/tools/msdev/$(VSVER) Z:/projects/hnd/tools/win/msdev/$(VSVER)/ ))
       VSINSTALLDIR     := $(VCINSTALLDIR)/Common7/IDE
  endif
  DevEnvDir      := $(VSINSTALLDIR)
  MSVCDir        := $(VCINSTALLDIR)/VC7
  MYVSPATH       := $(DevEnvDir) $(MSVCDir)/bin $(VCINSTALLDIR)/Common7/Tools $(VCINSTALLDIR)/Common7/Tools/bin/prerelease $(VCINSTALLDIR)/Common7/Tools/bin
  MYVSINCLUDE    := $(MSVCDir)/ATLMFC/INCLUDE;$(MSVCDir)/INCLUDE;$(MSVCDir)/PlatformSDK/include/prerelease;$(MSVCDir)/PlatformSDK/include
  MYVSLIB        := $(MSVCDir)/ATLMFC/LIB;$(MSVCDir)/LIB;$(MSVCDir)/PlatformSDK/lib/prerelease;$(MSVCDir)/PlatformSDK/lib

else # VS2005

  ifeq ($(origin VCINSTALLDIR), undefined)
       VSINSTALLDIR := $(firstword $(wildcard C:/tools/msdev/$(VSVER) D:/tools/msdev/$(VSVER) Z:/projects/hnd/tools/win/msdev/$(VSVER)/ ))
       VCINSTALLDIR := $(VSINSTALLDIR)/VC
  endif
  ifeq ($(origin RTMSDK60), undefined)
     RTMSDK60 := $(firstword $(wildcard C:/tools/msdev/RtmSDK60 D:/tools/msdev/RtmSDK60 Z:/projects/hnd/tools/win/msdev/RtmSDK60 ))
  endif
  DevEnvDir      := $(VSINSTALLDIR)/Common7/IDE
  MYWINDOWSSDKDIR:= $(RTMSDK60)/
  MyWindowsSDKDir:= $(RTMSDK60)/
  MYWINDOWSSDKPATCHESDIR:= $(MYWINDOWSSDKDIR)
  MSVCDir        := $(VCINSTALLDIR)
  MYVSPATH       := $(DevEnvDir) $(MSVCDir)/bin $(VSINSTALLDIR)/Common7/Tools $(VSINSTALLDIR)/Common7/Tools/bin $(MSVCDir)/PlatformSDK/bin $(VSINSTALLDIR)/SDK/v2.0/bin $(MYWINDOWSSDKDIR)/bin
  MYVSINCLUDE    := $(MSVCDir)/ATLMFC/INCLUDE;$(MSVCDir)/INCLUDE;$(MSVCDir)/PlatformSDK/include;$(VSINSTALLDIR)/SDK/v2.0/include;$(MYWINDOWSSDKDIR)/include
  MYLIBPATH      := $(MSVCDir)/ATLMFC/LIB
  MYVSLIB        := $(MYWINDOWSSDKDIR)/LIB;$(MSVCDir)/ATLMFC/LIB;$(MSVCDir)/LIB;$(MSVCDir)/PlatformSDK/lib;$(VSINSTALLDIR)/SDK/v2.0/lib

  # for x64
  MYVSPATH_X64   := $(DevEnvDir) $(MSVCDir)/bin/x86_amd64 $(MSVCDir)/bin $(VSINSTALLDIR)/Common7/Tools $(VSINSTALLDIR)/Common7/Tools/bin $(MSVCDir)/PlatformSDK/bin $(VSINSTALLDIR)/SDK/v2.0/bin $(MYWINDOWSSDKDIR)/bin
  MYVSLIB_X64    := $(MYWINDOWSSDKDIR)/LIB/x64;$(MSVCDir)/ATLMFC/LIB/amd64;$(MSVCDir)/LIB/amd64;$(MSVCDir)/PlatformSDK/lib/amd64;$(VSINSTALLDIR)/SDK/v2.0/lib/amd64
  MYLIBPATH_X64  := $(MSVCDir)/ATLMFC/LIB/amd64

endif # VSVER

## amd64 configuration for vs2003 projects
ifeq ($(VSVER),VS2003)
  ifdef BUILD_AMD64
    ## MSVC++ 64bit tools and new Platform SDK (3790SDK1830)

    export REQUIRE_VS2003AMD64=1

    include $(SRCBASE)/makefiles/env.mk

    export CPU     = AMD64
    export MSVCVer = Win64
    export MSSDK   = $(3790SDK1830)
    export Mstools = $(3790SDK1830)
    export MSVCDir = $(VS2003AMD64)

    VS2003.INCLUDE = $(VS2003AMD64)/atlmfc/include; \
                     $(VS2003AMD64)/include; \
                     $(3790SDK1830)/Include; \
                     $(3790SDK1830)/Include/crt; \
                     $(3790SDK1830)/Include/crt/sys; \
                     $(3790SDK1830)/Include/mfc; \
                     $(3790SDK1830)/Include/atl

    ## NEWPATH var in env.mk prefixes x86_amd64 path to NEWPATH
    NEWPATH       += $(shell cygpath -u $(VS2003AMD64))/bin/win64/x86/AMD64
    NEWPATH       += $(shell cygpath -u $(VS2003AMD64))/bin
    MYVSINCLUDE   := $(VS2003.INCLUDE);$(MYVSINCLUDE)
    MYVSLIB       := $(VS2003AMD64.LIBPATH);$(MYVSLIB)
  endif # BUILD_AMD64
endif # VSVER == VS2003

## HND Additions to Microsoft Visual Studio .NET 2003 paths
NEWPATH          += $(MYVSPATH) $(MYCYGPATH)
NEWPATH_X64      += $(MYVSPATH_X64) $(MYCYGPATH)
ifeq ($(VSVER),VS2003)
  NEWINCLUDE     += $(MYVSINCLUDE);$(WDM2600DDK)/inc/wxp
  NEWLIB         += $(MYVSLIB);$(WDM2600DDK)/lib/wxp/i386
  NEWINCLUDE     := $(subst //,/,$(NEWINCLUDE))
  NEWLIB         := $(subst //,/,$(NEWLIB))
  INCLUDE        := $(subst /,\,$(strip $(NEWINCLUDE)));$(INCLUDE)
  LIB            := $(subst /,\,$(strip $(NEWLIB)));$(LIB)
else
  NEWINCLUDE     += $(MYVSINCLUDE)$(if $(REQUIRE_WDM2600),;$(WDM2600DDK)/inc/wxp)
  NEWLIB         += $(MYVSLIB)$(if $(REQUIRE_WDM2600),;$(WDM2600DDK)/lib/wxp/i386)
  NEWLIBPATH     += $(MYLIBPATH)

  NEWINCLUDE     := $(subst //,/,$(NEWINCLUDE))
  NEWLIB         := $(subst //,/,$(NEWLIB))
  INCLUDE        := $(subst /,\,$(strip $(NEWINCLUDE)))
  LIB            := $(subst /,\,$(strip $(NEWLIB)))
  LIBPATH        := $(subst /,\,$(strip $(NEWLIBPATH)));$(LIBPATH)

  # x64 configurations
  NEWLIB_X64     += $(MYVSLIB_X64)
  NEWLIB_X64     := $(subst //,/,$(NEWLIB_X64))
  NEWLIBPATH_X64 += $(MYLIBPATH_X64)
  LIB_X64        := $(subst /,\,$(strip $(NEWLIB_X64)))
  LIBPATH_X64    := $(subst /,\,$(strip $(NEWLIBPATH_X64)));$(LIBPATH)
endif

## Final paths
PATH             := $(subst $(space),:,$(strip $(shell cygpath -p $(NEWPATH)))):$(PATH)
INCLUDE          := $(subst ; ,;,$(INCLUDE))
LIB              := $(subst ; ,;,$(LIB))
LIBPATH          := $(subst ; ,;,$(LIBPATH))

# x64 configurations
PATH_X64         := $(subst $(space),:,$(strip $(shell cygpath -p $(NEWPATH_X64)))):$(PATH)
LIB_X64          := $(subst ; ,;,$(LIB_X64))
LIBPATH_X64      := $(subst ; ,;,$(LIBPATH_X64))

## Export variables for subsequent builds
export PATH
export INCLUDE
export LIB
export LIBPATH
export PATH_X64
export LIB_X64
export LIBPATH_X64
export VCINSTALLDIR
export VSINSTALLDIR
export MyWindowsSDKDir
export MYWINDOWSSDKDIR
export MYWINDOWSSDKPATCHESDIR
export MYCYGPATH

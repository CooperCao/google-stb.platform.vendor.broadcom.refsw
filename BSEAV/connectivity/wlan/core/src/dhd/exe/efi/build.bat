@echo off

REM #
REM # $ Copyright Broadcom Corporation $
REM # <<Broadcom-WL-IPTag/Proprietary:>>
REM # All rights reserved.
REM #

REM #
REM # Environment setup for EFI-DHD utility build
REM #
REM
REM # SDK_BUILD_ENV
REM #        bios32 : bios32 environment
REM #	     nt32   : nt32 environment
REM #        sal64  : sal64 environment
REM #        em64t  : em64t environment
REM
REM # SDK_INSTALL_DIR
REM #        EFI Toolkit source directory
REM
REM # EFI_APPLICATION_COMPATIBILITY
REM #        EFI_APP_102        : Write application for EFI 1.02
REM #        EFI_APP_110        : Write application for EFI 1.10
REM #        EFI_APP_MULTIMODAL : Write application for multiple EFI version
REM #

if not defined WLAN_WINPFX (set WLAN_WINPFX=Z:)

@rem Look for EDK in a conventional local dir before defaulting to network.
if not defined EDK_BASE (
    set EDK_BASE=C:\tools\EFI
    if not exist %EDK_BASE%\ (set EDK_BASE=%WLAN_WINPFX%\projects\hnd\tools\win\EFI)
)
if not defined MASMPATH (set MASMPATH=%EDK_BASE%\EDK_1_02\MASM611)
if not defined EDK_SOURCE (set EDK_SOURCE=%EDK_BASE%\EDK_1_02\Edk)

@rem Look for DDK in a conventional local dir before defaulting to network.
if not defined WIN_DDK_PATH (
    set WIN_DDK_PATH=C:\tools\msdev\3790ddk1830
    if not exist %WIN_DDK_PATH%\ (set WIN_DDK_PATH=%WLAN_WINPFX%\tools\msdev\3790ddk1830)
)
set PATH=%WIN_DDK_PATH%\bin\x86;%PATH%
set MSSdk=%WIN_DDK_PATH%;%MSSdk%

set SDK_BUILD_ENV=bios32
set SDK_INSTALL_DIR=%EDK_BASE%\EFI_Toolkit_2.0
set EFI_APPLICATION_COMPATIBILITY=EFI_APP_110
set EFI_DEBUG=YES

set clean=
echo.

if "%1"=="" (
  echo Usage: build [bios32^|nt32^|sal64^|em64t]
  echo SDK_BUILD_ENV is set to em64t by default
  echo.
) ELSE (
  if "%1"=="clean" (
     set clean="clean"
  ) ELSE (
	  set SDK_BUILD_ENV=%1
  )
)

if "%2"=="clean" (
   set clean="clean"
)

set EDK_SOURCE
set EFI_APPLICATION_COMPATIBILITY
set EFI_DEBUG
set MSSdk
set PATH
set SDK_BUILD_ENV
set SDK_INSTALL_DIR
set WIN_DDK_PATH
set WLAN_WINPFX=
echo.
set MAKEFLAGS=
del /Q .\obj\em64t\bin\dhd*
del /Q .\obj\em64t\
nmake /nologo -f dhd.mak %clean%

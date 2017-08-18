@echo off

REM This command file has one optional parameter:
REM %1 - Relative path to where Cygwin is installed. It must not include drive letter,
REM      must use '/' as directory separator, and must not start or end with a slash.
REM      Default is tools/win32.
REM This can be overridden by environment setting.

set BUILD_DRIVE=C:
if '%WLAN_WINPFX%' == '' set WLAN_WINPFX=Z:

if '%CYGWIN_DRIVE%' == '' set CYGWIN_DRIVE=%BUILD_DRIVE%

if '%CYGWIN_DIRECTORY%' == '' set CYGWIN_DIRECTORY=%1
if '%CYGWIN_DIRECTORY%' == '' set CYGWIN_DIRECTORY=tools/win32

REM The following section is for the legacy Cygwin 1.5 version only.
if '%CYGWIN_DIRECTORY:tools/win32=%' == '%CYGWIN_DIRECTORY%' goto MODERN

REM Find cygpath in /bin (standard) or /usr/local/bin (very old).
set INITIAL_PATH=%PATH%
set PATH=%CYGWIN_DRIVE%\%CYGWIN_DIRECTORY%\bin;%CYGWIN_DRIVE%\%CYGWIN_DIRECTORY%\usr\local\bin;%PATH%
for /f %%x in ('cygpath -w %CYGWIN_DRIVE%/%CYGWIN_DIRECTORY%/bin') do set CYGWIN_BIN_PATH=%%x
if not '%CYGWIN_BIN_PATH%' == '' set PATH=%CYGWIN_BIN_PATH%;%INITIAL_PATH%
set CYGWIN_BIN_PATH=
set INITIAL_PATH=

set VIM=%BUILD_DRIVE%/tools/vim/vim72

goto EXPORTS

:MODERN

REM REM Remove old Cygwin 1.5 dirs from PATH in modern (1.7.17+) Cygwins.
set PATH=%PATH:C:\tools\win32\bin;=%
set PATH=%PATH:C:\tools\win32\usr\local\bin;=%

REM Remove paths to native tools which were needed with 1.5 but
REM can be replaced with bundled Cygwin versions now.
set PATH=%PATH:C:\tools\Python;=%
set PATH=%PATH:C:\tools\Subversion;=%
set PATH=%PATH:C:\tools\vim\vim72;=%
set PATH=%PATH:C:\tools\vim\vim73;=%

REM Put the modern Cygwin bin on PATH.
REM TODO Is this really needed given we come in via bash which
REM ought to place /usr/bin on PATH?
set PATH=/usr/bin;%PATH%

:EXPORTS

set INCLUDE=C:\tools\msdev\VS2003\SDK\v1.1\include;C:\tools\msdev\PlatformSDK\Include;C:\tools\msdev\RtmSDK60\VC\INCLUDE;C:\tools\msdev\RtmSDK60\INCLUDE
set LIB=C:\tools\msdev\VS2003\SDK\v1.1\Lib;C:\tools\msdev\RtmSDK60\VC\LIB;C:\tools\msdev\RtmSDK60\LIB

REM This set may be unnecessary. They don't seem to be in the environment
REM of legacy builds anyway.
set MSDEV=%BUILD_DRIVE%/tools/msdev/Studio
set MSDEVDIR=%BUILD_DRIVE%\tools\msdev\Studio\MSDev98
set MSDEV_DOS=%BUILD_DRIVE%\tools\msdev\Studio
set MSSDK=%BUILD_DRIVE%/tools/msdev/PlatformSDK
set MYWINDOWSDDKDIR=%BUILD_DRIVE%\TOOLS\MSDEV\7600WDK\
set MYWINDOWSSDKDIR=%MYWINDOWSDDKDIR%

set NTICE=%WLAN_WINPFX%/projects/hnd_tools/win/numega/SoftICE/

REM  Follow the old model for now: TEMP gets a legal Windows path into C:\Temp
REM  whereas TMP should end up with /cygdrive/c/DOCUME~1/%USER%/LOCALS~1/Temp
REM  and TMPDIR will point to the traditional Unixy /tmp.
REM  Preferably we'd set TMP=%TEMP%.
REM  TODO Clean this up once stable.
set TEMP=%BUILD_DRIVE%/temp
REM set TMP=%TEMP%
set TMPDIR=/tmp

set VERISIGN_PASSWORD=brcm1

REM Legacy builds only go up through VS90, implying that these may be obsolete.
set VS71COMNTOOLS=%BUILD_DRIVE%\Tools\msdev\VS2003\Common7\Tools\
set VS80COMNTOOLS=%BUILD_DRIVE%\Tools\msdev\VS2005\Common7\Tools\
set VS90COMNTOOLS=%BUILD_DRIVE%\Tools\msdev\VS2008\Common7\Tools\
set VS100COMNTOOLS=%BUILD_DRIVE%\Tools\msdev\VS2010\Common7\Tools\
set VS110COMNTOOLS=%BUILD_DRIVE%\tools\msdev\vs2012\Common7\Tools\
set VS120COMNTOOLS=%BUILD_DRIVE%\Tools\Msdev\VS2013\Common7\Tools\

REM This could be missing when coming from LSF and maybe ssh
REM but fortunately it never changes anymore.
set OS=Windows_NT

set WDK_OACR=no_oacr

REM Unset temporary variables.
set BUILD_DRIVE=
set CYGWIN_DRIVE=

:DONE

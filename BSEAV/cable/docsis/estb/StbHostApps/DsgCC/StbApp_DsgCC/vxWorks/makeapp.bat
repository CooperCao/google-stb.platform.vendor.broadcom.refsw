@echo off

:: File:       makeapp.bat
:: Author:     John McQueen
:: Modified by Mike Sieweke and others

if .%OS% == .Windows_NT setlocal			 
goto skiphelp

:print_help
echo This batch file is used to build the vxWorks modem application.
echo Options can be entered in any order or not present; however, 
echo a board must be present.
echo.
echo Usage: makeapp board [type] [options...] [target]
echo.
echo Parameters:
echo     [board]    = %option_board_list%  ( no default )
echo     [type]     = us or eu                                ( default us)
echo     nobpi      = omit bpi support
echo     vendor     = compile in vendor support
echo     factory    = add factory MIB support ( default except for vendor/cablehome )
echo     brcmibs    = add Broadcom runtime MIB support ( default )
echo     t21        = use Tornado 2.1 tools
echo     t22        = use Tornado 2.2 tools
echo     debug      = Tornado tools support
echo     slim       = build reduced-size image without message logging
echo     nopropane  = omit Propane support
echo     bcm80211   = add 802.11 HAL support
echo     bcm80211g  = add 802.11g HAL support
echo     bcm80211gwpa  = add 802.11g HAL support with WPA support.
echo     bluetooth  = add Bluetooth support
echo     [no]hpna   = add or omit HPNA support
echo     nokerb     = omit kerberos support for cablehome
echo     bsafe      = use bsafe library                       ( default openssl )
echo     cablehome  = CableHome support
echo     emta       = add EMTA support                        ( default for 3351/2 )
echo     ipsec      = add IPSEC support
echo     vpn        = add VPN support
echo     docsis DN  = set Docsis support to DN (10, 11, or 20)
echo     show       = show build parameters, but don't build
echo     c2         = use MiniLZO compression
echo     c3         = use NRV2D99 compression - best (slowest)
echo     c6         = use NRV2D99 compression - fastest       ( default )
echo     c9         = use NRV2D99 compression - better (slower)
echo     c4         = use LZMA compression
echo     mac14 CW   = Build MAC14 test image for cert wave CW
echo     sto   REV  = Specify revision string for programstore in 2.4.0a format.
echo     pid   PID  = use PID (4 hex digits) as ProgramStore signature
echo     board BOARD= use ^<BOARD^>_NonVolDefaults.h from Custom directory
echo     manageswitch = support port management on 5325 e-net switch
echo     standalone = CableHome standalone PS
echo     telnet     = Telnet Server console support
echo	 parental   = Online lookup service support for HTTP content filter
echo     cablemedeart   = CableMedea Retail
echo     swreset    = SW reset-to-factory support
echo     [no]sled   = support (or don't) eDOCSIS SLED. Default=yes for CH/EMTA, no otherwise.
echo     [target]   = Alternate build target                  ( default vxWorks )
echo         vxWorks.st - builds vxWorks with symbol table
echo         clean      - clean build directory
echo.
echo Examples:
echo     makeapp 3345 slim c3 - Build for 3345, using maximum compression.
echo     makeapp 3350 eu      - Build for European 3350, using normal compression.
echo     makeapp 3350 core.s  - Compile core.c to assembler listing.
echo     makeapp 3345 us mac14 20 sto 2.4.0c
echo     makeapp 3348 docsis 20 - Build with advanced phy enabled.
goto end

:skiphelp 

:: Look for a board-specific options file in in the customization directory.  It
:: can define a list of command-line options in the "board_options" variable.
:: The new way is to have a <board>_settings.bat file which defines both bsp and
:: app options in variables "bsp_options" and "app_options".
set board_options=
set app_options=
set customfile=
if exist ..\..\Custom\%1makeapp.bat   set customfile=..\..\Custom\%1makeapp.bat
if exist ..\..\Custom\%1_settings.bat set customfile=..\..\Custom\%1_settings.bat
if "%customfile%" == "" goto endif
   call %customfile%
   :: In case the old-style defaults were used...
   if "%app_options%" == "" set app_options=%board_options%
   echo Added parameters for %1 = %app_options%
   shift
:endif

:: List of valid board targets, separated by commas
set option_board_list=3345, 3348, 3350, 3351, 3352, 3360, 3730, 4710, 7110, 7111, 7038

:: Options which default to "no", "yes", or "default".
set option_no_list=emtasim ipsec vpn c0 c1 c2 c3 c4 c6 c9 slim applib bluetooth bootl show vendor t21 debug 80211 80211lib bsafe oms cs links cablehome bcm80211 bcm80211g bcm80211gwpa is80211 chnolib t22 check standalone cacheopt parental ps cablemedeart swreset bcm8021x dsg bcm3418 propane 
set option_yes_list=bpi snmp hh color cmhal kerb quiet 16m dsgcc
set option_default_list=hpna emta factory brcmibs telnet wmsnmp manageswitch sled  pvnet

:: Options which take a required parameter.
set option_parm_list=sto mac14 c board pid docsis

:: We can't use the "call" command with a "/?" parameter, so check it.  All
:: other help strings are handled in parse_params.bat.
if "%1" == "/?" goto print_help

:: Pull all the parameters into a single variable.  This is necessary because
:: the earler "shift" statement doesn't affect %*, so we can't pass %* to
:: parse_params.  Also, there may be more than 9 parameters, so we can't just
:: list the parameter as in %1 %2 %3 ...
set full_parms=%app_options%
:fullparmloop
   set full_parms=%full_parms% %1
   shift
if not "%1" == "" goto fullparmloop

:: Parse and verify the parameters.  This does most of the work of parsing.
call ..\..\cm_bsp_v2\bsp_common\os\parse_params %full_parms%

if "%parse_status%" == "help"  goto print_help
if "%parse_status%" == "error" goto end_error

set TARGET=%p_option%

:: If a target was not specified, use "vxWorks"
if "%TARGET%" == "" set TARGET=vxWorks

:: Accept "vxWorks.st" in all lower case.
if "%TARGET%" == "vxworks.st" set TARGET=vxWorks.st

:: If "libcablehome" is the target, turn on the cablehome option.
if "%TARGET%" == "libcablehome" set opt_cablehome=yes

:: If "libfirewall" is the target, turn on the cablehome option.
if "%TARGET%" == "libfirewall" set opt_cablehome=yes

:: If "libbcmnat" is the target, turn on the cablehome option.
if "%TARGET%" == "libbcmnat" set opt_cablehome=yes

:: Verify parameters for options that take parameters.

:: Check mac14 cert wave value.
if "%opt_mac14%" == "no" goto endif
   set cw_ok=no
   for /l %%w in (7,1,30) do if "%%w" == "%opt_mac14_parm%" set cw_ok=yes
   if "%cw_ok%" == "yes" goto endif
   echo Error: Cert wave (%opt_mac14_parm%) must be an integer from 7 to 30.
   goto end_error
:endif

:: The default compression level for ProgramStore is 36 (NRV2D99, level 6).
set ps_compression=36
if "%opt_c0%" == "yes" set ps_compression=0
if "%opt_c1%" == "yes" set ps_compression=1
if "%opt_c2%" == "yes" set ps_compression=2
if "%opt_c3%" == "yes" set ps_compression=3
if "%opt_c6%" == "yes" set ps_compression=36
if "%opt_c9%" == "yes" set ps_compression=39
if "%opt_c4%" == "yes" set ps_compression=4

:: Check if user type "c #" to set compression type.
if "%opt_c%" == "no" goto endif
   set ps_compression=%opt_c_parm%
   set c_ok=no
   for %%c in (0 1 2 3 4 31 32 33 34 35 36 37 38 39) do if "%%c" == "%opt_c_parm%" set c_ok=yes
   if "%c_ok%" == "yes" goto endif
   echo Error: Compression type (%opt_c_parm%) must be an integer from 0-3 or 31-39.
   goto end_error
:endif

:: Check programstore revision.
if %opt_sto% == no goto else
   call ..\..\check_rev.bat %opt_sto_parm%
   if %rev_ok% == no goto end
   set sto=%rev_num%
   goto endif
:else
   set sto=002.000
   set rev_std=2.x.x
:endif

:: Verify 802.11 options
if "%opt_80211%"    == "yes" set opt_bcm80211g=yes
if "%opt_80211lib%" == "yes" set opt_is80211=yes

set count_80211=0
if "%opt_bcm80211%"  == "yes"  set /a count_80211=count_80211 + 1
if "%opt_bcm80211gwpa%" == "yes"  set /a count_80211=count_80211 + 1
if "%opt_bcm80211g%" == "yes"  set /a count_80211=count_80211 + 1
if "%opt_is80211%"   == "yes"  set /a count_80211=count_80211 + 1

if %count_80211% LEQ 1 goto endif
   echo Error: You must specify only one of bcm80211, bcm80211g, bcm80211gwpa, or is80211.
   goto end_error
:endif

if %opt_bcm8021x% == yes (
   if %opt_bcm80211gwpa% == no (
      echo Error: You must specify bcm80211g or bcm80211gwpa with bcm8021x.
      goto end_error
   )
)

:: Verify Docsis parameter
if "%opt_docsis%" == "no" goto endif
   if "%opt_docsis_parm%" == "1.0" set opt_docsis_parm=10
   if "%opt_docsis_parm%" == "1.1" set opt_docsis_parm=11
   if "%opt_docsis_parm%" == "2.0" set opt_docsis_parm=20
   if "%opt_docsis_parm%" == "10" goto endif
   if "%opt_docsis_parm%" == "11" goto endif
   if "%opt_docsis_parm%" == "20" goto endif
      echo Error: The docsis parameter must be 10, 11, or 20.
      goto end_error
:endif

:: Set additional CableMedea Retail flags
if "%opt_cablemedeart%" == "yes" set opt_slim=yes& set opt_cablehome=yes& set opt_kerb=no& set ps_compression=4
:endif

:: Look for the WindManage include directory to decide whether to build with
:: WindManage or Epilogue.
if not "%opt_wmsnmp%" == "default" goto endif
   if "%WINDMANAGE_ROOT%" == "" goto else
      set opt_wmsnmp=yes
      echo ---- Defaulting to WindManage SNMP
      goto endif
   :else
      set opt_wmsnmp=no
      echo ---- Defaulting to Epilogue SNMP
:endif


:: ========================================================================
:: In case the user specified more than one board, this is the start of the
:: loop to process all boards.
:: ========================================================================
:major_loop
echo :major_loop

set board=
:: Find the first board in the list not yet processed.
if "%do_7038%" == "yes" set board=7038
if "%do_7111%" == "yes" set board=7111
if "%do_7110%" == "yes" set board=7110
if "%do_3730%" == "yes" set board=3730
if "%do_3360%" == "yes" set board=3360
if "%do_3352%" == "yes" set board=3352
if "%do_3351%" == "yes" set board=3351
if "%do_3350%" == "yes" set board=3350
if "%do_3345%" == "yes" set board=3345
if "%do_3348%" == "yes" set board=3348
if "%do_4710%" == "yes" set board=4710

:: If we already made them all, board == "" and we are done.
if "%board%" == "" goto end

:: Flag the current board so we won't try to do it again.
set do_%board%=no

:: Set an output string for the build target.
set buildtarget=%board%
if "%opt_board%" == "yes" set buildtarget=%opt_board_parm%


:: Place default parameters here...   All local variables should be lower
:: case.  Variables used in the make files should be upper case.
if "%opt_color%" == "yes" color
set MAKE_BOARD=%board%
set CPU=RC32364
set TOOL=sfgnu
set TORNADO_VERSION=20
set VXWORKS_SYMS_INCLUDED=0
set TARGETOS=vxWorks

set m_error=ok

set BCM_CABLEHOME_SUPPORT=0
set BCM_CABLEHOME_USE_LIBRARY=0
set BCM_FIREWALL_USE_LIBRARY=0
set BCM_BRCMNAT_USE_LIBRARY=0
set BCM_KERBEROS_SUPPORT=0
set CACHE_WRITEBACK_MODE=0
set BCM_REDUCED_IMAGE_SIZE=0
set BCM_BOOT_LOADER_SUPPORT=0
set BCM_BALIB_CHANGE=
set BCM_VENDOR_SUPPORT=0
set BCM_VENDOR_NONVOL_RESET=0
set BCM_SW_RESET_DEFAULTS=0
set BCM_FACTORY_SUPPORT=1
set BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=0
set BCM_RUNTIMEMIB_SUPPORT=1
set BCM_OLD_BOARDREV=0
set BCM_FIREWALL_ENABLED=0
set BCM_VENDOR_OMS=0
set BCM_RAM_SIZE=8
set MANAGE_SWITCH_INCLUDED=0
set CABLEHOME_STANDALONE_PS=0
set BCM_QUIET=1

set BPI_SUPPORT_HARDWARE=0
set EMAC_CAM_INCLUDED=0

set CM_HAL_INCLUDED=0
set HPNA_HAL_INCLUDED=0
set USB_HAL_INCLUDED=0
set BLUETOOTH_HAL_INCLUDED=0
set WIFI80211_HAL_INCLUDED=0
set BCM_80211=0
set BCM_80211B=0
set BCM_8021X_SUPPORT=0

:: BCM_IPSEC_SUPPORT/BPI_SUPPORT= (1) SUPPORTED (0) NOT SUPPORTED
set BPI_SUPPORT=0
set BCM_IPSEC_SUPPORT=0
set BCM_VPN_SUPPORT=0

::  SNMP_SUPPORT= (0) MEANS NO SNMP  (2) SNMPv2 & (3) SNMPv3
set SNMP_SUPPORT=0
::  SNMP_WINDMANAGE= (1) if using WindManage9.3 SNMP library instead of Epilogue
set SNMP_WINDMANAGE=0

:: SOFTWARE_CRYPTO_LIB = OPENSSL (OpenSSL library), BSAFE (Bsafe library)
set SOFTWARE_CRYPTO_LIB=OPENSSL
:: Clear this variable for those who use the pSOS setup script.
:: OPENSSL_DIR is set in torvars.bat set OPENSSL_DIR=
set OPENSSL_LIBNAME=libOpenSSLvxworks.a
set SEC_LIBNAME=SecLibVxworks.a

set TORNADO_DEBUGGING=0
set XTALFREQ=28000000

set EMTA_SUPPORT=0
set EMTA_SIMULATOR_SUPPORT=0
     
set PROPANE_LIBNAME=Propane05VxWorks.a

set BCM_3418_SUPPORT=0

set USE_PRIVATE_NETWORK=0

:: ------------------------------------------------
:: Override defaults with command-line parameters.
:: ------------------------------------------------

:: Check the Tornado "README.TXT" file to find which version we're running.
:: Default to 2.0 in case the file doesn't exist.
set TORNADO_VERSION=2.0
if not exist %WIND_BASE:/=\%\README.TXT goto endif
   :: Pick off the second and third token of the first line of the file.  Token
   :: 2 must be "Tornado" and token 3 must be 2.0, 2.1, or 2.2.
   set token2=
   for /f "tokens=2,3" %%i in (%WIND_BASE:/=\%\README.TXT) do set token2=%%i& set token3=%%j& goto endfor
   :endfor
   if not "%token2%" == "Tornado" goto endif
   if not "%token3:~0,1%" == "2" goto endif
      set TORNADO_VERSION=%token3%
:endif

:: Remove the decimal point from the version number.
set TORNADO_VERSION=%TORNADO_VERSION:.=%
if not "%TORNADO_VERSION%" == "20" set CPU=MIPS32

if %opt_bsafe%       == yes   set SOFTWARE_CRYPTO_LIB=BSAFE
if %opt_bpi%         == no    set BPI_SUPPORT=0

if %opt_t21%         == yes   set TORNADO_VERSION=21&  set CPU=MIPS32
if %opt_t22%         == yes   set TORNADO_VERSION=22&  set CPU=MIPS32

if %opt_debug%       == yes   set TORNADO_DEBUGGING=1

if %opt_slim%        == yes   set BCM_REDUCED_IMAGE_SIZE=1

if %opt_bcm80211%    == yes   set WIFI80211_HAL_INCLUDED=1& set BCM_80211=1& set BCM_80211B=1& set HAL80211_LIBNAME=lib80211
if %opt_bcm80211gwpa% == yes   set WIFI80211_HAL_INCLUDED=1& set BCM_80211=1& set HAL80211_LIBNAME=lib80211gwpa
if %opt_bcm80211g%   == yes   set WIFI80211_HAL_INCLUDED=1& set BCM_80211=1& set HAL80211_LIBNAME=lib80211g
if %opt_is80211%     == yes   set WIFI80211_HAL_INCLUDED=1& set BCM_80211=0& set HAL80211_LIBNAME=lib80211
if %opt_bcm8021x%    == yes   set BCM_8021X_SUPPORT=1

if %opt_bootl%       == yes   set BCM_BOOT_LOADER_SUPPORT=1

if %opt_ipsec%       == yes   set BCM_IPSEC_SUPPORT=1

if %opt_vpn%         == yes   set BCM_VPN_SUPPORT=1

if %opt_hpna%        == no    set HPNA_HAL_INCLUDED=0

if %opt_16m%         == yes   set BCM_RAM_SIZE=16

if %opt_links%       == yes   set BCM_OLD_BOARDREV=1& set BCM_VENDOR_NONVOL_RESET=1

if %opt_chnolib%     == yes   set opt_cablehome=yes& set BCM_CABLEHOME_USE_LIBRARY=0& set BCM_FIREWALL_USE_LIBRARY=0
if %opt_cablehome%   == yes   set BCM_CABLEHOME_SUPPORT=1& set opt_vendor=yes& set BCM_KERBEROS_SUPPORT=1& set MANAGE_SWITCH_INCLUDED=1
if %opt_standalone%  == yes   set BCM_CABLEHOME_STANALONE_PS=1& set CM_HAL_INCLUDED=0
if %opt_kerb%        == no    set BCM_KERBEROS_SUPPORT=0
if %opt_cablemedeart%    == yes   set BCM_CABLEMEDEA_RETAIL_SUPPORT=1& set BCM_CABLEHOME_USE_LIBRARY=1& set BCM_FIREWALL_USE_LIBRARY=1& set MANAGE_SWITCH_INCLUDED=0

if %opt_manageswitch% == yes  set MANAGE_SWITCH_INCLUDED=1
if %opt_manageswitch% == no   set MANAGE_SWITCH_INCLUDED=0


if %opt_emta%        == yes   set EMTA_SUPPORT=1
if %opt_emtasim%     == yes   set EMTA_SIMULATOR_SUPPORT=1

if %opt_cmhal%       == no    set CM_HAL_INCLUDED=0

if %opt_vendor%      == yes   set BCM_VENDOR_SUPPORT=1& set BCM_FACTORY_SUPPORT=0
if %opt_oms%         == yes   set BCM_VENDOR_OMS=1

if %opt_factory%     == no    set BCM_FACTORY_SUPPORT=0
if %opt_brcmibs%     == no    set BCM_RUNTIMEMIB_SUPPORT=0
if %opt_factory%     == yes   set BCM_FACTORY_SUPPORT=1
if %opt_brcmibs%     == yes   set BCM_RUNTIMEMIB_SUPPORT=1
if %opt_snmp%        == no    set SNMP_SUPPORT=0
if %opt_snmp%        == no    set BCM_FACTORY_SUPPORT=0
if %opt_snmp%        == no    set BCM_RUNTIMEMIB_SUPPORT=0
if %opt_wmsnmp%      == yes   set SNMP_WINDMANAGE=1

if %opt_quiet%       == no    set BCM_QUIET=0

if %opt_parental%    == yes   set BCM_PARENTALCTRL_INCLUDED=1
if %opt_swreset%     == yes   set BCM_SW_RESET_DEFAULTS=1

if %opt_dsg%         == yes   set BCM_DSGCC_SUPPORT=1

if %opt_bcm3418%     == yes   set BCM_3418_SUPPORT=1
			
      
:: The default for telnet support depends on whether we are building slim or
:: fat.  The default is no telnet support for slim; full telnet support for
:: fat.
if %opt_slim% == no  set BCM_TELNET_SUPPORT=1
if %opt_slim% == yes set BCM_TELNET_SUPPORT=0

if %opt_sled%        == yes   set BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=1
if %opt_sled%        == no    set BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=0
      
:: If the user explicity specified telnet support enable or disable, then
:: we need to override the default.  If the [no]telnet parameter was not
:: specified at all, then leave the default alone.
if %opt_telnet%   == no    set BCM_TELNET_SUPPORT=0
if %opt_telnet%   == yes   set BCM_TELNET_SUPPORT=1


:: If we have a Propane library in LibSupport, override the environment variable.
if not exist ../../LibSupport/Propane/vxWorks/*.a goto endif
   :: Set a variable equal to the root (../..).
   for %%r in (../..) do set root=%%~fr
   :: Turn backslashes into forward slashes.
   set PROPANE_LIB_PATH=%root:\=/%/LibSupport/Propane/vxWorks
:endif

:: If the user didn't specify a propane option, see if the library exists...
if not "%opt_propane%" == "default" goto endif
   set opt_propane=no
   :: Enable propane by default only if the library directory exists.
   if not "%PROPANE_LIB_PATH%" == "" if exist %PROPANE_LIB_PATH%/ set opt_propane=yes
:endif
if %opt_propane%  == no    set PROPANE_INCLUDED=0
if %opt_propane%  == yes   set PROPANE_INCLUDED=1

if %opt_pvnet%  == yes   set USE_PRIVATE_NETWORK=1

if "%TARGET%" == "vxWorks.st" set VXWORKS_SYMS_INCLUDED=1

set PROGRAM_STORE_SIGNATURE=0x%board%
:: Unset bsp_dir.  Somehow this gets set in lower case by some external script.
set bsp_dir=
set BSP_DIR=bsp_bcm9%board%
::set LIB_EXT=vxWorks%board%_BSP_%country%.a
set HPNA_LIB=libhpna_BCM9%board%.a

:: The default target (build) directory is named after the chip ID.  If the
:: user specified a "board" parameter, this replaces the directory name.
set TARGETDIR=bcm9%board%
if "%opt_board%" == "no" goto endif
   set TARGETDIR=%opt_board_parm%
:endif

:: Set TARGETDIR appropriately for different build types
if %opt_slim%      == yes  set TARGETDIR=%TARGETDIR%_slim&      goto skip
if %opt_cablehome% == yes  set TARGETDIR=%TARGETDIR%_cablehome& goto skip
if %opt_vendor%    == yes  set TARGETDIR=%TARGETDIR%_vendor&    goto skip
if %opt_propane%   == yes  set TARGETDIR=%TARGETDIR%_propane
:skip

if exist %TARGETDIR%\ goto endif
   echo Warning: Build directory %TARGETDIR% does not exist.  Creating...
   echo ==== mkdir %TARGETDIR%
             mkdir %TARGETDIR%
:endif

:: ------------------------------------------------
:: Goto the board-specific settings.
:: ------------------------------------------------
goto build_%board%
:: We only fall through to here if the board is invalid.
goto BuildFailed

:: ------------------------------------------------------------------
:: Board-specific settings.
:: ------------------------------------------------------------------

:: 3345 Build Settings
:build_3345
   set HPNA_LIB=
   set DOCSIS_CORE=DOCSIS_CORE_BCM334x
   set BPI_SUPPORT_HARDWARE=0
   set HPNA_HAL_INCLUDED=0
   set EMAC_CAM_INCLUDED=0
   set CACHE_WRITEBACK_MODE=1
   set CMHAL_TARGET=bcm3345
   goto begin

:: 3348 Build Settings
:build_3348
   set DOCSIS_CORE=DOCSIS_CORE_BCM3348
   set XTALFREQ=24000000
   set BPI_SUPPORT_HARDWARE=0
   if not "%opt_hpna%" == "yes" set HPNA_HAL_INCLUDED=0
   set EMAC_CAM_INCLUDED=0
   set CACHE_WRITEBACK_MODE=1
   
   set HPNA_LIB=libhpna_BCM93350.a
   
   if %HPNA_HAL_INCLUDED%    == 1 set CACHE_WRITEBACK_MODE=0
   if %BCM_80211B% == 1 set CACHE_WRITEBACK_MODE=0
   set CMHAL_TARGET=bcm3348
   set INTEGRATED_LED_CONTROL=1
   goto begin
	  
:: 3350 Build Settings
:build_3350
   if %country% == us  set XTALFREQ=24000000
   set BCM_BALIB_CHANGE=1
   set DOCSIS_CORE=DOCSIS_CORE_BCM335x
   set CMHAL_TARGET=bcm335x
   goto begin

:: 3351 Build Settings.  The 3351 make job uses some 3352 components.
:build_3351
   if %country% == us  set XTALFREQ=27000000
   set DOCSIS_CORE=DOCSIS_CORE_BCM335x
   set BPI_SUPPORT_HARDWARE=0
   set BSP_DIR=bsp_bcm93352
   set HPNA_LIB=libhpna_BCM93352.a
   set CMHAL_TARGET=bcm335x
   set EMTA_SUPPORT=1
   if %opt_emta% == no set EMTA_SUPPORT=0
   goto begin

:: 3352 Build Settings
:build_3352
   if %country% == us  set XTALFREQ=27000000
   set DOCSIS_CORE=DOCSIS_CORE_BCM335x
   set CMHAL_TARGET=bcm335x
   set EMTA_SUPPORT=1
   if %opt_emta% == no set EMTA_SUPPORT=0
   goto begin

:: 3360 Build Settings
:build_3360
   set DOCSIS_CORE=DOCSIS_CORE_BCM336x
   set CACHE_WRITEBACK_MODE=1
   :: Disable cache writeback if HPNA or 802.11 Hals are enabled.
   if %HPNA_HAL_INCLUDED%    == 1 set CACHE_WRITEBACK_MODE=0
   if %BCM_80211B% == 1 set CACHE_WRITEBACK_MODE=0
   set CMHAL_TARGET=bcm3360
   goto begin

:: 3730 Build Settings
:build_3730
:build_7038
   :: Override with settop version
   set DOCSIS_CORE=DOCSIS_CORE_BCM335x
   set CPU=MIPS64
   if "%TORNADO_VERSION%" == "20" set CPU=VR5400
   set TOOL=gnu
   set USB_HAL_INCLUDED=0
   set BPI_SUPPORT_HARDWARE=0
   set HPNA_HAL_INCLUDED=0
   set EMAC_CAM_INCLUDED=0
   set BCM_SETTOP_SUPPORT=1
   set SETTOP_API_INCLUDED=1
   set CMHAL_TARGET=bcm335x
   set VIDEO_INCLUDED=0
   set BCM_DSG_SUPPORT=1
   set DSGDEMO_IPVIDEO_SUPPORT=0
   set DSG_CABLECARD_SUPPORT=0

   set BCM_DSG_SUPPORT=0
   set BCM_DSGCC_SUPPORT=1
   set BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=0
   set IDE_DISK=1
   set BOOT_FROM=CODE_IN_ROM
   set STB_PLATFORM=BCM97038
   set SETTOP_DHCP_DNS_SUPPORT=1
   
   ::To include SPY, uncomment the following line and build for vxWorks.st
   :: set TORNADO_SPY=1
   
   :: only enable 1 of the following 2 DSG Management Apps(MA) sample code
   :: Default is bcm97110 with no MA support
   set DSG_CABLECARD_SUPPORT=0

   :: Enable to include DSG API Test Code
   set BCM_DSG_API_TEST=0
   set BCM_DSG_API_TEST_DUAL_PROCESS=0
   set BCM_DSG_DUAL_PROCESSOR_INTERFACE=1
   set BCM_DSG_DUAL_PROCESS_INTERFACE=0
   set BCM_DUAL_DOWNLOAD_SUPPORT=0
   
   set CLIENT_INTERFACE_SUPPORT=0
   set BCM_TELNET_SUPPORT=0
   
   ::To include SPY, uncomment the following line and build for vxWorks.st
   ::set TORNADO_SPY=1
   
   :: Enable to include DSG API Test Code
   set BCM_DSG_API_TEST=0
   
   :: board specific defines
   if "%board%" == "3730" set QAMVIDEO_LIBNAME=../../../LibSupport/QamPhyApi/vxbcmPIF_bcm9%board%.%CPU%.t%TORNADO_VERSION%.a
   if "%board%" == "7038" set QAMVIDEO_LIBNAME=../../../../Settop_7038/diagvideo_bcm9%board%.a
   if "%board%" == "7038" set BCM_3418_SUPPORT=1
   goto begin

:: 4710 Build Settings
:build_4710
   set DOCSIS_CORE=DOCSIS_CORE_BCM335x
   set CM_HAL_INCLUDED=
   set HPNA_LIB=
   set HPNA_HAL_INCLUDED=0
   set USB_HAL_INCLUDED=0
   set BPI_SUPPORT_HARDWARE=0
   goto begin

:: 7110 Build Settings
:build_7110
:build_7111
   :: Override with settop version
   set CM_HAL_INCLUDED=0
   set STB_VXWORKS=1
   set BCM711X_INTERNAL_AMP=1
   set DOCSIS_CORE=DOCSIS_CORE_BCM334x
   set USB_HAL_INCLUDED=0
   set BPI_SUPPORT_HARDWARE=0
   set BPI_SUPPORT=0
   set HPNA_HAL_INCLUDED=0
   set EMAC_CAM_INCLUDED=0
   set BCM_SETTOP_SUPPORT=1
   set SETTOP_API_INCLUDED=0
   set CMHAL_TARGET=bcm3345
   set SNMP_SOCKET_MODE=0
   set CACHE_WRITEBACK_MODE=1
   set VIDEO_INCLUDED=0
   set BCM_DSG_SUPPORT=0
   set BCM_DSGCC_SUPPORT=1
   set BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=0
   set IDE_DISK=1
   set BOOT_FROM=CODE_IN_ROM
   set STB_PLATFORM=BCM97110
   set SETTOP_DHCP_DNS_SUPPORT=1
   
   ::To include SPY, uncomment the following line and build for vxWorks.st
   :: set TORNADO_SPY=1
   
   :: only enable 1 of the following 2 DSG Management Apps(MA) sample code
   :: Default is bcm97110 with no MA support
   set DSG_CABLECARD_SUPPORT=0

   :: Enable to include DSG API Test Code
   set BCM_DSG_API_TEST=0
   set BCM_DSG_API_TEST_DUAL_PROCESS=0
   set BCM_DSG_DUAL_PROCESSOR_INTERFACE=1
   set BCM_DSG_DUAL_PROCESS_INTERFACE=0
   set BCM_DUAL_DOWNLOAD_SUPPORT=0
   
   set CLIENT_INTERFACE_SUPPORT=0
   set BCM_TELNET_SUPPORT=0   
   ::Specify default QAM video library filename
   if %VIDEO_INCLUDED% == 1 set QAMVIDEO_LIBNAME=../../../../SetTop/BuildLib_diagvideo/diagvideo.a
   
   ::Support DSG HW, overwrite above default settings
   if "%opt_board_parm%" == "bcm97110dsg" goto dsg_overwrite
   if "%opt_board_parm%" == "bcm97110dsg2" goto dsg2_overwrite
   if "%opt_board_parm%" == "bcm97111" goto bcm7111_overwrite
   goto cont_7110
   
:dsg_overwrite
   set QAMVIDEO_LIBNAME=../../../../SetTop/BuildLib_diagvideo/diagvideo_dsg.a
   set IDE_DISK=0
   set BSP_DIR=bsp_bcm9%board%dsg
   set DSG_CABLECARD_SUPPORT=0
   goto cont_7110
   
:dsg2_overwrite   
   set QAMVIDEO_LIBNAME=../../../../SetTop/BuildLib_diagvideo/diagvideo_dsg2.a
   set IDE_DISK=0
   set BSP_DIR=bsp_bcm9%board%dsg2
   set DSG_CABLECARD_SUPPORT=0
   set BOOT_FROM=CODE_IN_FLASH
   set BCM_3418_SUPPORT=1
   goto cont_7110

:bcm7111_overwrite
   @echo :bcm7111_overwrite
   set QAMVIDEO_LIBNAME=../../../../SetTop/BuildLib_diagvideo/diagvideo_3419.a
   set IDE_DISK=0
   set BSP_DIR=bsp_bcm97111
   set DSG_CABLECARD_SUPPORT=0
   set BOOT_FROM=CODE_IN_FLASH
   set DSGDEMO_IPVIDEO_SUPPORT=0
   goto cont_7110
   
:cont_7110
   ::Include jpeg libraries
   if %DSGDEMO_IPVIDEO_SUPPORT% == 1 set QAMVIDEO_LIBNAME=%QAMVIDEO_LIBNAME% ../../../../SetTop/BuildLib_jpeg-6d/vxlibjpeg.a
   
   ::Include CableCard library
   if %DSG_CABLECARD_SUPPORT% == 1 set QAMVIDEO_LIBNAME=%QAMVIDEO_LIBNAME% ../../../../SetTop/BuildLib_diagvideo/podlib_vx_MIPS32.a

   goto begin

:: ------------------------------------------------------------------
:: End board-specific settings.
:: ------------------------------------------------------------------


:: ====================================================================
:: Report build parameters and go.
:: ====================================================================
:begin

:: If T21, T22 or T221, change default libraries name
if "%TORNADO_VERSION%" == "20" goto endif
   set OPENSSL_LIBNAME=libOpenSSLvxworks.%CPU%.t%TORNADO_VERSION%.a
   set SEC_LIBNAME=SecLibVxworks.%CPU%.t%TORNADO_VERSION%.a
   set PROPANE_LIBNAME=Propane05VxWorks.%CPU%.t%TORNADO_VERSION%.a
:: Redirect Openssl library to 0.9.7 version
   if %DSG_CABLECARD_SUPPORT% == 1 set OPENSSL_LIBNAME=../../../SetTop/BuildLib_diagvideo/libOpenSSL097vxworks.%CPU%.t%TORNADO_VERSION%.a
   
   echo ==== Libraries included: %OPENSSL_LIBNAME% ; %QAMVIDEO_LIBNAME%

   if "%SETTOP_API_INCLUDED%" == "1" set rev_std=1.1.0z
:endif

:: To provide the elapsed time for a build, first save the current time.
call ..\..\cm_bsp_v2\bsp_common\os\timer_start

:: There's no way to check whether we have the old or new SNMP directory
:: structure, so we'll hard-code it.  This line should be the only difference
:: between the two versions of makeapp.bat.
set snmp_type=new

if not "%TARGET%" == "clean"   goto endif
   :: Clean build directory.
   cd %TARGETDIR%
   rm -f *.o *.rpo  vxWorks*  bootrom*  ctdt.c  symTbl.c depend.* *.map *.a *.klp vxram*.* ram*.* *.h *.list
   if "%snmp_type%" == "new" make -f..\..\..\snmp\Envoy\vxworks\makefile clean
   if "%snmp_type%" == "old" make -C..\..\..\snmp\vxworks clean
   cd ..
   goto endboard
:endif

if not "%opt_cs%" == "yes" goto skipsettings
   cd %TARGETDIR%
   if not exist buildsettings.txt goto endif
      rm -f buildsettings.old
      rename buildsettings.txt buildsettings.old
   :endif
   for %%o in (%option_no_list% %option_yes_list% %option_default_list%) do set opt_%%o >>buildsettings.txt
   if not exist buildsettings.old goto endif
      echo n | comp buildsettings.txt buildsettings.old >NUL: 2>NUL:
      if not errorlevel 1 goto endif
         echo.
         echo ***** Build settings have changed.  You may want to clean and rebuild. *****
   :endif
   cd ..
   echo off
:skipsettings

set title_string=Building Application T%TORNADO_VERSION% %board% %country% SNMP=%SNMP_SUPPORT%
if %BCM_REDUCED_IMAGE_SIZE%   == 1   set title_string=%title_string% SLIM
if %BCM_REDUCED_IMAGE_SIZE%   == 0   set title_string=%title_string% FAT
if %opt_propane%              == yes set title_string=%title_string% PROPANE
if %opt_cablehome%            == yes set title_string=%title_string% CABLEHOME
if %opt_standalone%           == yes set title_string=%title_string% STANDALONE
if %EMTA_SUPPORT%             == 1   set title_string=%title_string% EMTA
if %EMTA_SUPPORT%             == 0   set title_string=%title_string% noEMTA
if %opt_ipsec%                == yes set title_string=%title_string% IPSEC
if %HPNA_HAL_INCLUDED%        == 1   set title_string=%title_string% HPNA
if %SNMP_WINDMANAGE%          == 1   set title_string=%title_string% WMSNMP
if %MANAGE_SWITCH_INCLUDED%   == 1   set title_string=%title_string% MANAGESWITCH
if %BCM_DSG_SUPPORT%          == 1   set title_string=%title_string% DSG
if %BCM_DSGCC_SUPPORT%        == 1   set title_string=%title_string% DSGCC
if %BCM_3418_SUPPORT%         == 1   set title_string=%title_string% TUNER3418
if %USE_PRIVATE_NETWORK%      == 1   set title_string=%title_string% PRIVATE_NETWORK

:: Put the username in the title if on a build server.
if "%COMPUTERNAME:SWBLD=%" == "%COMPUTERNAME%" goto else
   TITLE %USERNAME% %title_string%
   goto endif
:else
   TITLE %title_string%
:endif
echo ##############################
:: Here we go...start the build.. 

:: Write to the screen for those building remotely.
echo.
echo %title_string%
echo.

:: This is for batch file debugging purposes.
:: It will display all the environment variable before they're cleared
if %opt_show%  == yes   goto show_build_parameters

:: (1)Just create a depend.bcmXXXX file; otherwise, Win98 compile will
:: have a hard time since a make clean deletes the depends file.
:: (2)Required romInit.s and sysAlib.s to be in the current dir.
:: Add this workaround instead of changing rules.x86-win32 
echo. > %TARGETDIR%/depend.%TARGETDIR%
:: Something strange in make or the make files causes make to try to build
:: "depend." before making defines.h.  This fails because defines.h is
:: required to build "depend.".
echo. > %TARGETDIR%/depend.

cd %TARGETDIR%

:: Cleanup previous build files...
rm -f vxram*.*
rm -f vxworks*.*
rm -f *.log
rm -f *.list
rm -f defines.h

:: Delete version info file to get new build date.
rm -f CmAppVersionInfo.o

cd ..


:: Define USERNAME macro for banner.
echo #define USERNAME "%USERNAME%"> %TARGETDIR%\username.h



:: Make the default settings file in the build directory.
cd %TARGETDIR%

:: If the user didn't specify a customer board spec, make a non-vol defaults file.
if "%opt_board%" == "yes" goto else1
   
   :: Check for any kDefaultValue_Cm80211* values in the vendor's file.  If
   :: present, then print an error and pause.  These values have been renamed
   :: to kDefaultValue_WiFi80211*, and the vendor needs to change their file to
   :: match the new name.
   if "%defaultfile%" == "" goto :endif1
   if not exist ..\..\..\Custom\%defaultfile% goto :endif1
       findstr kDefaultValue_Cm8021 ..\..\..\Custom\%defaultfile%
       if errorlevel 1 goto endif2
           echo.
           echo WARNING:  Vendor NonVolDefaults file contains old 802.11 nonvol defaults
           echo           keywords.  All kDefaultValue_Cm80211* keywords have been renamed
           echo           to kDefaultValue_WiFi80211*.
           echo.
           echo           Please modify your file before building.
           echo.
           echo Filename: %defaultfile%
           echo.
           pause Hit any key to exit build...
           goto end
       :endif2
   :endif1
   
   :: Copy the defaults file to the build directory.
   copy ..\..\CustomerNonVolDefaults.h MergedNonVolDefaults.h.new >NUL:
   attrib -r MergedNonVolDefaults.h.new
   
   :: Surround added text with #if/#endif statements.
   echo #ifndef MERGEDNONVOLDEFAULTS_H>>                 MergedNonVolDefaults.h.new
   echo #define MERGEDNONVOLDEFAULTS_H>>                 MergedNonVolDefaults.h.new
   
   :: Define the SNMP system descriptor string.
   echo #undef  kDefaultValue_SysDescr>>                 MergedNonVolDefaults.h.new
   echo #define kDefaultValue_SysDescr "V2 cablemodem reference design <<HW_REV: V1.0; VENDOR: Broadcom; BOOTR: 1.0.10; SW_REV: %rev_std%; MODEL: BCM9%board%>>" >>MergedNonVolDefaults.h.new

   :: Define the CableHome SNMP system descriptor string.
   echo #undef  kCableHome_DefaultValue_SysDescr>>       MergedNonVolDefaults.h.new
   echo #define kCableHome_DefaultValue_SysDescr "V2 cablemodem reference design <<HW_REV: V1.0; VENDOR: Broadcom; BOOTR: 1.0.10; SW_REV: %rev_std%; MODEL: BCM9%board%>>" >>MergedNonVolDefaults.h.new
   
   :: Define the current software revision.
   echo #undef  kDefaultValue_DocsDevSwCurrentVers>>     MergedNonVolDefaults.h.new
   echo #define kDefaultValue_DocsDevSwCurrentVers "%rev_std%">> MergedNonVolDefaults.h.new

   :: Override HPNA setting if included.   
   if not "%HPNA_HAL_INCLUDED%" == "1" goto endif
      echo #undef  kDefaultValue_HpnaEnabled>>           MergedNonVolDefaults.h.new
      echo #define kDefaultValue_HpnaEnabled true>>      MergedNonVolDefaults.h.new
   :endif
   
   :: if EMTA support is turned on, change the default message log IP stack 
   :: setting from '2' to '3'.  Also, set the proper EmteSysDescr String.
   if not "%EMTA_SUPPORT%" == "1" goto endif
      if "%emta_sw_rev%" == "" set emta_sw_rev=%rev_std%
      if "%emta_hw_rev%" == "" set emta_hw_rev=1.0
      echo #undef  kDefaultValue_EmtaSysDescr>>                   MergedNonVolDefaults.h.new
      echo #define kDefaultValue_EmtaSysDescr "EMTA Reference Design <<HW_REV: V%emta_hw_rev%; VENDOR: Broadcom; BOOTR: 1.0.10; SW_REV: %emta_sw_rev%; MODEL: BCM9%board%>>" >>MergedNonVolDefaults.h.new
      echo #undef  kDefaultValue_RemoteAccessIpStackNumber>>      MergedNonVolDefaults.h.new
      echo #define kDefaultValue_RemoteAccessIpStackNumber 3 >>   MergedNonVolDefaults.h.new
      if "%emta_SysContact%" == "" goto l_1
      echo #undef  kDefaultValue_EmtaSysContact>>                 MergedNonVolDefaults.h.new
      echo #define kDefaultValue_EmtaSysContact "%emta_SysContact%" >>MergedNonVolDefaults.h.new
      :l_1
      if "%emta_SysName%" == "" goto l_2
      echo #undef  kDefaultValue_EmtaSysName>>                    MergedNonVolDefaults.h.new
      echo #define kDefaultValue_EmtaSysName "%emta_SysName%" >>  MergedNonVolDefaults.h.new
      :l_2
      if "%emta_SysLocation%" == "" goto l_3
      echo #undef  kDefaultValue_EmtaSysLocation>>                   MergedNonVolDefaults.h.new
      echo #define kDefaultValue_EmtaSysLocation "%emta_SysLocation%" >> MergedNonVolDefaults.h.new
      :l_3
      if "%emta_SysObjID%" == "" goto l_4
      echo #undef  kDefaultValue_EmtaSysObjectId>>                   MergedNonVolDefaults.h.new
      echo #define kDefaultValue_EmtaSysObjectId "%emta_SysObjID%" >> MergedNonVolDefaults.h.new
      :l_4
   :endif
   
   :: Override 802.11 setting if included.
   if not "%WIFI80211_HAL_INCLUDED%" == "1" goto endif
      echo #undef  kDefaultValue_802_11Enabled>>         MergedNonVolDefaults.h.new
      echo #define kDefaultValue_802_11Enabled true>>    MergedNonVolDefaults.h.new
   :endif
   
   :: See if the user specified a Docsis override.
   if "%opt_docsis%" == "no" goto endif2
   
      :: Build for Docsis 1.0.  Default is Docsis 1.1.
      if not "%opt_docsis_parm%" == "10" goto endif
         echo #undef  kDefaultValue_Docsis11Support>>       MergedNonVolDefaults.h.new
         echo #define kDefaultValue_Docsis11Support false>> MergedNonVolDefaults.h.new
      :endif
      
      :: Build for Docsis 2.0.  Default is Docsis 1.1.
      if not "%opt_docsis_parm%" == "20" goto endif
         echo #undef  kDefaultValue_AdvancedPhySupport>>      MergedNonVolDefaults.h.new
         echo #define kDefaultValue_AdvancedPhySupport true>> MergedNonVolDefaults.h.new
      :endif
      
   :endif2
   
   :: The 3348 needs some different defaults.  These can be set in
   :: non-vol, but it makes it easier to have them set here.
   if not "%board%" == "3348" goto endif
      echo #undef  kDefaultValue_Docsis1xCmtsHackForDocsis20>>       MergedNonVolDefaults.h.new
      echo #define kDefaultValue_Docsis1xCmtsHackForDocsis20 false>> MergedNonVolDefaults.h.new
      echo #undef  kDefaultValue_DsRefFreq>>                         MergedNonVolDefaults.h.new
      echo #define kDefaultValue_DsRefFreq     24000000>>            MergedNonVolDefaults.h.new
      echo #undef  kDefaultValue_UsRefFreq>>                         MergedNonVolDefaults.h.new
      echo #define kDefaultValue_UsRefFreq     24000000>>            MergedNonVolDefaults.h.new
   :endif

   if not "%BCM_3418_SUPPORT%" == "1" goto endif
	  echo #undef  kDefaultValue_DpmDefaultsFile>>                   MergedNonVolDefaults.h.new
	  echo #define kDefaultValue_DpmDefaultsFile "DefaultBcm3418DocsisCmDownstreamCalibrationTables.h">>                         MergedNonVolDefaults.h.new
	  echo #define kDefaultTuner_BCM3418>>                   MergedNonVolDefaults.h.new 
   :endif
   
   :: Surround added text with #if/#endif statements.
   echo #endif>>  MergedNonVolDefaults.h.new
   
   goto endif1
   echo 
:: The user did specify a customer board spec, so use its settings file
:else1
   :: The non-vol defaults file is named after the "board" parameter.
   set defaultfile=%opt_board_parm%NonVolDefaults.h
   if not exist ..\..\..\Custom\%defaultfile% set defaultfile=%opt_board_parm%_NonVolDefaults.h
   
   :: Make sure the file exists
   if exist ..\..\..\Custom\%defaultfile% goto endif
      echo Error: Non-vol defaults file not found: Custom/%defaultfile%
      cd ..
      goto end_error
   :endif

   copy ..\..\CustomerNonVolDefaults.h MergedNonVolDefaults.h.new
   attrib -r MergedNonVolDefaults.h.new
   sed -f ../undefines.sed -e "s/{rev_std}/%rev_std%/" ../../../Custom/%defaultfile% >> MergedNonVolDefaults.h.new

   :: Surround added text with #if/#endif statements.
   echo #ifndef MERGEDNONVOLDEFAULTS_H>>                 MergedNonVolDefaults.h.new
   echo #define MERGEDNONVOLDEFAULTS_H>>                 MergedNonVolDefaults.h.new

   :: Define the SNMP system descriptor string.
   echo #undef  kDefaultValue_SysDescr>>                 MergedNonVolDefaults.h.new
   echo #define kDefaultValue_SysDescr "V2 cablemodem reference design <<HW_REV: V1.0; VENDOR: Broadcom; BOOTR: 1.0.10; SW_REV: %rev_std%; MODEL: BCM9%board%>>" >>MergedNonVolDefaults.h.new
	  
   :: Define the current software revision.
   echo #undef  kDefaultValue_DocsDevSwCurrentVers>>     MergedNonVolDefaults.h.new
   echo #define kDefaultValue_DocsDevSwCurrentVers "%rev_std%">> MergedNonVolDefaults.h.new

   :: If the user wants to override the Docsis default setting...   
   if "%opt_docsis%" == "no" goto endif2
   
      :: Build for Docsis 1.0.  Override board default.
      if not "%opt_docsis_parm%" == "10" goto endif
         echo #undef  kDefaultValue_Docsis11Support>>          MergedNonVolDefaults.h.new
         echo #define kDefaultValue_Docsis11Support false>>    MergedNonVolDefaults.h.new
         echo #undef  kDefaultValue_AdvancedPhySupport>>       MergedNonVolDefaults.h.new
         echo #define kDefaultValue_AdvancedPhySupport false>> MergedNonVolDefaults.h.new
      :endif
      
      :: Build for Docsis 1.1.  Override board default.
      if not "%opt_docsis_parm%" == "11" goto endif
         echo #undef  kDefaultValue_Docsis11Support>>          MergedNonVolDefaults.h.new
         echo #define kDefaultValue_Docsis11Support true>>     MergedNonVolDefaults.h.new
         echo #undef  kDefaultValue_AdvancedPhySupport>>       MergedNonVolDefaults.h.new
         echo #define kDefaultValue_AdvancedPhySupport false>> MergedNonVolDefaults.h.new
      :endif
      
      :: Build for Docsis 2.0.  Override board default.
      if not "%opt_docsis_parm%" == "20" goto endif
         echo #undef  kDefaultValue_Docsis11Support>>         MergedNonVolDefaults.h.new
         echo #define kDefaultValue_Docsis11Support true>>    MergedNonVolDefaults.h.new
         echo #undef  kDefaultValue_AdvancedPhySupport>>      MergedNonVolDefaults.h.new
         echo #define kDefaultValue_AdvancedPhySupport true>> MergedNonVolDefaults.h.new
      :endif
	  
   :endif2

   :: Surround added text with #if/#endif statements.
   echo #endif>>  MergedNonVolDefaults.h.new
   
:endif1

:: Now compare the new settings file with the previous one, if it exists.
if not exist MergedNonVolDefaults.h goto else
   :: Compare the files.
   echo n | comp MergedNonVolDefaults.h MergedNonVolDefaults.h.new >NUL: 2>NUL:
   :: If they are different, replace the old one.
   if not errorlevel 1 goto endif
      del/f MergedNonVolDefaults.h
      rename MergedNonVolDefaults.h.new MergedNonVolDefaults.h
   goto endif
:: There was no old file, so just rename the new one.
:else
   rename MergedNonVolDefaults.h.new MergedNonVolDefaults.h
:endif
   
cd ..






:: If we are building vxWorks.st, remove VxWorksConsoleCommands.o because
:: this is where the #define for shellInit() is done and we may be switching 
:: between vxWorks and vxWorks.st but not touching VxWorksConsoleCommands.cpp
if "%TARGET%" == "vxWorks.st" rm -f %TARGETDIR%/VxWorksConsoleCommands.o

:: If we're just checking the environment, skip make.
if "%opt_check%" == "yes" goto endboard

if "%opt_mac14%" == "no"  goto endif
   rm -f %TARGETDIR%\mac14_image_name.h
   :: Configure the environment with the appropriate MAC14 string.
   set MAC_14_IMAGE=1
   echo #define MAC_14_IMAGE_NAME "MAC-14 test image for CW%opt_mac14_parm%" > %TARGETDIR%\mac14_image_name.h
   :: Make the MAC14 app, first removing the object files
   :: which contain MAC14 strings or string handling code.
   rm -f %TARGETDIR%\docsDevBridge.o
   rm -f %TARGETDIR%\systemBridge.o
   rm -f %TARGETDIR%\CmSnmpAgent.o
:endif

if "%opt_bootl%" == "no"  goto endif
   :: These files must be re-made to put the bootloader in the image.
   rm -f %TARGETDIR%\bootloaderstore.o
   rm -f %TARGETDIR%\root.o
   rm -f %TARGETDIR%\cxx.opt
:endif

if "%opt_ps%" == "yes" goto skipmake
:: If we're not making the app, don't make SNMP
if not %TARGET% == vxWorks if not %TARGET% == vxWorks.st if not %TARGET% == oldlink goto skip_snmp
if %SNMP_SUPPORT% == 0 goto skip_snmp 
   set snmp_target=all
   if %BCM_VENDOR_SUPPORT% == 1 set snmp_target=all
   if "%snmp_type%" == "new" make -C%TARGETDIR% -f..\..\..\snmp\Envoy\vxworks\makefile %snmp_target%
   if "%snmp_type%" == "old" make -C..\..\snmp\vxworks %snmp_target%
   if errorlevel 1 goto BuildFailed
:skip_snmp

rm -f %TARGETDIR%/%TARGET%
:: Make sure there's an up-to-date defines.h.
echo :: Make sure there's an up-to-date defines.h.
if not %TARGET% == defines.h make --makefile=../Makefile --directory=%TARGETDIR% --no-print-directory defines.h

make --makefile=../Makefile --directory=%TARGETDIR% -w %TARGET%
if errorlevel 1 goto BuildFailed
:skipmake
if %TARGET% == vxWorks    goto postprocess
if %TARGET% == vxWorks.st goto postprocess
if %TARGET% == oldlink    set TARGET=vxWorks& goto postprocess
goto cleanup

:postprocess

cd %TARGETDIR%
if not exist %TARGET% cd ..& goto BuildFailed

:: if this is a settop build, skip normal postprocess
if "%board%" == "3730" goto settop_postprocess
if "%board%" == "7110" goto settop_postprocess
if "%board%" == "7111" goto settop_postprocess
if "%board%" == "7038" goto settop_postprocess

if not exist vxworks.map goto skipdemangle
   rm -f vxram.map
   rename vxworks.map vxram.map
   :: Sort the map file if we have the tools to do so.  First try the mingw
   :: tools if they exist, because they are significantly faster.
::   tcsh ../sortmap.csh vxram.map 2>NUL:
::   if not errorlevel 1 goto endif
      :: No mingw tools, so try MKS tools.
      sh ../sortmap.ksh >NUL: 2>NUL:
::   :endif
   :: If the sorted map exists, demangle the names.
   if exist vxram.map.s  c++filtmips <vxram.map.s >vxram.map.d& goto endif
      :: Otherwise, demangle the unsorted names.
      c++filtmips <vxram.map   >vxram.map.d
   :endif
   :: If there were no errors, delete the original map file.
   if not errorlevel 1 rm -f vxram.map
   rm -f vxram.map.s
:skipdemangle

:: If there's a map file from a partial link (when making vxWorks.st), then
:: demangle the names.  If that worked OK, delete the original partial.map.
if not exist partial.map goto skipdmpartial
   c++filtmips <partial.map   >partial.map.d
   if not errorlevel 1 rm -f partial.map
:skipdmpartial

:: Convert .elf output into .bin format.
elfToBin < %TARGET% > vxram.bin

:: The programstore signature has become a product ID.  By default we use the
:: chip ID, but the user may override this value with the "pid <prodID>" option.
set productID=%PROGRAM_STORE_SIGNATURE%
if "%opt_pid%" == "yes" set productID=0x%opt_pid_parm%

call :echocall programstore -c %ps_compression% -f vxram.bin -o vxram_sto.bin -v %sto% -a 0x80010000 -s %productID%

:: If the LZMA algorithm is specified (c4), some bootloaders won't be able to
:: decompress the binary.  Make a special binary with a piggyback decompressor
:: to provide backward compatibility.
if not "%ps_compression%" == "4" goto endif
if errorlevel 1 goto endif
if not exist ..\LZMAdecomp.bin goto endif
   echo.
   echo ==== Making piggyback binary...
   :: Get the uncompressed size of the application (vxram.bin).
   for /f "skip=6 tokens=3" %%i in ('dir vxram.bin') do (set vxram_size=%%i& goto endfor)
   :endfor
   set vxram_size=%vxram_size:,=%
   :: Set the piggyback load address based on the uncompressed size.  We don't
   :: know if we'll run on a modem with 8M, 16M, or 32M of RAM, so we can't use
   :: a fixed load address.
   set startAddress=0x80500000
   if %vxram_size% GEQ 0x4f0000 set startAddress=0x80600000
   if %vxram_size% GEQ 0x5f0000 set startAddress=0x80700000
   if %vxram_size% GEQ 0x6f0000 set startAddress=0x80800000
   if %vxram_size% GEQ 0x7f0000 set startAddress=0x80900000
   if exist vxram_sto.pb del /f vxram_sto.pb
   :: Now append the decompressor and the compressed image and put another
   :: ProgramStore header onto the package.
   copy /b ..\LZMAdecomp.bin+vxram_sto.bin vxram_sto.pb >NUL:
   call :echocall programstore -c 0 -f vxram_sto.pb -o vxram_pb.bin -v %sto% -a %startAddress% -s %productID%
   del /f vxram_sto.pb
:endif
cd ..

if not exist %TARGETDIR%\vxram_sto.bin goto BuildFailed

:buildcomplete
:: If cache optimization is required, sort the map file and run the utility.
echo :: If cache optimization is required, sort the map file and run the utility.
if "%opt_cacheopt%" == "no" goto endif
   cd %TARGETDIR%
   set mapfile=vxram.map.d
   if "%TARGET%" == "vxWorks.st" set mapfile=partial.map.d
   set slim=
   if "%opt_slim%" == "yes" set slim=slim
   sort < %mapfile% > %board%%slim%_sort.map
   :: The cache optimization now runs in 2 phases; first, we do the downstream
   :: path.  Then we feed this into the upstream.  We feed the combined result
   :: into the link phase again.
   ::
   :: Call the helper script to manage the multi-pass optimization.
   call ..\OptimizeCacheDsUs.bat %board%%slim%_sort.map %board%%slim%.cachefiles.inc
   cd ..
   :: Call makeapp again without the "cacheopt" option.
   call makeapp %full_parms:cacheopt=%
:endif

echo #########################################
echo     %buildtarget% %country% completed successfully!
echo #########################################

dir %TARGETDIR%\vxram*

goto cleanup

:BuildFailed
echo :BuildFailed
   if "%opt_color%" == "yes" color 47
   echo #########################################
   echo     %buildtarget% build failed!
   echo #########################################
   set m_error=error
   goto cleanup


:settop_postprocess
   dir %TARGET%
   copy %TARGET% d:\public\vxWorks_%board%
   cd ..
   goto buildcomplete


:cleanup

if "%opt_bootl%" == "no" goto endif
   :: These files were re-made to put the bootloader in the image.
   rm -f %TARGETDIR%\bootloaderstore.o
   rm -f %TARGETDIR%\vxroot.o
   rm -f %TARGETDIR%\cxx.opt
:endif

:: If we built for MAC14, remove the evidence.
if "%opt_mac14%" == "no" goto endif
   rm -f %TARGETDIR%\mac14_image_name.h
   rm -f %TARGETDIR%\docsDevBridge.o
   rm -f %TARGETDIR%\systemBridge.o
   rm -f %TARGETDIR%\CmSnmpAgent.o
   set MAC_14_IMAGE=
:endif

:: Loop again to see if any other board targets need to be made.
:endboard

:: Get and print the elapsed time for this build job.
call ..\..\cm_bsp_v2\bsp_common\os\timer_end

if not "%m_error%" == "error" goto major_loop

:end_error
echo :end_error
   if .%OS% == .Windows_NT endlocal
   :: The "set errorlevel" command will set the error status to 1 (failure).
   :: This is an easy way to let a make job know the command failed.
   set errorlevel=
   set errorlevel 2>NUL:
   goto :eof


:echocall
   echo ----%*
   call %*
   goto :eof

:show_build_parameters
   set

:end

if .%OS% == .Windows_NT endlocal


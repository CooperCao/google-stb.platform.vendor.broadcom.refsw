#****************************************************************************
#
# Copyright (c) 2007-2009 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
# AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
# SOFTWARE.  
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#****************************************************************************
#  $Id$
#
#  Filename:       functions.bash
#  Author:         Mike Sieweke
#  Creation Date:  Oct 20, 2003
#
#****************************************************************************

#----------------------------------------------------------------------------
# Helper functions start here...  This file will be included by other build
# scripts to define common functions.
#----------------------------------------------------------------------------

# Don't export functions.  When exported to a thread's DOS environment, these
# functions look like very long environment variables.  They exceed the size
# limit for a DOS environment, which makes creating a sub-thread fail with a
# "Resource temporarily unavailable" error.

# *** IMPORTANT ***  Don't define any environment variables between this line
# and the "set -o allexport" at the end of this file.
set +o allexport

#----------------------------------------------------------------------------

# These are dummy functions that can be overridden by the system-specific
# extension script.  These functions are called at various points; the
# default implementations don't do anything interesting.

function SetValidParametersExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "SetValidParametersExt : default implementation does nothing."
    doNothing=""
}

function ProcessBuildProcessorExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "ProcessBuildProcessorExt : default implementation does nothing."
    doNothing=""
}

function ProcessYesNoOptionsExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "ProcessYesNoOptionsExt : default implementation does nothing."
    doNothing=""
}

function ProcessParamOptionsExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "ProcessParamOptionsExt : default implementation does nothing."
    doNothing=""
}

function ConfigureOutputDirSuffixExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "ConfigureOutputDirSuffixExt : default implementation does nothing."
    doNothing=""
}

function CreateMergedNonVolDefaultsExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "CreateMergedNonVolDefaultsExt : default implementation does nothing."
    doNothing=""
}

function PreBuildCalloutExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "PreBuildCalloutExt : default implementation does nothing."
    doNothing=""
}

function PostBuildCalloutExt {
    # If your external script isn't being called, enable this to aid in
    # debugging.
    #echo "PostBuildCalloutExt : default implementation does nothing."
    doNothing=""
}

# Check to see if the extension script exists; if so, include it, allowing
# its functions to override the default implementations.
if [ -e makeapp_ext.bash ] ; then
    . makeapp_ext.bash
fi

#----------------------------------------------------------------------------

function FormatColumns {
    columns=$1
    shift
    echo "$*" | sed -r -e 's/ +/\n/g' | sort | pr --columns=$columns --omit-pagination --width=100 | column -t | sed -e 's/^/    /'
}

#----------------------------------------------------------------------------

function PrintUsage {

    echo
    echo "Usage:"
    echo "    makeapp [products] [processors] [yes/no options] [value options] [targets]"
    echo
    echo "Build parameters can be specified in any order, and can even be specified more"
    echo "than once, in which case the last one will take effect."
    echo
    echo "[products] is a list of 0 or more product names.  A product is defined by the"
    echo "  presence of a separate {product}_settings.bash setup script in the Custom"
    echo "  directory.  This script specifies a list of build options that should be"
    echo "  used as if they were specified on the command line.  Note that options which"
    echo "  are specified on the command line will be evaluated after this script, so"
    echo "  they can override the values set by the script.  If you specify the same"
    echo "  product name more than once, it will be built more than once; we don't filter"
    echo "  out duplicates."
    echo
    validProducts=`echo Custom/* | sed -e "s,Custom/,,g" -e "s/_settings.bash//g"`
    echo "  Valid products:"
    FormatColumns 3 $validProducts
    echo
    echo "[processors] is a list of 0 or more processor numbers (i.e. 3348).  These are"
    echo "  primarily used for reference design builds, where the product definition"
    echo "  script doesn't exist.  If you specify the same processor more than once, it"
    echo "  will be built more than once; we don't filter out duplicates."
    echo
    echo "  Valid processors:"
    FormatColumns 12 $validProcessors
    echo
    echo "[yes/no options] is a list of 0 or more options used to configure the build."
    echo "  These options can be specified to enable the feature, or can be specified"
    echo "  with a 'no' to disable (e.g. 'slim' or 'noslim').  All options have a default"
    echo "  value that will be used if not specified on the command line; the default"
    echo "  might change based on other options.  If you specify the same option more"
    echo "  than once, the last one will be used."
    echo
    echo "  Valid yes/no options:"
    FormatColumns 4 $validYesnoOptions
    echo
    echo "[value options] is a list of 0 or more options used to configure the build."
    echo "  These options take a value, which must immediately follow the option name."
    echo "  All options have a default value that will be used if not specified on the"
    echo "  command line; the default might change based on other options.  If you specify"
    echo "  the same option more than once, the last one will be used."
    echo
    echo "  Valid value options:"
    FormatColumns 6 $validParamOptions
    echo
    echo "[targets] is a list of 0 or more targets for the build; the default is linux."
    echo "  This will be anything that was specified as a command line parameter that"
    echo "  wasn't otherwise recognized as a product, processor or option.  You can"
    echo "  specify anything that the make tool will understand, like -d to enable make"
    echo "  debugging, or clean to do a clean build.  The two main targets of interest are"
    echo "  linux and linux.st (with symbol table).  In both of these cases, the build"
    echo "  will also produce vxram.bin and vxram_sto.bin for the final executable image."
    echo "  You can also specify one or more .o files as the build target.  The targets"
    echo "  will be passed to make in the order they were found in the product setup"
    echo "  script and on the command line.  Thus, if you want to build clean, you should"
    echo "  make sure 'clean' appears first so that you don't build the image, then clean"
    echo "  it..."
    echo
    echo "You must specify at least one [product] or [processor] in order for a build to"
    echo "occur.  You can specify multiple of both if desired."
    echo
    echo "When building, the output directory will be set to the current product or"
    echo "processor name, combined with certain build options.  For example, a 3348"
    echo "build will be placed in bcm93348, and a slim 3348 build will be in"
    echo "bcm93348_slim."
    echo
    echo "Examples:"
    echo "    makeapp 3348 clean                 : Makes the bcm93348 output dir clean"
    echo "    makeapp 3348 clean linux            : Builds clean, then the full image"
    echo "    makeapp clean slim linux 3348       : Builds clean, then a slim image"
    echo "    makeapp bcm93348rg                 : Builds using options in bcm93348rg_settings.bash"
    echo

}

#----------------------------------------------------------------------------

function SetValidParameters {

    # These are the processor types that we know how to build.  Many of them
    # have the same core CPU and can be handled similarly.
    validProcessors="3345 3348 3360 3255 3368 5365 5352 1112 1115 6348 7118"

    # These are the yes/no options that we know how to parse and process.
    validYesnoOptions="slim superslim cacheopt quiet show dualbuild serialportoff \
        bcm80211g bcm80211g_debug wifimfg homeplug parental battery power telnet map popup managedswitch \
        dualflash intelflash amdflash cfiflash jedecflash deps bonded dasm fpm \
        fpm_descr_only ipc openssl098 openssh usb20 vendorhttps tftp_server \
        httpssl piggyback spiflash sip ipv6 giga_enet internalusb sipdqos sigtls \
        wifihotspot thermalmonitor smp ejtag fn_profile demangle vpn factorymibs \
        mibnames mgmtmibs sipdbg 3380fpga 3380fpga_dsmac l2tp pppoe pptp"

    # These are the options that take a single parameter that we know how to
    # parse and process.
    validParamOptions="sto c pid board imagename"


    # These are the default values for the yes/no options if they are not
    # specified.
    defaultYesnoOptions="nosuperslim slim nocacheopt quiet noshow dualbuild noserialportoff \
        nobcm80211g nobcm80211g_debug nowifimfg nohomeplug noparental nobattery nopower notelnet nomap nopopup nomanagedswitch \
        nodualflash intelflash amdflash cfiflash nojedecflash deps nobonded nodasm nofpm \
        nofpm_descr_only noipc openssl098 noopenssh nousb20 novendorhttps notftp_server \
        nohttpssl nopiggyback nospiflash nosip noipv6 nogiga_enet nointernalusb nosipdqos nosigtls \
        nowifihotspot nothermalmonitor nosmp noejtag nofn_profile demangle vpn factorymibs \
        mgmtmibs nosipdbg no3380fpga no3380fpga_dsmac nol2tp nopppoe nopptp"

    # These are the default values for the parameter options if they are not
    # specified.  Don't specify the default PID parameter here; its default is
    # based on the current processor.
    defaultParamOptions="sto 003.000 c 4 imagename ecram"


    # This handles old-style compression specifier.  We prefer that the compression
    # type be specified as "c #", but the old script allowed "c#".  We need to
    # translate the old way to the new way.  Each of these will be pre-pended with
    # a "c" for comparison purposes.
    validOldComprOptions="0 1 2 3 4 6 9"


    # Call extension function to set up app-specific parameters that will be
    # allowed.
    SetValidParametersExt

    if [ "$debug" == "1" ] ; then
        # Display the parameters that are valid for this build.
        echo
        echo "    validProcessors = ${validProcessors}"
        echo "  validYesnoOptions = ${validYesnoOptions}"
        echo "  validParamOptions = ${validParamOptions}"
        echo "defaultYesnoOptions = ${defaultYesnoOptions}"
        echo "defaultParamOptions = ${defaultParamOptions}"
        echo
    fi

}


#----------------------------------------------------------------------------

function ProcessBuildProcessor {

    # Processor-specific setup.

    # Most of our "current" processors use a 24MHz crystal.
    XTALFREQ=24000000

    # Selectively override this on an as-needed basis.
    #
    # The 3360 uses a 28MHz crystal.
    #
    # DPullen - building for the 3360 not support for linux.
    #if "%1" == "3360" set XTALFREQ=28000000

    # UNFINISHED - others?

    # Call extension script (if it exists) to process its own options.
    ProcessBuildProcessorExt $1
}

#----------------------------------------------------------------------------

function ProcessYesNoOptions {

    for parm in $* ; do
        case $parm in
            superslim)
                opt_slim=2
                continue
                ;;
            nosuperslim)
                opt_slim=0
                continue
                ;;
        esac

        # Note that the 'mibnames' parameter is not in the defaultYesNoOptions
        # list.  This means that if it's not specified, it gets no default.
        # So here we can flag if this is specified or not.  If it's not 
        # specified, then later we will assign it based on slim or superslim.
        # If it is specified, then it gets that value.
        case $parm in
            mibnames)
                mibnames_specified=1
                ;;
            nomibnames)
                mibnames_specified=1
                ;;
        esac
        
        for p in $validYesnoOptions ; do

            if [[ "$parm" == "$p" ]] ; then
                # debug: print the parm
                #echo $parm
                eval opt_${p}=1
                continue 2
            fi

            if [[ "$parm" == "no$p" ]] ; then
                eval opt_${p}=0
                continue 2
            fi

        done

    done


    # Slim/noslim build.  This sets an environment variable that the
    # makefile uses to decide whether or not to run MessageLogZapper.
    # When the user types "superslim" then opt_slim == 2.
    if [[ $opt_slim == 2 ]] ; then
        BFC_CONFIG_SUPERSLIM=1
        tmp_slim=1
    else
        BFC_CONFIG_SUPERSLIM=0
        tmp_slim=$opt_slim
    fi
    BCM_REDUCED_IMAGE_SIZE=${tmp_slim}
    BFC_CONFIG_MINIMAL_CONSOLE=${tmp_slim}

    BFC_INCLUDE_BATTERY_SUPPORT=${opt_battery}

    BFC_INCLUDE_THERMAL_MONITOR_SUPPORT=${opt_thermalmonitor}

    BFC_INCLUDE_OPENSSL098=${opt_openssl098}

    BFC_INCLUDE_OPENSSH_SUPPORT=${opt_openssh}

    BCM_VPN_SUPPORT=${opt_vpn}
    
    BCM_L2TP_SUPPORT=${opt_l2tp}

    BCM_PPTP_SUPPORT=${opt_pptp}

    BCM_PPPOE_SUPPORT=${opt_pppoe}
    
    POWER_MGMT=${opt_power}

    BFC_MAKE_MAP_FILE=${opt_map}

    # 802.11 Hal support
    if [[ "$opt_bcm80211g" == "1" ]] ; then
          BFC_INCLUDE_WIFI80211_SUPPORT=1
	if [[ "$opt_wifimfg" == "1" ]] ; then
	     BFC_INCLUDE_WIFI80211MFG_SUPPORT=1
	else
          BFC_INCLUDE_WIFI80211MFG_SUPPORT=0
	fi
    else
	if [[ "$opt_bcm80211g_debug" == "1" ]] ; then
               BFC_INCLUDE_WIFI80211_SUPPORT=1
          else
               BFC_INCLUDE_WIFI80211_SUPPORT=0
         fi
    fi

    if [[ "$buildProcessor" == "3368" ]] ; then
          BFC_INCLUDE_WIFI80211G_BEARS_SUPPORT=${opt_bcm80211g}
          BFC_INCLUDE_WIFI80211G_MBSS_SUPPORT=0
	  if [[ "$opt_bcm80211g_debug" == "1" ]] ; then
               BFC_INCLUDE_WIFI80211G_BEARS_SUPPORT=${opt_bcm80211g_debug}
               BFC_INCLUDE_WIFI80211G_BEARS_DEBUG_SUPPORT=${opt_bcm80211g_debug}
	  fi
    else
    if [[ "$buildProcessor" == "3255" ]] ; then
          BFC_INCLUDE_WIFI80211G_BEARS_SUPPORT=${opt_bcm80211g}
          BFC_INCLUDE_WIFI80211G_MBSS_SUPPORT=0
	  if [[ "$opt_bcm80211g_debug" == "1" ]] ; then
               BFC_INCLUDE_WIFI80211G_BEARS_SUPPORT=${opt_bcm80211g_debug}
               BFC_INCLUDE_WIFI80211G_BEARS_DEBUG_SUPPORT=${opt_bcm80211g_debug}
	  fi
    else
       BFC_INCLUDE_WIFI80211G_BEARS_SUPPORT=0
       BFC_INCLUDE_WIFI80211G_MBSS_SUPPORT=0
    fi
    fi

    # HomePlug Hal support
    BFC_INCLUDE_HOMEPLUG_SUPPORT=${opt_homeplug}

    # Bonded channel support
    BFC_BONDED_CHANNEL_SUPPORT=${opt_bonded}

    # The "echo" setting has the opposite sense of the "quiet" option.
    BCM_ECHO_MAKE_COMMANDS=$(( ! opt_quiet ))

    BCM_OPTIMIZE_CACHE=$opt_cacheopt
    if [[ "$opt_cacheopt" == "1" ]] ; then
        LINKER_SCRIPT=target.cacheopt.ld
    else
        #LINKER_SCRIPT=../../../Bfc/linux/target.ld
        LINKER_SCRIPT=
    fi

    # If set to 1, the serial port will be disabled at startup.
    BFC_CONFIG_SERIAL_PORT_DISABLED=${opt_serialportoff}

    # If set to 1, online lookup service support for parental controls will be enabled.
    BFC_PARENTAL_CONTROL_ENABLED=${opt_parental}

    # If set to 1, the telnet server will be included with console support.
    BFC_INCLUDE_TELNET_CONSOLE_SUPPORT=${opt_telnet}

    # If set to 1, pop-up blocking support will be included.
    BFC_POPUPBLOCKER_ENABLED=${opt_popup}

    # If set to 1, managed Ethernet switch support will be included.
    BCM_MANAGED_SWITCH_INCLUDED=${opt_managedswitch}

    # If set to 1, USB20 support will be included.
    BFC_USB20_INCLUDED=${opt_usb20}

    BFC_INTERNAL_USB20=${opt_internalusb}

    # If set to 1, inter-process support will be included.
    BCM_IPC_INCLUDED=${opt_ipc}

    # If set to 1, use vendor HTTP server instead of ours
    BCM_VENDOR_HTTP_SERVER=${opt_vendorhttps}

    # PR7327 - added various 'flash' build options.
    BFC_INCLUDE_DUAL_FLASH_SUPPORT=${opt_dualflash}
    BFC_INCLUDE_INTEL_FLASH_SUPPORT=${opt_intelflash}
    BFC_INCLUDE_AMD_FLASH_SUPPORT=${opt_amdflash}
    BFC_INCLUDE_CFI_FLASH_SUPPORT=${opt_cfiflash}
    BFC_INCLUDE_JEDEC_FLASH_SUPPORT=${opt_jedecflash}
    BFC_INCLUDE_SPI_FLASH_SUPPORT=${opt_spiflash}

    # If set to 1, generate dependency source files
    BFC_MAKE_DEPENDENCY_FILES=${opt_deps}

    # If set to 1, disassemble linux.elf to produce ecram.dasm
    BFC_DISASSEMBLE_OUTPUT=${opt_dasm}

    # If set to 1, Hardware Free Pool Manager support will be included.
    BCM_HARDWARE_FREE_POOL_MANAGER_INCLUDED=${opt_fpm}

    # If set to 1, Hardware Free Pool Manager SRAM Descriptor Only Mode support will be included.
    # This option requires "fpm" option above as well
    BCM_HARDWARE_FPM_SRAM_DESCRIPTORS_ONLY_INCLUDED=${opt_fpm_descr_only}

    # Optional TFTP server support.
    BFC_INCLUDE_TFTP_SERVER_SUPPORT=${opt_tftp_server}

    # Optional SSL support for HTTP server
    BRCM_HTTP_SSL_SUPPORT=${opt_httpssl}

    # Optional WiFi Hotspot support
    BCM_WIFI_HOTSPOT_SUPPORT=${opt_wifihotspot}

    # Set supported SIP option for EMTA.  Default is no support for SIP (and therefore support
    # for NCS is assumed since they are mutually exclusive for now).
    BFC_INCLUDE_SIP_SUPPORT=${opt_sip}

    # Set supported SIP-DQoS option for EMTA.  Default is no support for SIP-DQoS.
    BFC_INCLUDE_SIPDQOS_SUPPORT=${opt_sipdqos}

    # Option to build an image with a piggy-back decompressor.  For backward
    # compatibility with very old bootloaders.
    BFC_PIGGYBACK=${opt_piggyback}

    # Optional IPv6 support.
    BFC_INCLUDE_IPV6_SUPPORT=${opt_ipv6}

    # Process Giga-bit Ethernet yes/no options.
    BFC_INCLUDE_GIGA_BIT_ENET_SUPPORT=$opt_giga_enet

    # Set supported TLS option for EMTA signaling.  Default is no support for TLS signaling.
    BFC_INCLUDE_EMTA_SIGTLS_SUPPORT=${opt_sigtls}

    # Set support for SMP (dual-thread) operation.  This links in an linux kernel
    # with support for both TP's.  This option is not compatible with using the
    # second TP for DSP code.
    BFC_INCLUDE_SMP_SUPPORT=${opt_smp}

    # PR12211 - set support for EJTAG DSU.
    BFC_INCLUDE_EJTAG_SUPPORT=${opt_ejtag}

    # PR12289 - set support for function profiling.
    BFC_INCLUDE_FUNCTION_PROFILE_SUPPORT=${opt_fn_profile}
    BFC_DEMANGLE_MAP_FILE=${opt_demangle}

    # Set supported SIP debug option for EMTA.  Default is no support for SIP debug.
    #    Note: SIP debug allows tracing of the SIP stack and call control flow.
    BFC_INCLUDE_EMTA_SIPDBG_SUPPORT=${opt_sipdbg}

    # PR 12883 - set support for factory and runtime MIB support.
    BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT=${opt_factorymibs}
    BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT=${opt_mgmtmibs}
    
    # PR 13391 - set support for MIB names.  If not specified on the 
    # command line we will support MIB names only for a 'fat' build.  If
    # it is specified then the command line takes priority.
    if [ "$mibnames_specified" == "1" ] ; then
        BFC_SUPPORT_SNMP_OBJECT_NAMES=$opt_mibnames
    else
            if [ $tmp_slim == 1 ] ; then
                BFC_SUPPORT_SNMP_OBJECT_NAMES=0
            else
                BFC_SUPPORT_SNMP_OBJECT_NAMES=1
            fi
    fi
    
    BFC_INCLUDE_3380FPGA_SUPPORT=$opt_3380fpga
	
    BFC_INCLUDE_3380FPGA_DSMAC_SUPPORT=$opt_3380fpga_dsmac

    # UNFINISHED - process the rest of the options

    # show
    # etc.

    # Call the extension script (if it exists) to process its own options.
    ProcessYesNoOptionsExt
}

#----------------------------------------------------------------------------

function ProcessParamOptions {

    while [ $# != 0 ] ; do
        parm=$1
        value=$2
        # Use separate shift statements instead of "shift 2", which quietly
        # does nothing if there's only one parameter.
        shift
        shift

        # ProgramStore version number.
        #
        # UNFINISHED - validate the version number for correct formatting?
        if [[ "$parm" == "sto" ]] ; then
            PROGRAM_STORE_VERSION=$value
            continue
        fi

        # ProgramStore compression type.
        #
        # UNFINISHED - validate the compression number that is specified.
        if [[ "$parm" == "c" ]] ; then
            PROGRAM_STORE_COMPRESSION=$value
            continue
        fi

        # ProgramStore PID.
        #
        # UNFINISHED - validate the PID for correct formatting?
        if [[ "$parm" == "pid" ]] ; then
            PROGRAM_STORE_PID=$value
            continue
        fi

        # Set the image name prefix.  The final output files will use this in
        # their names - ecram.bin, ecram_sto.bin, ecram.map.d, etc.
        #
        if [[ "$parm" == "imagename" ]] ; then
            BFC_IMAGE_NAME=$value
            continue
        fi

        # Warn the user that the 'board' parameter is deprecated.
        #
        if [[ "$parm" == "board" ]] ; then
            echo "WARNING - 'board' parameter is deprecated and has no effect."
            continue
        fi

        # UNFINISHED - process the rest of the options that I know about.

        # Call the extension script (if it exists) to process its own options.
        ProcessParamOptionsExt $parm $value

    done

}

#----------------------------------------------------------------------------

function ProcessBuildTargets {

    # We want to check for and strip out the clean target so that it is
    # handled separately.
    buildClean=0
    buildCleanOnly=0
    nonCleanTargets=""

    for parm in $* ; do

        if [[ "$parm" == "clean" ]] ; then
            buildClean=1
        else
            nonCleanTargets="${nonCleanTargets} $parm"
        fi

    done

    # Update the build targets so that clean is stripped out.  If clean was the
    # only target, take note of it.
    buildTargets="${nonCleanTargets}"
    if [[ "$buildTargets" == "" ]] ; then
        buildCleanOnly=1
    fi
}

#----------------------------------------------------------------------------

function ParseParameters {

    # Set these to empty lists initially.
    products=""
    processors=""
    yesnoOptions=""
    paramOptions=""
    buildTargets=""

    # Iterate through the list of command line parameters, trying to separate the
    # various types of parameters out.

    while [ $# != 0 ] ; do
        parm=$1
        shift

        # See if this is a product name (with corresponding configuration script).
        # I have to check two possible directories for this.
        if [[ -e ${SystemCustomDir}/${parm}_settings.bash || \
              -e ${BfcOsCustomDir}/${parm}_settings.bash ]] ; then

            # Add this to the list of product names that will be built.
            RemoveExtraSpaces $products $parm
            products=$removeExtraSpacesResult

            continue

        fi

        # See if this is a macro name (with corresponding macro script).  I have
        # to check two possible directories for this.
        if [[ -e ${SystemCustomDir}/${parm}_macro.bash || \
              -e ${BfcOsCustomDir}/${parm}_macro.bash ]] ; then

            # Select the directory where the macro script is located.
            macroScript="${BfcOsCustomDir}/${parm}_macro.bash"
            if [[ -e ${SystemCustomDir}/${parm}_macro.bash ]] ; then
                macroScript="${SystemCustomDir}/${parm}_macro.bash"
            fi

            echo "Loading additional build options from macro ${macroScript}..."

            # Need to do a little recursion here.  Store the current parameter
            # lists off temporarily so that we don't lose what we've parsed so
            # far.
            macro_products="$products"
            macro_processors="$processors"
            macro_yesnoOptions="$yesnoOptions"
            macro_paramOptions="$paramOptions"
            macro_buildTargets="$buildTargets"

            PrintBuildOptions Build options before macro

            # Next, clear the macro options variable (in case a previous macro
            # had been processed), and source the macro script to get its
            # options.
            macro_options=""
            source $macroScript

            # Now recurse to process the options.
            ParseParameters $macro_options

            # Finally, append the parsed options to the lists that we saved
            # off.
            RemoveExtraSpaces $macro_products $products
            products=$removeExtraSpacesResult

            RemoveExtraSpaces $macro_processors $processors
            processors=$removeExtraSpacesResult

            RemoveExtraSpaces $macro_yesnoOptions $yesnoOptions
            yesnoOptions=$removeExtraSpacesResult

            RemoveExtraSpaces $macro_paramOptions $paramOptions
            paramOptions=$removeExtraSpacesResult

            RemoveExtraSpaces $macro_buildTargets $buildTargets
            buildTargets=$removeExtraSpacesResult

            PrintBuildOptions Build options after macro

            continue

        fi

        # See if this parameter is a processor.  If so, add it to the list of
        # processors that will be built.
        for p in $validProcessors ; do

            if [[ "$parm" == "$p" ]] ; then

                RemoveExtraSpaces $processors $parm
                processors=$removeExtraSpacesResult

                continue 2
            fi

        done

        # See if this is a yesno parameter.  If it matches the current token (or
        # the current token with a 'no' in front), add it to the list.
        for p in $validYesnoOptions ; do

            if [[ "$parm" == "$p" || "$parm" == "no$p" ]] ; then

                RemoveExtraSpaces $yesnoOptions $parm
                yesnoOptions=$removeExtraSpacesResult

                # This is an odd case.  We'd like to have a superslim macro to
                # set the new defaults, but we can't have a macro with the same
                # name as a valid parameter.
                if [[ "$parm" == "superslim" ]] ; then
                    RemoveExtraSpaces $paramOptions c 45
                    paramOptions=$removeExtraSpacesResult
                fi
            
                continue 2
            fi

        done

        # See if this parameter is an option that has a value associated with it.
        # If so, add it (and the value that follows) to the list.  Since we are
        # consuming the second parameter, we need to do an extra shift here.
        for p in $validParamOptions ; do

            if [[ "$parm" == "$p" ]] ; then

                RemoveExtraSpaces $paramOptions $parm $1
                paramOptions=$removeExtraSpacesResult

                shift
                continue 2
            fi

        done

        # See if this parameter is one of the old-style compression options.  If
        # so, translate it into a new-style parameter option.
        for p in $validOldComprOptions ; do

            if [[ "$parm" == "c$p" ]] ; then

                RemoveExtraSpaces $paramOptions c $p
                paramOptions=$removeExtraSpacesResult

                continue 2
            fi

        done

        # Otherwise, assume that this is a build target.  Add this to the list of
        # values that will be passed to make.
        RemoveExtraSpaces $buildTargets $parm
        buildTargets=$removeExtraSpacesResult

    done

    # Display the options that were parsed from the command line.
    PrintBuildOptions After parsing commandline parameters

}

#----------------------------------------------------------------------------

function BuildProduct {

    # The product-specific script name is used several times; build the string
    # for it once.  I know that it exists in one of two places; default to the
    # BFC OS-specific directory, but use the system-specific custom directory
    # if the script exists there.
    productScript="${BfcOsCustomDir}/${1}_settings.bash"
    if [[ -e ${SystemCustomDir}/${1}_settings.bash ]] ; then
        productScript="${SystemCustomDir}/${1}_settings.bash"
    fi

    # Save the abbreviated script name for MergedNonVolDefaults.h
    product_script=$1

    # Store these off; they will be appended to whatever we parse from the
    # product-specific settings.
    orig_yesnoOptions=$yesnoOptions
    orig_paramOptions=$paramOptions
    orig_buildTargets=$buildTargets

    # Clear these variables.
    app_options=""
    product_name=""

    echo "Loading additional build options from ${productScript}..."

    # Run the settings script to configure the product-specific build options.
    # It should set a variable called app_options with the values.
    source $productScript

    # Verify that the script configured the correct variable.
    if [[ "$app_options" == "" ]] ; then
        echo
        echo "ERROR - $productScript didn't configure app_options with the product-specific build options!  Can't build!"
        echo

        return 1
    fi

    # Warn about setting deprecated variables.
    if [[ "$bsp_options" != "" ]] ; then
        echo
        echo "WARNING - $productScript set deprecated bsp_options.  This variable is no longer used..."
        echo
    fi

    # Parse these values, separating them out into the appropriate lists.
    ParseParameters $app_options

    # Make sure no products were specified; product nesting is not allowed.
    if [[ "$products" != "" ]] ; then
        echo
        echo "ERROR - $productScript specified another product script '$products'; product nesting not supported!  Can't build!"
        echo

        return 1
    fi

    # Make sure exactly one processor was specified.
    # UNFINISHED - for now, just enforce that at least one is specified, and
    # hope that they don't do more than one.
    if [[ "$processors" == "" ]] ; then
        echo
        echo "ERROR - $productScript must set one, and only one, valid processor!  Can't build!"
        echo

        return 1
    fi

    # Parsing completed successfully.  Append the original command line
    # parameters that were parsed; this allows command line settings to
    # override what's in the product-specific script.
    if [[ "$yesnoOptions" == "" ]] ; then
        yesnoOptions=$orig_yesnoOptions
    else
        yesnoOptions="$yesnoOptions $orig_yesnoOptions"
    fi

    if [[ "$paramOptions" == "" ]] ; then
        paramOptions=$orig_paramOptions
    else
        paramOptions="$paramOptions $orig_paramOptions"
    fi

    if [[ "$buildTargets" == "" ]] ; then
        buildTargets=$orig_buildTargets
    else
        buildTargets="$buildTargets $orig_buildTargets"
    fi

    # Display the options after parsing from the product-specific file.
    PrintBuildOptions After merging product-specific build options

    # Set the target processor that this build is for.  This is used when
    # setting default environment variables.
    buildProcessor=${processors}

    # The output directory is just the product name.
    outputDir=$1

    # Now do the build thing.
    BuildIt
    return $?

}

#----------------------------------------------------------------------------

function BuildProcessor {

    # Set the target processor that this build is for.  This is used when
    # setting default environment variables.
    buildProcessor=$1

    # We're not building a product, so clear related variables.
    product_script=""
    product_name=""

    # The output directory needs to be built from the target platform parameter.
    outputDir=bcm9$1

    # Now do the build thing.
    BuildIt
    BfcBuildResult=$?

    # Preserve the build result across the endlocal.
    #
    # Do something that will cause errorlevel to be set to non-0 so that we can
    # break out of the build loop (cd to a non-existant directory).
    if [[ $BfcBuildResult != 0 ]] ; then

        echo "*"
        echo "* Failed to build ${buildProcessor}!"
        echo "*"
    fi

    return $BfcBuildResult

}

#----------------------------------------------------------------------------

# Save the current time for the elapsed build time.
function GetBuildStartTime {

    # To provide the elapsed time for a build, first save the current time.
    s_time=`date '+%j %H %M %S'`

}

#----------------------------------------------------------------------------

# Compute and print the elapsed build time.
#    Input - s_time must be set by timer_start
function DisplayElapsedBuildTime {

    # Get and print the elapsed time for this build job.
    e_time=`date '+%j %H %M %S'`

    echo
    echo "$e_time $s_time" | awk '{et=((($1*24+$2)*60+$3)*60+$4)-((($5*24+$6)*60+$7)*60+$8);es=et%60;em=int(et/60);printf ("\n---> Elapsed time  %dm %02ds\n\n",em,es)}'
    echo

}

#----------------------------------------------------------------------------

function PrintBuildOptions {

    # Don't print this info if not requested.
    if [ "$debug" != "1" ] ; then
        return
    fi

    # Since the clean target can be stripped out, we want to add it back for
    # display purposes.
    buildTargetsWithClean="$buildTargets"
    if [[ "$buildClean" == "1" ]] ; then
        buildTargetsWithClean="clean${buildTargetsWithClean}"
    fi

    echo
    echo "$*:"
    echo
    echo "products     = '$products'"
    echo "processors   = '$processors'"
    echo "yesnoOptions = '$yesnoOptions'"
    echo "paramOptions = '$paramOptions'"
    echo "buildTargets = '$buildTargetsWithClean'"
    echo

}

#----------------------------------------------------------------------------

function ConfigureEnvironment {

    # Process the yes/no and parameter options, allowing them to override the
    # default values.  These are done in helper "functions" so that we can use
    # the shift command to iterate more easily.
    #
    # Note that we prepend the default values so that they will be processed first,
    # then overridden by any values specified by the user.  Also note that the PID
    # parameter has to be done here since we need to know the processor in order to
    # know the default value.
    ProcessBuildProcessor $buildProcessor
    ProcessYesNoOptions $defaultYesnoOptions $yesnoOptions
    ProcessParamOptions pid $buildProcessor $defaultParamOptions $paramOptions
    ProcessBuildTargets $buildTargets

    # Now that we finished evaluating the options, configure the output directory
    # suffix.
    outputDirSuffix=""

    # Call the extension script (if it exists) to generate its own output
    # directory suffix.
    ConfigureOutputDirSuffixExt

    # If this build uses a macro file, put the macro name in the suffix.
#    if [[ "$macroScript" != "" ]] ; then
#        local macro
#        macro=${macroScript##*/}
#        macro=${macro%_macro.bash*}
#        if [ "$macro" != "" ] ; then
#            outputDirSuffix=${outputDirSuffix}_${macro}
#        fi
#    fi
    
    # The 'sip' option changes the output directory name.
    if [[ ${opt_sip} == 1 ]] ; then
        outputDirSuffix=${outputDirSuffix}_sip
    fi

    # The 'smp' option changes the output directory name.
    if [[ ${opt_smp} == 1 ]] ; then
        outputDirSuffix=${outputDirSuffix}_smp
    fi

    # The 'ipv6' option changes the output directory name.
    if [[ ${opt_ipv6} == 1 ]] ; then
        outputDirSuffix=${outputDirSuffix}_ipv6
    fi

    # The 'slim' options changes the output directory name.
    if [[ $opt_slim == 1 ]] ; then
        outputDirSuffix=${outputDirSuffix}_slim
    elif [[ $opt_slim == 2 ]] ; then
        outputDirSuffix=${outputDirSuffix}_superslim
    fi

    if [[ $opt_show == 1 ]] ; then
        typeset -x
        exit
    fi

}

#----------------------------------------------------------------------------

function CreateUserName {

    # This is used as part of the version banner to show who built the image,
    # giving some amount of traceability for the origin of a particular image.

    echo "Creating username.h..."
    echo "#define USERNAME \"${USER}\"" >username.h.new


    # A new username.h is only generated if it is different from the existing
    # one. Otherwise, the old username.h is retained. This prevents source
    # files that include username.h from being re-compiled unnecessarily
    # every iteration due to dependency files.
    cmp -s username.h username.h.new || cp -f username.h.new username.h
    rm -f username.h.new

}

#----------------------------------------------------------------------------

function CreateBuildDateAndTimeWithWeedkay {

    # This creates an optional build timestamp that includes the weekday

    echo "Creating bcm_date_weekday.h..."
    date=`date +"%A, %b %d %Y"`
    echo "#define BCM_DATE_WEEKDAY \"$date\"" >bcm_date_weekday.h

}

#----------------------------------------------------------------------------

function RemoveExtraSpaces {

    removeExtraSpacesResult=""

    while [ $# != 0 ] ; do
        if [[ "$removeExtraSpacesResult" == "" ]] ; then
            removeExtraSpacesResult="$1"
        else
            removeExtraSpacesResult="${removeExtraSpacesResult} $1"
        fi
        shift
    done

}

#----------------------------------------------------------------------------

function CopyCompanyLogo {

    # Copy the company logo header file to the output directory.  Use the customer
    # specific file if present.

    echo "$CompanyLogoFile"

    if [[ "$CompanyLogoFile" != "" ]] ; then
        if [[ -e ../$CompanyLogoFile ]] ; then
            echo
            echo "Using customer-specific Company Logo header file:"
            echo "    $CompanyLogoFile"

            cp ../${CompanyLogoFile} _CompanyLogo.h.new
        fi
    fi

    if [[ ! -e _CompanyLogo.h.new ]] ; then
        echo
        echo "Using default BRCM Company Logo header file."

        cp ../${rootDirPath}Bfc/BfcApp/BrcmCompanyLogo.h _CompanyLogo.h.new
    fi


    # A new _CompanyLogo.h is only generated if it is different from the existing
    # one. Otherwise, the old _CompanyLogo.h is retained. This prevents source
    # files that include _CompanyLogo.h from being re-compiled unnecessarily
    # every iteration due to dependency files.
    cmp -s _CompanyLogo.h _CompanyLogo.h.new || cp -f _CompanyLogo.h.new _CompanyLogo.h
    rm -f _CompanyLogo.h.new

}

#----------------------------------------------------------------------------

function CreateMergedNonVolDefaults {

    echo
    echo Creating MergedNonVolDefaults.h...

    if [[ -e MergedNonVolDefaults.h ]] ; then
      mv -f MergedNonVolDefaults.h MergedNonVolDefaults.h.old
    fi


    # Create a file containing the "don't modify yourself" banner.
    echo "// ==========================================================================" >>_AutoHeader.h
    echo "// WARNING - THIS FILE WAS GENERATED BY Bfc/linux/functions.bash!  ANY CHANGES" >>_AutoHeader.h
    echo "//           THAT YOU MAKE TO THIS FILE MANUALLY WILL BE DESTROYED THE NEXT  " >>_AutoHeader.h
    echo "//           TIME YOU BUILD!                                                 " >>_AutoHeader.h
    echo "// ==========================================================================" >>_AutoHeader.h
    echo "" >>_AutoHeader.h

    # Blast out the file containing the Snoop priority values, along with the
    # file containing the nonvol default values.
    cat _AutoHeader.h ../${BfcCustomDir}/BfcSnoopPriorities.h ../${BfcCustomDir}/BfcNonVolDefaults.h  ../${BfcCustomDir}/BfcParentalControlDefaults.h ../${rootDirPath}Stb/EstbNonVolDefaults.h >MergedNonVolDefaults.h 

    # Get rid of the header file.
    rm -f _AutoHeader.h

    if [ "$product_name" != "" ] ; then
        echo "#define BFC_PRODUCT_NAME   \"$product_name\""     >>MergedNonVolDefaults.h
    fi
    if [ "$product_script" != "" ] ; then
        echo "#define BFC_PRODUCT_SCRIPT \"$product_script\""   >>MergedNonVolDefaults.h
    fi

    # Call extension script (if it exists) to do app-specific modifications to
    # MergedNonVolDefaults.h.
    CreateMergedNonVolDefaultsExt


    # A new MergedNonVolDefaults.h is only generated if it is different from the existing
    # one. Otherwise, the old MergedNonVolDefaults.h is retained. This prevents source
    # files that include MergedNonVolDefaults.h from being re-compiled unnecessarily
    # every iteration due to dependency files.
    cmp -s MergedNonVolDefaults.h.old MergedNonVolDefaults.h && mv -f MergedNonVolDefaults.h.old MergedNonVolDefaults.h
    rm -f MergedNonVolDefaults.h.old

}

#----------------------------------------------------------------------------

function SaveBuildOptions {

    # Write the complete list of build options to makeapp.txt, minus duplicates.
    # The output has two lines - one for yes/no options, and one for options
    # with parameters.

    # Process the yes/no list:

    reverseyesnolist=""

    # Reverse the list...
    for o in $defaultYesnoOptions $yesnoOptions ; do
        reverseyesnolist="$o $reverseyesnolist"
    done
    
    yesnolist=" "

    # For each item in reversed list, if it's not already in the final list, add it
    # to the front.  This un-reverses the list and eliminates overridden options.
    for o in $reverseyesnolist ; do
        # Remove leading "no" to get base option name.
        base_o=$o
        if [ "${o:0:2}" == "no" ] ; then
            base_o=${o:2}
        fi
        # If we haven't already seen the option or a negated version, add to the list.
        if [ "${yesnolist/ $base_o /}" == "${yesnolist}" -a "${yesnolist/ no$base_o /}" == "${yesnolist}" ] ; then
            yesnolist=" $o$yesnolist"
        fi
    done


    # Process the param list:

    reverseparamlist=""

    # Reverse the list...
    for o in $defaultparamOptions $paramOptions ; do
        reverseparamlist="$o $reverseparamlist"
    done
    
    paramlist=" "

    # For each item in reversed list, if it's not already in the final list, add it
    # to the front.  This un-reverses the list and eliminates overridden options.
    val=""
    for o in $reverseparamlist ; do
        if [ "$val" == "" ] ; then
            val=$o
            continue
        fi
        # If we haven't already seen the option, add to the list.
        if [ "${paramlist/ $o /}" == "${paramlist}" ] ; then
            paramlist=" $o $val$paramlist"
        fi
        val=""
    done

    echo "$yesnolist" >  makeapp.txt
    echo "$paramlist" >> makeapp.txt
}

#----------------------------------------------------------------------------

function BuildIt {

    # Store the start time for the build; this is used later to display the
    # elapsed time.
    GetBuildStartTime

    # Evaluate the build options that were specified by the user, and set any
    # default values for the current target processor.  The defaults may be
    # different for each processor, so we have to evaluate this every time we
    # start a build (and inside the setlocal block so that old defaults are not
    # applied).
    ConfigureEnvironment

    # For display purposes, shrink the list of processors to be only the one that
    # is currently being built.
    processors=$buildProcessor

    # Display the build options that are being used for this make job.
    PrintBuildOptions Current build options

    # Append the options suffix to the output directory.
    outputDir=${outputDir}${outputDirSuffix}

    # Make sure the output directory exists.
    if [[ ! -d $outputDir ]] ; then
        mkdir $outputDir
    fi

    # Go to the output directory.
    echo
    echo "Output directory is $outputDir"
    cd $outputDir

    # If we detected a clean target, do the clean build separately.
    if [[ "$buildClean" == "1" ]] ; then
        make -f ../Makefile.mak clean
    fi

    # If clean was the only target, skip the rest.
    if [[ "$buildCleanOnly" == "1" ]] ; then
        return 0
    fi

    # Create username.h, which is used by the version banner.
    CreateUserName

    # Create optional build timestamp with weedkay
    CreateBuildDateAndTimeWithWeedkay

    # Select the appropriate company logo and copy it to the output directory.
    CopyCompanyLogo

    # Generate the MergedNonVolDefaults.h file in the output directory.  This file
    # is included by numerous other files in the system, and it contains values
    # that set default values for nonvol or compile-time constants.
    CreateMergedNonVolDefaults

    # Call the extension script (if it exists) to do whatever pre-processing it
    # needs to do before the build starts.
    PreBuildCalloutExt

    # Save the build options to a file.
    SaveBuildOptions

    # Do our own pre-build thing for the serial port disable case.
    if [[ $BFC_CONFIG_SERIAL_PORT_DISABLED == 1 ]] ; then

        echo ""
        echo "Configuring the build for serial port disable..."
        echo ""

        # Get rid of this file so that it will be rebuilt with serial port
        # disabled.
        rm -f ecroot.o
    fi

    # Handle cache optimization; for linux, we do this through the linker script
    # file.
    if [[ $BCM_OPTIMIZE_CACHE == 1 ]] ; then

        echo " "
        echo "cacheopt was specified; generating custom linker script file."
        echo " "

        # Verify the existance of the linker script file to be inserted into the
        # primary linker script file.
        if [[ -e $CacheOptScript ]] ; then

            # Get rid of the custom linker script file, if it exists.
            rm -f $LINKER_SCRIPT

            # Loop over all of the lines in the primary linker script file,
            # writing them to the custom linker script file.  If we come across
            # the line of interest, insert the custom lines from the file that
            # was set up by the top level script.
            cat ../../../Bfc/linux/target.ld | while read inputLine ; do

                echo "$inputLine" >>$LINKER_SCRIPT

                if [[ "$inputLine" == "*(.text)"* ]] ; then

                    cat $CacheOptScript >>$LINKER_SCRIPT

                fi

            done

        else

            echo " "
            echo "WARNING - cacheopt was selected but the CacheOptScript variable was not set to"
            echo "          a valid script file.  Cache Optimization will be skipped."
            echo "          CacheOptScript = $CacheOptScript"
            echo " "

            BCM_OPTIMIZE_CACHE=0
            LINKER_SCRIPT=../../../Bfc/linux/target.ld

        fi

    fi

    # If the "dualbuild" option was specified, let make start two compile jobs at
    # once.  This makes it run faster on a dual-processor system.
    if [ $opt_dualbuild == 1 ] ; then
        dualbuild="-j 2"
    else
        dualbuild=
    fi

    # Build the app.
    echo make $dualbuild -f ../Makefile.mak $buildTargets
         make $dualbuild -f ../Makefile.mak $buildTargets
    BfcBuildResult=$?

    # Call the external script (if it exists) to do whatever post processing it
    # needs to do before the endlocal occurs.
    PostBuildCalloutExt

    # Do our own pre-build thing for the serial port disable case.
    if [[ $BFC_CONFIG_SERIAL_PORT_DISABLED == 1 ]] ; then

        echo ""
        echo "Cleaning up after serial port disabled build..."
        echo ""

        # Get rid of this file so that it will be rebuilt with whatever the
        # proper option is next time.
        rm -f ecroot.o
    fi

    # Check the result and set a global variable as appropriate, since errorlevel
    # will not be preserved by the cd command below (but it is preserved by the
    # echo off and endlocal commands).
    if [[ $BfcBuildResult != 0 ]] ; then
        echo "*"
        echo "* Build FAILED!"
        echo "*"
    else
        echo "*"
        echo "* Build succeeded..."
        echo "*"
    fi

    # Now calculate and display the elapsed time for the build.
    DisplayElapsedBuildTime

    return $BfcBuildResult
}

# Now export all variables again.  *** IMPORTANT ***  Any environment variables
# should be defined below this point.
set -o allexport

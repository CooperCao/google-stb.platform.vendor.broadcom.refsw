#****************************************************************************
#
# Copyright (c) 2009 Broadcom Corporation
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
#  Filename:       makeapp_ext.bash
#  Author:         David Pullen
#  Creation Date:  Jan 7, 2004
#
#****************************************************************************

# Extension script for the main build script (Bfc/linux/functions.bash).  This
# file does all of the stuff specific to the CmDocsisSystem.
#
# These function have dummy implementations in functions.bash; mine are included
# after those, allowing me to override them.

#----------------------------------------------------------------------------

function SetValidParametersExt {

    # Set additional build parameters that are valid for this system (but not
    # generally for the BFC core).
    validYesnoOptions="${validYesnoOptions} erouter eps cmvendor oms us eu \
        newleds sled emta emtasim emtamaxcpe msc bcm93380_ds_bonding ds_bonding \
        multicast30_support single_ds dspaddr udc mulpi us_bonding"
    validParamOptions="${validParamOptions} docsis num_sids mac14 fpga bpi23"

    defaultYesnoOptions="${defaultYesnoOptions} nocmvendor noerouter noeps nooms us \
        nonewleds nosled noemta noemtasim msc nobcm93380_ds_bonding nods_bonding \
        nomulticast30_support nosingle_ds noudc nomulpi nous_bonding"
    defaultParamOptions="${defaultParamOptions} docsis 11 num_sids 4"

    # These parameters are deprecated and have no effect.  BPI and Propane are
    # always compiled in, and can't be turned off any more.
    validYesnoOptions="${validYesnoOptions} bpi propane"

    # UNFINISHED - need to add support for these
    #
    # emta emtasim cablehome chnolib parental cablemedeart

    # UNFINISHED - do I also want to support h/w rev as a parameter?
    
    SetValidStbParameters
}

function SetValidStbParameters {
    # Set additional build parameters that are valid for this system (but not
    # generally for the BFC core).
    validYesnoOptions="${validYesnoOptions} ecm estb pvnet pci_client ubus_client \
        spilnv rmagnum dsg rswdload ip_rnvol estb_config oob two_ds"

    defaultYesnoOptions="${defaultYesnoOptions} noecm noestb nopvnet nopci_client noubus_client \
        nospilnv normagnum nodsg norswdload noip_rnvol noestb_config oob notwo_ds"
}
#----------------------------------------------------------------------------

function ProcessBuildProcessorExt {

    # $1 contains the current processor target; anything that I need to do?  For
    # now, I don't think so...
    
    # Bash doesn't seem to like empty functions, so this makes it not empty.
    dummy=1
}

#----------------------------------------------------------------------------

function ProcessYesNoOptionsExt {
    
    # The options have already been parsed; need to look at opt_{name} to
    # determine whether it was enabled or disabled.
    if [[ "$opt_cmvendor" == "1" ]] ; then
    
        # Make sure somebody configured the environment with the directory
        # containing the extension, and that the directory exists.
        if [[ ! -e "${rootDirPath}Bfc/make/${CmVendorExtensionMakefile}" ]] ; then

            echo "ERROR - cmvendor option is enabled but couldn't find makefile '${CmVendorExtensionMakefile}' in Bfc/make!"
            echo "        You must configure the environment variable CmVendorExtensionMakefile with the name of the"
            echo "        makefile to use for this product!"
            
            exit 1
            
        elif [[ ! -e "${rootDirPath}Cm/CmExtensionApi/${CmVendorExtensionDir}" ]] ; then
        
            echo "ERROR - cmvendor option is enabled but product directory '${CmVendorExtensionDir}' doesn't exist under Cm/CmExtensionApi!"
            echo "        You must configure the environment variable CmVendorExtensionDir with the name of the directory that contains"
            echo "        product-specific files!"
            
            exit 1
            
        else
            BFC_INCLUDE_CM_VENDOR_SUPPORT=1
        fi
        
    else
        BFC_INCLUDE_CM_VENDOR_SUPPORT=0
    fi
    
    if [[ "$opt_dspaddr" == "1" ]] ; then
      BFC_INCLUDE_MTA_DYNAMIC_BOOTUP_ADDR_SUPPORT=1
    fi
    
    # Process CableHome yes/no options
    if [[ "$opt_eps" == "1" ]] ; then
    
        # Make sure somebody configured the environment with the directory
        # containing the extension, and that the directory exists.
        if [[ ! -e "${rootDirPath}Bfc/make/${ChVendorExtensionMakefile}" ]] ; then

            echo "ERROR - eps option is enabled but couldn't find makefile '${ChVendorExtensionMakefile}' in Bfc/make!"
            echo "        You must configure the environment variable ChVendorExtensionMakefile with the name of the"
            echo "        makefile to use for this product!"
            
            exit 1
            
        elif [[ ! -e "${rootDirPath}CableHome/ChExtensionApi/${ChVendorExtensionDir}" ]] ; then
        
            echo "ERROR - eps option is enabled but product directory '${ChVendorExtensionDir}' doesn't exist under CableHome/ChExtensionApi!"
            echo "        You must configure the environment variable ChVendorExtensionDir with the name of the directory that contains"
            echo "        product-specific files!"
            
            exit 1
            
        else
            BFC_INCLUDE_CH_VENDOR_SUPPORT=1
            BFC_CABLEHOME_SUPPORT=1
        fi
        
    else
        BFC_INCLUDE_CH_VENDOR_SUPPORT=0
    fi
    
    # Process eRouter yes/no options
    if [[ "$opt_erouter" == "1" ]] ; then
    
        BFC_EROUTER_SUPPORT=1
    fi 
    
    # Process EMTA yes/no options.
    BFC_INCLUDE_EMTA_SUPPORT=$opt_emta
    BFC_INCLUDE_MTA_GATEWAY_SUPPORT=$opt_eps
    BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT=1
    
    # PR12290 - select a different cacheopt script for EMTA builds.
    if [[ "$opt_emta" == "1" ]] ; then
    
        CacheOptScript=../EmtaTarget.ld
        
    fi
    
    # Process EMTA sim yes/no options.
    BFC_INCLUDE_EMTA_SIM_SUPPORT=$opt_emtasim
    
    # Process ESTB yes/no options.
    BFC_INCLUDE_ESTB_SUPPORT=$opt_estb
    BFC_USE_ESTB_PRIVATENET=$opt_pvnet
    
    # Process DSG yes/no options.
    BFC_INCLUDE_STB_DSG_SUPPORT=$opt_dsg
    
    # Process RMAGNUM yes/no options.
    BFC_INCLUDE_STB_RMAGNUM_SUPPORT=$opt_rmagnum
    
    # Process OOB yes/no options.
    BFC_INCLUDE_STB_OOB_SUPPORT=$opt_oob
    
    # Process RSWDLOAD yes/no options.
    BFC_INCLUDE_STB_RSWDLOAD_SUPPORT=$opt_rswdload
    
    BCM_VENDOR_OMS=$opt_oms

    # DOCSIS 3.0-related options.
    BFC_MULTICAST30_SUPPORT=$opt_multicast30_support
    BFC_MULPI_SUPPORT=$opt_mulpi
    BFC_DS_BONDING=$opt_ds_bonding
    BFC_US_BONDING=$opt_us_bonding
    BCM93380_DS_BONDING=$opt_bcm93380_ds_bonding
    if [ $opt_bcm93380_ds_bonding == 1 ] ; then
        BFC_DS_BONDING=1
    fi
    
    # The 3254 is like a 3255, but with one downstream.
    BCM_SINGLE_DOWNSTREAM=$opt_single_ds

    # Process PCI_CLIENT yes/no options.
    BFC_INCLUDE_PCI_CLIENT_SUPPORT=$opt_pci_client
    BFC_INCLUDE_UBUS_CLIENT_SUPPORT=$opt_ubus_client
    BFC_INCLUDE_SPI_LOCAL_NONVOL_SUPPORT=$opt_spilnv
    BFC_INCLUDE_IP_REMOTE_FLASH_SUPPORT=$opt_ip_rnvol
    BCM_ESTB_SUPPORTS_CONFIG_FILE_ENCAPSULATION=$opt_estb_config

    # Note that 'nous' is an alias for 'eu', and 'noeu' is an alias for 'us'.
    # Nobody specifies them that way, but it is supported by the parser, so we
    # should support it in the option processor.
    if [[ "$opt_us" == "1" ]] ; then
        SelectUsOption
    else
        SelectEuOption
    fi

    if [[ "$opt_eu" == "1" ]] ; then
        SelectEuOption
    else
        SelectUsOption
    fi

    # Set the build option that controls use of the new visualization LED h/w.
    BCM_USE_NEW_VISUALIZATION_LEDS=$opt_newleds
    
    # Set the build option that controls inclusion of SLED support.
    BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=$opt_sled
    
    # Set the build option that controls the code that generates eDOCSIS DHCP
    # Option43.
    EMBEDDED_CM_SUPPORT=$opt_ecm
    
    BFC_INCLUDE_HPPC=$opt_msc

    # PR12948 - add (optional) support for Upstream Drop Classifiers.
    BFC_INCLUDE_CM_UDC_SUPPORT=$opt_udc

    # The remaining options are all deprecated; just warn the user that they
    # shouldn't use them.
    if [[ "$opt_bpi" != "" ]] ; then
        echo "WARNING - option 'bpi' is deprecated and has no effect."
    fi
    if [[ "$opt_propane" != "" ]] ; then
        echo "WARNING - option 'propane' is deprecated and has no effect."
    fi
    
}

#----------------------------------------------------------------------------

function ProcessParamOptionsExt {

    # $1 contains the option name, and $2 contains the value.

    if [[ "$1" == "docsis" ]] ; then

        docsisVersion=$2
        if [[ "$2" == "1.0" ]] ; then
            docsisVersion=10
        fi
        if [[ "$2" == "1.1" ]] ; then
            docsisVersion=11
        fi
        if [[ "$2" == "2.0" ]] ; then
            docsisVersion=20
        fi

        if [[ "$docsisVersion" != "10" && \
              "$docsisVersion" != "11" && \
              "$docsisVersion" != "20" ]] ; then
            echo "WARNING - illegal value for 'docsis' option - '$2'!  Must be '10', '11', or '20'"
        fi
        
        return 0
    fi

    # UNFINISHED - the value must be 1..16.  Need to validate this.
    if [[ "$1" == "num_sids" ]] ; then
        BCM_NUM_SIDS=$2
        
        return 0
    fi

    # Handle MAC-14 or BPI-23 image.
    if [[ "$1" == "mac14" || "$1" == "bpi23" ]] ; then
        MAC_14_IMAGE=1
        if [[ "$1" == "bpi23" ]] ; then
            opt_bpi23=1
        fi
        cw=$2
        # Handle the common eror of putting "CW" in front of the wave number.
        if [[ "${cw:0:2}" == "cw" || "${cw:0:2}" == "CW" ]] ; then
            cw=${cw:2}
        fi

        # If this expression results in 0, then cw wasn't an integer.
        (( cw = cw + 0 ))
        if [[ $cw == 0 ]] ; then
            echo 'ERROR - MAC-14 cert wave number must be an integer as in "mac14 38"'
            exit 1
        fi

        mac14CertWave=$cw
        unset cw
        
        return 0
    fi

    if [[ "$1" == "fpga" ]] ; then
        case $2 in
            ep2c20|ep2c35|ep2c70)
                FPGA_IMAGE_TYPE=$2
                ;;
            *)
                echo "ERROR - Illegal value for fpga type -'$2'!  Should be ep2c20, ep2c35, or ep2c70."
                exit 1
                ;;
        esac
        return 0
    fi

    # The remaining options are all deprecated; just warn the user that they
    # shouldn't use them.
    echo "WARNING - option '$1' is deprecated and has no effect."

}

#----------------------------------------------------------------------------

function ConfigureOutputDirSuffixExt {

    # If this is a Euro build, change the output directory suffix.  US builds
    # don't modify the suffix.
    if [[ "$country" == "eu" ]] ; then
        outputDirSuffix=${outputDirSuffix}_eu
    fi
    if [[ "$opt_mulpi" == "1" ]] ; then
        outputDirSuffix=${outputDirSuffix}_mulpi
    fi
    # UNFINISHED - we'll need PacketCable and CableHome options, possibly others.

}

#----------------------------------------------------------------------------

function CreateMergedNonVolDefaultsExt {

    echo ""                                                                               >>MergedNonVolDefaults.h
    echo "// =========================================================================="  >>MergedNonVolDefaults.h
    echo "// THE FOLLOWING ENTRIES WERE ADDED BY CmDocsisSystem/vxWorks/makeapp_ext.bat"  >>MergedNonVolDefaults.h
    echo "// =========================================================================="  >>MergedNonVolDefaults.h
    echo ""                                                                               >>MergedNonVolDefaults.h

    # Surround the stuff I include inline with multiple-inclusion protection.
    echo ""                                                                               >>MergedNonVolDefaults.h
    echo "#ifndef CmDocsisSystem_makeapp_ext"                                             >>MergedNonVolDefaults.h
    echo "#define CmDocsisSystem_makeapp_ext"                                             >>MergedNonVolDefaults.h
    echo ""                                                                               >>MergedNonVolDefaults.h


    # First off, I need to make sure the CM-specific file is included.  Rather
    # than appending my file to the existing MergedNonVolDefaults.h, I'll just
    # append a #include statement.
    #echo "#include \"CmDocsisNonVolDefaults.h\""                                          >>MergedNonVolDefaults.h
    echo ""                                                                               >>MergedNonVolDefaults.h


    # Next, check the DOCSIS version that was selected, and adjust the default
    # values appropriately.
    echo "// Selected DOCSIS version:  $docsisVersion                                  "  >>MergedNonVolDefaults.h
    if [[ "$docsisVersion" == "10" ]] ; then
        echo "#undef  kDefaultValue_Docsis11Support                                    "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_Docsis11Support false                              "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_AdvancedPhySupport                                 "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_AdvancedPhySupport false                           "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_DocsisVersionFeatureString                         "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_DocsisVersionFeatureString \"DOCSIS 1.0 \"         "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_SysOrId                                            "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_SysOrId \"1.3.6.1.4.1.4413.2.3.2.1\"               "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_SysOrDescr                                         "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_SysOrDescr \"An agent which supports all MIBs required by the DOCSIS 1.0 OSS specification.\""  >>MergedNonVolDefaults.h
    fi
    if [[ "$docsisVersion" == "11" ]] ; then
        echo "#undef  kDefaultValue_Docsis11Support                                    "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_Docsis11Support true                               "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_AdvancedPhySupport                                 "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_AdvancedPhySupport false                           "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_DocsisVersionFeatureString                         "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_DocsisVersionFeatureString \"DOCSIS 1.0/1.1 \"     "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_SysOrId                                            "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_SysOrId \"1.3.6.1.4.1.4413.2.3.2.2\"               "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_SysOrDescr                                         "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_SysOrDescr \"An agent which supports all MIBs required by the DOCSIS 1.1 OSS specification.\""  >>MergedNonVolDefaults.h
    fi
    if [[ "$docsisVersion" == "20" ]] ; then
        echo "#undef  kDefaultValue_Docsis11Support                                    "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_Docsis11Support true                               "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_AdvancedPhySupport                                 "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_AdvancedPhySupport true                            "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_DocsisVersionFeatureString                         "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_DocsisVersionFeatureString \"DOCSIS 1.0/1.1/2.0 \" "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_SysOrId                                            "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_SysOrId \"1.3.6.1.4.1.4413.2.3.2.3\"               "  >>MergedNonVolDefaults.h
        echo "#undef  kDefaultValue_SysOrDescr                                         "  >>MergedNonVolDefaults.h
        echo "#define kDefaultValue_SysOrDescr \"An agent which supports all MIBs required by the DOCSIS 2.0 OSS specification.\""  >>MergedNonVolDefaults.h
    fi
    echo ""                                                                               >>MergedNonVolDefaults.h


    if [[ "$BFC_CABLEHOME_SUPPORT" == "1" ]] ; then
    echo "#include \"CableHomeNonVolDefaults.h\""                                         >>MergedNonVolDefaults.h
    echo "" 
    fi
    
    # Handle CH vendor extension stuff...
    if [[ "$ChVendorExtensionNonVolDefaults" != "" ]] ; then
        echo "// Including CH vendor-specific nonvol defaults file.                    "  >>MergedNonVolDefaults.h
        echo "#include \"$ChVendorExtensionNonVolDefaults\"                            "  >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    else
        echo "// ChVendorExtensionNonVolDefaults was not defined; vendor-specific      "  >>MergedNonVolDefaults.h
        echo "// nonvol defaults file will not be used!                                "  >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    fi
    
    # Handle EMTA stuff
    if [[ "$BFC_INCLUDE_EMTA_SUPPORT" == "1" ]] ; then
        #echo "EMTA SUPPORT"
        echo "#include \"EmtaNonVolDefaults.h\""                                          >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    
        #echo "EMTA BLIND DATA NONVOL SUPPORT"
        echo "#include \"EmtaBlindDataNonVolDefaults.h\""                                 >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
        
        # Handle EMTA vendor extension stuff...
        # NOTE: this is kind of a placeholder, right now there is no support
        # for EMTA w/ vendor extension.
        if [[ "$EmtaVendorExtensionNonVolDefaults" != "" ]] ; then
            echo "// Including EMTA vendor-specific nonvol defaults file.                ">>MergedNonVolDefaults.h
            echo "#include \"$EmtaVendorExtensionNonVolDefaults\"                        ">>MergedNonVolDefaults.h
            echo ""                                                                       >>MergedNonVolDefaults.h
        else
            echo "// EmtaVendorExtensionNonVolDefaults was not defined; vendor-specific  ">>MergedNonVolDefaults.h
            echo "// nonvol defaults file will not be used!                              ">>MergedNonVolDefaults.h
            echo ""                                                                       >>MergedNonVolDefaults.h
        fi
        
        if [[ "$opt_emtamaxcpe" == "0" ]] ; then
           echo "#undef  kDefaultValue_EmtaIncludedInMaxCpe                               "  >>MergedNonVolDefaults.h
           echo "#define kDefaultValue_EmtaIncludedInMaxCpe false                         "  >>MergedNonVolDefaults.h
        elif [[ "$opt_emtamaxcpe" == "1" ]] ; then
           echo "#undef  kDefaultValue_EmtaIncludedInMaxCpe                               "  >>MergedNonVolDefaults.h
           echo "#define kDefaultValue_EmtaIncludedInMaxCpe true                          "  >>MergedNonVolDefaults.h
        fi
    
        # Handle EMTA SIP stuff
        if [[ "$BFC_INCLUDE_SIP_SUPPORT" == "1" ]] ; then
           #echo "SIP SUPPORT"
           echo "#include \"EmtaSipNonVolDefaults.h\""                                    >>MergedNonVolDefaults.h
           echo ""                                                                        >>MergedNonVolDefaults.h

           # Handle EMTA SIP vendor extension stuff...
           # NOTE: this is kind of a placeholder, right now there is no support
           # for EMTA SIP w/ vendor extension.
           if [[ "$EmtaSipVendorExtensionNonVolDefaults" != "" ]] ; then
               echo "// Including EMTA SIP vendor-specific nonvol defaults file.          ">>MergedNonVolDefaults.h
               echo "#include \"$EmtaSipVendorExtensionNonVolDefaults\"                   ">>MergedNonVolDefaults.h
               echo ""                                                                    >>MergedNonVolDefaults.h
           else
               echo "// EmtaSipVendorExtensionNonVolDefaults was not defined              ">>MergedNonVolDefaults.h
               echo "// nonvol defaults file will not be used!                            ">>MergedNonVolDefaults.h
               echo ""                                                                    >>MergedNonVolDefaults.h
           fi
        fi
    fi 
    
    # Handle ESTB stuff
    if [[ "$BFC_INCLUDE_ESTB_SUPPORT" == "1" ]] ; then
        #echo "ESTB SUPPORT"
        echo "#include \"EstbNonVolDefaults.h\""                                          >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    
        # Handle ESTB vendor extension stuff...
        # NOTE: this is kind of a placeholder, right now there is no support
        # for ESTB w/ vendor extension.
        if [[ "$EstbVendorExtensionNonVolDefaults" != "" ]] ; then
            echo "// Including ESTB vendor-specific nonvol defaults file.                ">>MergedNonVolDefaults.h
            echo "#include \"$EstbVendorExtensionNonVolDefaults\"                        ">>MergedNonVolDefaults.h
            echo ""                                                                       >>MergedNonVolDefaults.h
        else
            echo "// EstbVendorExtensionNonVolDefaults was not defined; vendor-specific  ">>MergedNonVolDefaults.h
            echo "// nonvol defaults file will not be used!                              ">>MergedNonVolDefaults.h
            echo ""                                                                       >>MergedNonVolDefaults.h
        fi
        
    fi 

    # Handle Macro vendor extension stuff...
    if [[ "$MacroVendorExtensionNonVolDefaults" != "" ]] ; then
        echo "// Including MSO vendor-specific nonvol defaults file.                    " >>MergedNonVolDefaults.h
        echo "#include \"${BfcCustomDir}/$MacroVendorExtensionNonVolDefaults\"                          " >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    else
        echo "// MacroVendorExtensionNonVolDefaults was not defined; vendor-specific   "  >>MergedNonVolDefaults.h
        echo "// nonvol defaults file will not be used!                                "  >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    fi
    
    if [[ "$BFC_EROUTER_SUPPORT" == "1" ]] ; then
    echo "#include \"eRouterNonVolDefaults.h\""                                         >>MergedNonVolDefaults.h
    echo "" 
    fi
    
    if [[ "$country" == "us" ]] ; then
        echo "#define BFC_COUNTRY_CODE \"NA\""                                            >>MergedNonVolDefaults.h
    else
        echo "#define BFC_COUNTRY_CODE \"EURO\""                                          >>MergedNonVolDefaults.h
    fi

    
    # DON'T ADD ANYTHING BELOW THIS POINT.

    # Based on the country that was selected, we need to add the appropriate
    # include statement to get board/country specific default settings and
    # overrides.
    echo "// Including product-specific nonvol defaults, if a file was specified.      "  >>MergedNonVolDefaults.h
    if [[ "$country" == "us" ]] ; then
        if [[ "$CmProductDefaults_us" != "" ]] ; then
            echo "#include \"$CmProductDefaults_us\"                                   "  >>MergedNonVolDefaults.h
            echo ""                                                                       >>MergedNonVolDefaults.h
        fi
    else
        if [[ "$CmProductDefaults_eu" != "" ]] ; then
            echo "#include \"$CmProductDefaults_eu\"                                   "  >>MergedNonVolDefaults.h
            echo ""                                                                       >>MergedNonVolDefaults.h
        fi
    fi

    # Handle CM vendor extension stuff...
    if [[ "$CmVendorExtensionNonVolDefaults" != "" ]] ; then
        echo "// Including CM vendor-specific nonvol defaults file.                    "  >>MergedNonVolDefaults.h
        echo "#include \"$CmVendorExtensionNonVolDefaults\"                            "  >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    else
        echo "// CmVendorExtensionNonVolDefaults was not defined; vendor-specific      "  >>MergedNonVolDefaults.h
        echo "// nonvol defaults file will not be used!                                "  >>MergedNonVolDefaults.h
        echo ""                                                                           >>MergedNonVolDefaults.h
    fi
    
    
    # Close the multiple-inclusion protection thing.  THIS MUST BE AFTER THE STUFF
    # THAT I ADD!
    echo ""                                                                               >>MergedNonVolDefaults.h
    echo "#endif // CmDocsisSystem_makeapp_ext                                         "  >>MergedNonVolDefaults.h
    echo ""                                                                               >>MergedNonVolDefaults.h

}

#----------------------------------------------------------------------------

function PreBuildCalloutExt {

    # If this is a MAC14 build, there are several things to do.
    if [[ "$MAC_14_IMAGE" == "1" ]] ; then

        echo ""
        echo "Configuring the build for MAC14/BPI23 image (CW${mac14CertWave})..."
        echo ""

        # First, write the necessary information to mac14_image_name.h so that we
        # will report the correct value via SNMP.
        rm -f mac14_image_name.h
        if [[ "$opt_bpi23" == "1" ]] ; then
            echo "#define MAC_14_IMAGE_NAME \"BPI-23 test image for CW${mac14CertWave}\"">mac14_image_name.h
        else
            echo "#define MAC_14_IMAGE_NAME \"MAC-14 test image for CW${mac14CertWave}\"">mac14_image_name.h
        fi

        # Then remove the .o files for modules that make use of the MAC_14_IMAGE
        # define so that they will be recompiled and use MAC14 values.
        rm -f docsDevBridge.o CmSnmpAgent.o
    else
        echo "#undef MAC_14_IMAGE_NAME">mac14_image_name.h.new
    
        # A new mac14_image_name.h.new is only generated if it is different from the existing
        # one. Otherwise, the old mac14_image_name.h.new is retained. This prevents source
        # files that include mac14_image_name.h.new from being re-compiled unnecessarily 
        # every iteration due to dependency files.
        cmp -s mac14_image_name.h mac14_image_name.h.new || cp -f mac14_image_name.h.new mac14_image_name.h
        rm -f mac14_image_name.h.new
    fi

}

#----------------------------------------------------------------------------

function PostBuildCalloutExt {

    # If this is a MAC14 build, we need to remove all evidence of it.
    if [[ "$MAC_14_IMAGE" == "1" ]] ; then

        echo ""
        echo "Cleaning up after MAC14/BPI23 build..."
        echo ""

        rm -f mac14_image_name.h docsDevBridge.o CmSnmpAgent.o
    fi
    
}

#----------------------------------------------------------------------------

function SelectUsOption {

    # The V2 build script used this to select the appropriate XTALFREQ value.
    # However, the processors we support with BFC all use a single crystal for
    # both, so this isn't necessary.
    #
    # It does, however, play a part in selecting the default DPM coefficients,
    # the Annex mode, and several AGI register values, so I'll take note of the
    # selected country here for use during CreateMergedNonVolDefaults
    country=us

}

#----------------------------------------------------------------------------

function SelectEuOption {

    # Same comments as in SelectUsOption
    country=eu

    # Select the Euro CablexChagne library as default.  This may be overridden
    # in other customization files.
    export BFC_MTA_LIBRARY_SUFFIX="_euro"

}


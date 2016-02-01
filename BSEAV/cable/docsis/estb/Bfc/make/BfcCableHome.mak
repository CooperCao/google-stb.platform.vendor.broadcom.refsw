##############################################################################
#
# Copyright (c) 2003-2008 Broadcom Corporation
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
##############################################################################
#  $Id$
#
#  Filename:       BfcCableHome.mak
#  Author:         John McQueen
#  Creation Date:  November 24, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build components for CableHome.
#
##############
#
# External Variables (input/output):
#
#	BFC_OBJECTS - the list of object files that need to be built for the
#		      project based on the current configuration.
#
#	BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#			       to the compiler.  This may be deprecated in the
#			       future, in favor of just using BfcSystemConfig.h.
#
#	BFC_INCLUDE_DIRS - list of directories where header files can be found;
#			   this will be passed to the compiler.
#
#	BFC_SOURCE_DIRS - list of directories where source files can be found;
#			  this is used by the Make utility.
#
#	BFC_LIBRARIES - list of libraries that are to be used when linking the
#			application.
#
# External Constants (input only; do not modify these):
#
#	BFC_TARGET_OS - the OS that the project is being built for (i.e. Win32,
#			VxWorks, Ecos).  This can be used when making decisions
#			about which files/directories/libararies to include.
#
#	BFC_TARGET_PROCESSOR - the processor that the project is being built
#			       for (i.e. Intel, MIPS32, etc).  This can be used
#			       when making decisions about which
#			       files/directories/libararies to include.
#
#	BFC_ROOT_DIR - the "mount point" for the source code, relative to the
#		       output directory for the project.  This is defined in
#		       the top-level makefile for the OS/target.
#
#	BFC_MAKE_DIR - this is essentially $(BFC_ROOT_DIR)/Bfc/make, and is
#		       the directory where the core BFC component makefiles
#		       are located.
#
##############
#
# NOTE:  This file is geared toward the GNU Make utility, and may use any
#        syntax that is valid for recent-vintage GNU Make.
#
##############################################################################


##############################################################################
#
# Paths - definitions of relative paths for this component.  These are used
#         below, and should be defined once, rather than using the actual
#         value in multiple places.  You can specify multiple directories here
#	  if necessary, but you should have one variable per directory.
#
#         Note that the path needs to be specified relative to the "mount point"
#         of the parent project, not relative to the output directory.  The
#         make processing will specify the rest of the path for you.  For
#	  example, if the Bfc directory is in c:\source\projects\bfcprojects\Bfc
#	  then c:\source\projects\bfcprojects is the mount point.  You will only
#	  specify the path beyond this point (i.e. Bfc).

BFC_CABLEHOME_DIR = CableHome

BFC_CH_PSDEV_DIR     = $(BFC_CABLEHOME_DIR)/PsDev
BFC_CH_CAP_DIR       = $(BFC_CABLEHOME_DIR)/Cap
BFC_CH_CDP_DIR       = $(BFC_CABLEHOME_DIR)/Cdp
BFC_CH_CTP_DIR       = $(BFC_CABLEHOME_DIR)/Ctp
BFC_CH_CQP_DIR       = $(BFC_CABLEHOME_DIR)/Cqp
BFC_CH_CSP_DIR       = $(BFC_CABLEHOME_DIR)/Csp
BFC_CH_CMP_DIR       = $(BFC_CABLEHOME_DIR)/Cmp
BFC_CH_FW_DIR        = $(BFC_CABLEHOME_DIR)/Firewall
BFC_CH_OSS_DIR       = $(BFC_CABLEHOME_DIR)/Oss
BFC_CH_API_DIR       = $(BFC_CABLEHOME_DIR)/Api
BCM_CH_IPHELPERS_DIR = $(BFC_CABLEHOME_DIR)/IpHelpers
BFC_CH_KERB_DIR      = $(BFC_CABLEHOME_DIR)/Kerberos
BFC_CH_EDOCSIS_DIR   = $(BFC_CABLEHOME_DIR)/eDOCSIS


BFC_CH_ARP_HELPERS_DIR = Bfc/IpHelpers/Arp 
BFC_CH_HTTPS_DIR     = Bfc/IpHelpers/Https

BFC_CH_HTTPS_SSL_DIR = Bfc/LibSupport/OpenSSL098/include

BFC_CH_PARENTAL_CONTROL_DIR = Bfc/IpHelpers/ParentalControl 
BFC_CH_TLS_HELPERS_DIR = Bfc/IpHelpers/Tls
BFC_CH_DNS_DIR = ResidentialGateway/Dns

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.




BFC_CABLEHOME_OPEN_OBJECTS += \
        NatIpServiceAppIf.o \
        NatRelaySnoop.o \
        NatSnoop.o \
        NatRamDatabaseIf.o \
        NatDatabaseApi.o \
        NatDatabaseFactory.o \
        DynamicNatSnoop.o 


BFC_CABLEHOME_OPEN_OBJECTS += \
        RgGatewayIpServiceManagerChEventHandler.o \
        PacketSwitchBridgingHalIf.o \
        CapNonVolSettings.o \
        CdpNonVolSettings.o \
        PsDevNonVolSettings.o \
        RgNonVolSettings.o \
        CspNonVolSettings.o \
        CapPassThroughTable.o \
        CapPassThroughEntry.o \
        CapMappingEntry.o \
        CdpLanAddrEntry.o \
        CdpWanDataAddrEntry.o \
        CdpWanDataAddrTable.o \
        CdpWanDataServerAddrEntry.o \
        CtpIpServiceAppIf.o \
        CtpSnoop.o \
        CableHomeConnSpeedTool.o \
        UsfsSnoop.o \
        UsfsIpServiceAppIf.o \
        AccessControlSnoop.o \
        ArpFilterSnoop.o \
        FirewallWanLanSnoop.o \
        FirewallThread.o \
        NatMacAddrSpoofSnoop.o \
        MacAddrSpoofIpServiceAppIf.o \
        ArpTable.o \
        ArpPacketManagerThread.o \
        ArpSnoop.o \
        ArpIpServiceAppIf.o \
        DropBroadcastSnoop.o \
        IpDatagramFragmentReassembler.o \
        IpDatagramFragmenter.o \
        bcmchVendorConfig.o \
        bcmchPingToolApi.o \
        bcmchConnSpeedToolApi.o \
        bcmchCtlThreadApi.o \
        bcmchVendorApi.o \
        DhcpDiscoverDirectSnoop.o \
        DhcpcOfferSnoop.o \
	DhcpcClientGleanSnoop.o \
        DhcpServerSnoop.o \
        DhcpServerThread.o \
        DhcpServerIf.o \
        DhcpServerLease.o \
        CableHomeDhcpServerSnoop.o \
        CableHomeDhcpServerIpServiceAppIf.o \
        CableHomeDhcpServerIpServiceAppEventACT.o \
        CableHomeSnoopFactory.o \
        CableHomePingTool.o \
        CableHomeStatusEventCodes.o \
        CableHomeWanManIpHelper.o \
        CableHomeWanDataIpHelper.o \
        CableHomeIpStackHelper.o \
        CableHomeTodIpHelper.o \
        CableHomeCapIpHelper.o \
        CableHomeIpServiceAppIf.o \
        CableHomeApplication.o \
        CableHomeCommonApplication.o \
        CableHomeTimeOfDayThread.o \
        CableHomeCtlThread.o \
        CableHomeCtlThreadDhcpClientEventACT.o \
        KerberosThreadCableHomeStatusClientACT.o \
        KerberosThread.o \
        ReassemblySnoop.o \
	FragmentSnoop.o \
	MulticastFilterSnoop.o \
        WanFilterSnoop.o 
        
 
         
ifeq ($(BFC_TARGET_OS), VxWorks)
    BFC_CABLEHOME_OPEN_OBJECTS  += res_send.o
endif
        

BFC_CABLHOME_SNMP_OBJECTS = \
        clabdefMib.o \
        clabdefBridge.o \
        capMib.o \
        capBridge.o \
        cdpMib.o \
        cdpBridge.o \
        ctpBridge.o \
        ctpMib.o \
        psdevBridge.o \
        psdevMib.o \
        pscsaBridge.o \
        pscsaMib.o \
        secBridge.o \
        secMib.o


BFC_CABLHOME_SNMP_OBJECTS += \
        CableHomeSnmpAgent.o \
        CableHomeCpeSnmpAgent.o \
        ChSnmpAgentCableHomeStatusACT.o \
        CableHomeEventLog.o \
        CableHomeEventLogNonVolSettings.o \
        chDocsDevMib.o \
        cablehometrap.o \
        ChStubhalIf.o \
        chDocsDevSyslog.o \
        CableHomeNetToMediaSnoop.o\
        CableHomeIfEntryBridge.o \
        PsDevAccessControlTable.o \
        PsDevAccessControlEntry.o \
        CspSnmpIf.o \
        CdpSnmpIf.o \
        CapSnmpIf.o \
        esafeSnmpIf.o \
        CmpNonVolSettings.o \
        CableHomeTftpIpHelper.o \
        CspSecFwPolicyFileThread.o \
        CableHomeAggregateLanHalIf.o \
        CableHomeAggregateWirelessLanHalIf.o 

BFC_CABLEHOME_FACTORY_OBJECTS = \
        CableHomeFactoryEventLog.o \
        CableHomeFactorySnmpAgent.o \
        brcmCableHomeFactoryMib.o \
        brcmCableHomeFactoryBridge.o \
        chCabledataFactoryBridge.o
        
BFC_CABLEHOME_BRCM_SNMP_OBJECTS =

#
# CableHome CSA SNMP support
#

BFC_CABLHOME_SNMP_OBJECTS += \
        rip2mibMib.o \
        rip2mibBridge.o \
        ipCidrRouteBridge.o \
        ipCidrRouteMib.o 


ifeq ($(BCM_KERBEROS_SUPPORT),1)
    BFC_CABLEHOME_OPEN_OBJECTS += KerbSnmpConfig.o KerbApi.o Csocket.o
endif

                   
# Optional console support files.

BFC_CABLEHOME_CONSOLE_OBJECTS = \
        CapNonVolSettingsCommandTable.o \
        CdpNonVolSettingsCommandTable.o \
        PsDevNonVolSettingsCommandTable.o \
        CspNonVolSettingsCommandTable.o \
        CableHomeCommandTable.o \
        DnsCommandTable.o \
        DhcpServerCommandTable.o
        

BFC_CABLEHOME_SNMP_CONSOLE_OBJECTS = \
        CmpNonVolSettingsCommandTable.o \
        CableHomeSnmpAgentConsole.o \
        EventLogCommandTable.o \
        EventLogNonVolSettingsCommandTable.o \
        

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.


BFC_CABLEHOME_DEFINES += BCM_CABLEHOME_SUPPORT=1  

BFC_CABLEHOME_DEFINES += BFC_DNS_RESOLVER_SUPPORT=1 BCM_DNS_LAN_RESOLVER=1 

ifeq ($(BCM_KERBEROS_SUPPORT),1)
    BFC_CABLEHOME_DEFINES += DEMO=1 KERB_SUPPORT=1 BCM_KERBEROS_SUPPORT=1
endif

ifeq ($(buildProcessor), 5365)
    BFC_CABLEHOME_DEFINES += BCM_5365_SWITCH_INCLUDED=1
endif

ifeq ($(buildProcessor), 5352)
    BFC_CABLEHOME_DEFINES += BCM_5352_SWITCH_INCLUDED=1
endif

ifeq ($(buildProcessor), 1112)
    BFC_CABLEHOME_DEFINES += BCM_1112_SWITCH_INCLUDED=1
endif






##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_CABLEHOME_INCLUDE_DIRS += \
        $(BFC_CABLEHOME_DIR) \
        $(BFC_CH_PSDEV_DIR) \
        $(BFC_CH_CAP_DIR) \
        $(BFC_CH_API_DIR) \
        $(BFC_CH_CDP_DIR) \
        $(BFC_CH_CTP_DIR) \
        $(BFC_CH_CQP_DIR) \
        $(BFC_CH_CSP_DIR) \
        $(BFC_CH_FW_DIR) \
        $(BFC_CH_DNS_DIR) \
        $(BFC_CH_CMP_DIR) \
        $(BFC_CH_OSS_DIR) \
        $(BFC_CH_HTTPS_DIR) \
        $(BFC_CH_HTTPS_SSL_DIR) \
        $(BFC_CH_PARENTAL_CONTROL_DIR)  \
        $(BFC_CH_ARP_HELPERS_DIR) \
        $(BCM_CH_IPHELPERS_DIR) \
        $(BFC_CH_KERB_DIR) \
        $(BFC_CH_TLS_HELPERS_DIR) \
        $(BFC_CH_EDOCSIS_DIR) \
        $(BFC_DDNS_DIR)

#ifeq ($(BCM_KERBEROS_SUPPORT),1)
        BFC_CH_KERB_APPINC = $(BFC_CH_KERB_DIR)/appInc
        BFC_CABLEHOME_INCLUDE_DIRS += $(BFC_CH_KERB_APPINC)
#endif

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_CABLEHOME_SOURCE_DIRS += \
        $(BFC_CABLEHOME_DIR) \
        $(BFC_CH_PSDEV_DIR) \
        $(BFC_CH_CAP_DIR) \
        $(BFC_CH_API_DIR) \
        $(BFC_CH_CDP_DIR) \
        $(BFC_CH_CTP_DIR) \
        $(BFC_CH_CQP_DIR) \
        $(BFC_CH_CSP_DIR) \
        $(BFC_CH_FW_DIR) \
        $(BFC_CH_DNS_DIR) \
        $(BFC_CH_CMP_DIR) \
        $(BFC_CH_OSS_DIR) \
        $(BFC_CH_HTTPS_DIR) \
        $(BFC_CH_PARENTAL_CONTROL_DIR) \
        $(BFC_CH_ARP_HELPERS_DIR) \
        $(BCM_CH_IPHELPERS_DIR) \
        $(BFC_CH_KERB_DIR) \
        $(BFC_CH_TLS_HELPERS_DIR) \
        $(BFC_CH_EDOCSIS_DIR)  \
        $(BFC_DDNS_DIR)
        
        
        
ifeq ($(BCM_KERBEROS_SUPPORT),1)
    BFC_CABLEHOME_SOURCE_DIRS += $(BFC_CH_KERB_DIR)/appKerb
endif


##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

ifeq ("$(BCM_CABLEHOME_USE_LIBRARY)", "1")
    ifeq ($(BCM_CABLEMEDEA_RETAIL_SUPPORT),1)
        BFC_CABLEHOME_LIBRARIES += $(CABLEHOME_DIR)/$(BFC_TARGET_OS)$(board)_libcablemedea_rt.a
    else
        BFC_CABLEHOME_LIBRARIES += $(CABLEHOME_DIR)/$(BFC_TARGET_OS)$(board)_libcablehome.a
    endif    
endif

# Change OpenSSL paths if using the latest OpenSSL and if we are eCos.	
ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
    OPENSSL_DIR = Bfc/LibSupport/OpenSSL098
else
    OPENSSL_DIR = Bfc/LibSupport/OpenSSL
endif


ifeq ($(BCM_KERBEROS_SUPPORT),1)

BFC_CH_KERB_LIBS_DIR  = $(BFC_CH_KERB_DIR)/Libs

ifeq ($(BFC_TARGET_OS),VxWorks)
  BFC_CABLEHOME_LIBRARIES += \
        $(BFC_CH_KERB_LIBS_DIR)/kerbVx.a \
        $(BFC_CH_KERB_LIBS_DIR)/SecLibVxDemo.a \
        $(OPENSSL_DIR)/libOpenSSlVxworks.a \
        $(BFC_CH_KERB_LIBS_DIR)/parseLibVx.a \
        $(BFC_CH_KERB_LIBS_DIR)/libbaseosVX.a
else
    ifneq ($(BFC_INCLUDE_OPENSSL098), 1)
        BFC_CABLEHOME_LIBRARIES += $(BFC_CH_KERB_LIBS_DIR)/kerbEcos.a 
        ifneq ($(BFC_INCLUDE_EMTA_SUPPORT),1)
            BFC_CABLEHOME_LIBRARIES += $(BFC_CH_KERB_LIBS_DIR)/SecLibEcosDemo.a
        endif 
        BFC_CABLEHOME_LIBRARIES += $(OPENSSL_DIR)/libOpenSSLecos.a 
        BFC_CABLEHOME_LIBRARIES += $(BFC_CH_KERB_LIBS_DIR)/parseLibEcos.a 
    else   
	# Using OpenSSL 0.9.8
	BFC_CABLEHOME_LIBRARIES += $(BFC_CH_KERB_LIBS_DIR)/kerbEcos.a 
        ifneq ($(BFC_INCLUDE_EMTA_SUPPORT),1)
            BFC_CABLEHOME_LIBRARIES += $(BFC_CH_KERB_LIBS_DIR)/SecLibEcosDemo_openssl098.a
        endif 
        BFC_CABLEHOME_LIBRARIES += $(OPENSSL_DIR)/libOpenSSLecos.a 
        BFC_CABLEHOME_LIBRARIES += $(BFC_CH_KERB_LIBS_DIR)/parseLibEcos.a 
    endif
endif  
endif



ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

    BFC_$(BFC_TARGET_OS)_CABLEHOME_LIBRARIES += \
        $(OPENSSL_DIR)/libOpenSSLtlsecos.a 

endif           


##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).




##############################################################################
#
# SNMP Support 

ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_CABLHOME_SNMP_OBJECTS)
    
    # These are included only in fat images.
    ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)

        BFC_OBJECTS += $(BFC_CABLEHOME_SNMP_CONSOLE_OBJECTS)
    
    endif
    
    # Add factory objects if we have factory support
    ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CABLEHOME_FACTORY_OBJECTS)
    
    endif
    
    # Add runtime objects if we have BRCM runtime MIB support
    ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CABLEHOME_BRCM_SNMP_OBJECTS)
    
    endif

endif 






BFC_OBJECTS += $(BFC_CABLEHOME_OPEN_OBJECTS) 

# Add CableHome console components, if support for Console are both included.
ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

    # These are included only in fat images.
    ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)

        BFC_OBJECTS += $(BFC_CABLEHOME_CONSOLE_OBJECTS)
    
    endif

endif

BFC_COMPILER_DEFINES += $(BFC_CABLEHOME_DEFINES)

BFC_INCLUDE_DIRS += $(BFC_CABLEHOME_INCLUDE_DIRS)

BFC_SOURCE_DIRS += $(BFC_CABLEHOME_SOURCE_DIRS)

BFC_LIBRARIES += $(BFC_CABLEHOME_LIBRARIES) $(BFC_$(BFC_TARGET_OS)_CABLEHOME_LIBRARIES)





##############################################################################
#
# Includes - include other makefiles here (if necessary).  You must specify
#	     the full path to the makefile, using either BFC_ROOT_DIR or
#	     BFC_MAKE_DIR as appropriate.



############################################################################
## CableHome 1.1 Support
############################################################################

include $(BFC_MAKE_DIR)/BfcCableHome11.mak


############################################################################
## CableHome EPS/PS Support
############################################################################

ifeq ($(BFC_CABLEHOME_STANALONE_PS),1)
    include $(BFC_MAKE_DIR)/BfcCableHomeStandalonePs.mak
else
    include $(BFC_MAKE_DIR)/BfcCableHomeEmbeddedPs.mak
endif


############################################################################
## CableHome Customer Extension Support
############################################################################
ifeq ($(BFC_INCLUDE_CH_VENDOR_SUPPORT), 1)

    include $(BFC_MAKE_DIR)/$(ChVendorExtensionMakefile)
    
endif




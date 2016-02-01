##############################################################################
# 
#  Copyright (c) 2003-2009 Broadcom Corporation
# 
#  This program is the proprietary software of Broadcom Corporation and/or
#  its licensors, and may only be used, duplicated, modified or distributed
#  pursuant to the terms and conditions of a separate, written license
#  agreement executed between you and Broadcom (an "Authorized License").
#  Except as set forth in an Authorized License, Broadcom grants no license
#  (express or implied), right to use, or waiver of any kind with respect to
#  the Software, and Broadcom expressly reserves all rights in and to the
#  Software and all intellectual property rights therein.  IF YOU HAVE NO
#  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#  SOFTWARE.  
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
# 
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
# 
##############################################################################
#  $Id$
#
#  Filename:       BfcCm.mak
#  Author:         Kevin O'Neal
#  Creation Date:  October 8, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for DOCSIS Cable Modem.
#
##############
#
# External Variables (input/output):
#
#	BFC_OBJECTS - the list of object files that need to be built for the
#		      project based on the current configuration.
#
#	BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#			       to the compiler.
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
# Includes - include other makefiles here (if necessary).  You must specify
#	     the full path to the makefile, using either BFC_ROOT_DIR or
#	     BFC_MAKE_DIR as appropriate.

# CM Vendor Extension support.  We need to include their makefile so they can
# their components.
ifeq ($(BFC_INCLUDE_CM_VENDOR_SUPPORT), 1)

    include $(BFC_VENDOR_MAKE_DIR)/$(CmVendorExtensionMakefile)
    
endif

# PacketCable support.  If we have voice support, include EMTA makefile.
ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)

    include $(BFC_MAKE_DIR)/BfcMta.mak

endif

# OpenCable support.  If we have STB support, include STB makefile.
ifeq ($(BFC_INCLUDE_ESTB_SUPPORT), 1)

    include $(BFC_MAKE_DIR)/BfcEstb.mak

endif

# CM mini Firewall support
ifeq ($(BFC_CM_FIREWALL_INCLUDED), 1)

    include $(BFC_MAKE_DIR)/BfcMiniFirewall.mak

endif



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

BFC_CM_DIR = Cm

BFC_CM_NETWORKING_DIR = Bfc/Networking/DocsisCm

BFC_CM_DOCSIS_DIR = Docsis

BFC_CM_BPI_DIR = $(BFC_CM_DOCSIS_DIR)/BaselinePrivacy
   

# Set the OpenSSL paths
BFC_CM_OPENSSL_DIR = Bfc/LibSupport/OpenSSL/include/openssl Bfc/LibSupport/OpenSSL/include
BFC_CM_OPENSSL_LIB_DIR = Bfc/LibSupport/OpenSSL

# Change OpenSSL paths if using the latest OpenSSL and if we are eCos.	
ifeq ($(BFC_TARGET_OS), Ecos)      
ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
	BFC_CM_OPENSSL_DIR = Bfc/LibSupport/OpenSSL098/include/openssl Bfc/LibSupport/OpenSSL098/include
	BFC_CM_OPENSSL_LIB_DIR = Bfc/LibSupport/OpenSSL098
endif
endif
       
BFC_CM_DSX_DIR = $(BFC_CM_DOCSIS_DIR)/DsxTransactions

BFC_CM_OSS_DIR = $(BFC_CM_DOCSIS_DIR)/Oss

BFC_CM_STUB_DIR = $(BFC_CM_DIR)/Stubs

BFC_CM_PROPANE_DIR = $(BFC_CM_DIR)/Propane Bfc/LibSupport/Propane Bfc/IpHelpers/BcmCommunicationProtocol

BFC_CM_BSP_COMMON_DIR = cm_bsp_v2/bsp_common
BFC_CmHal = $(buildProcessor)
ifeq ($(buildProcessor), 3350)
    BFC_CmHal = 335x
endif
ifeq ($(buildProcessor), 3255)
    BFC_CmHal = 3368
endif
ifeq ($(buildProcessor), 3378)
    BFC_CmHal = 3378
endif
ifeq ($(buildProcessor), 3379)
    BFC_CmHal = 3378
endif
ifeq ($(buildProcessor), 3361)
    BFC_CmHal = 3368
endif
ifeq ($(buildProcessor), 7118)
    BFC_CmHal = 3368
endif


BFC_CM_DRIVERS_DIR = $(BFC_CM_BSP_COMMON_DIR)/CmHal $(BFC_CM_BSP_COMMON_DIR)/CmHal/bcm$(BFC_CmHal)

BFC_CM_VENDOR_EXTENSION_DIR = $(BFC_CM_DIR)/CmExtensionApi

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

BFC_CM_OBJECTS = CmDownstreamScanThread.o   CmUsRateshapingThread.o \
                 DocsisCmDownstreamCalibrationNonVolSettings.o \
                 DocsisCmHalIf.o            DocsisCmSubInterfaceCounters.o \
                 DocsisCmUpstreamCalibrationNonVolSettings.o \
                 CmChanTargeting.o          CmDocsisBatteryACT.o \
		 CmDocsisCtlThread.o        CmDocsisDsScanEventACT.o \
                 CmDocsisFactory.o          CmDocsisIpInitClientACT.o   CmDocsisIpThread.o \
                 CmDocsisNonVolSettings.o   CmDocsis30NonVolSettings.o  CmDocsisRestartErrorCodes.o \
                 CmDocsisStatusEventCodes.o CmDsxHelper.o \
                 CmUsPhyOverheadSettings.o  CmUsTarget.o CmUsTargetMset.o \
		 DocsisTimeOfDayThread.o \
                 IpHelperDhcpClientEventACT.o \
		 CmDsChan.o CmDsPrimaryChan.o CmMultiDsHelper.o CmHwDetDsPhySettings.o \
		 IpHelperDhcpV6ClientEventACT.o BcmMaskTable.o BcmMulticastTable.o \
		 MulticastTableEntry.o

# PR13544
ifeq ($(BFC_INCLUDE_IPV6_SUPPORT), 1)
BFC_CM_OBJECTS += CmDhcpV6MsgACT.o
endif

BFC_CM_BONDING_OBJECTS  =  Cm3dfEventCensor.o Cm3dfDocsisMsgACT.o Cm3dfMsgTx.o \
		           Cm3dfMsgTxTimerMuxACT.o DsServiceGroupList.o DsServiceGroup.o
			   

ifeq ($(BFC_DS_BONDING), 1)
BFC_CM_OBJECTS += $(BFC_CM_BONDING_OBJECTS)
endif

ifeq ($(BCM93380_DS_BONDING), 1)
    # For the BCM93380 platform, need to include the additional TFTP server
    # file that allows us to serve the s/w image.
    BFC_CM_OBJECTS += SwImageTftpFile.o
endif

# For the 3255 with DS Bonding support, add the s/w DS Bonding module.
ifeq ($(buildProcessor), 3255)
    ifeq ($(BFC_DS_BONDING), 1)
        BFC_CM_OBJECTS += DsBonding.o
    endif
endif

# For the 7118 with DS Bonding support, add the s/w DS Bonding module.
ifeq ($(buildProcessor), 7118)
    ifeq ($(BFC_DS_BONDING), 1)
        BFC_CM_OBJECTS += DsBonding.o
    endif
endif


BFC_CM_BPI_OBJECTS = BpiCmDocsisStatusClientACT.o \
                     CmBpiAuthFsm.o CmBpiManagerThd.o \
                     CmBpiSaMapFsm.o CmBpiTekFsm.o CmSecureDownload.o

ifneq ($(BFC_TARGET_OS), Win32)

    BFC_CM_BPI_OBJECTS += OpenSSLCryptoApi.o

endif

ifneq ($(BFC_TARGET_OS), Win32)

    # Protect Win32 from IGMP stuff for now; it isn't clear whether or not it
    # will work properly in a simulation environment, and I don't have time to
    # try it out right now...
    BFC_CM_IGMP_OBJECTS = IgmpCmDocsisStatusClientACT.o IgmpGroup.o IgmpThread.o \
                          DocsisMcastIgmpSnoop.o

endif

BFC_CM_DSX_OBJECTS = AdmittedResourceRefreshTimerMuxACT.o \
                     AutoSuiteDsxCompletionHandlerACT.o \
                     DsaLocalTransaction.o      DsaRemoteTransaction.o \
                     DscLocalTransaction.o      DscRemoteTransaction.o \
                     DsdLocalTransaction.o      DsdRemoteTransaction.o \
                     DsxCompletionHandlerACT.o  DsxTransaction.o \
                     DsxTransactionPtrList.o    DsxTransactionTimerMuxACT.o \
                     DynamicServiceFlow.o       DynamicServiceTest.o \
                     FlowDiagramInputEvent.o
                     
BFC_CM_FACTORY_OBJECTS = CmFactoryEventLog.o    CmFactorySnmpAgent.o \
                         CpeFactorySnmpAgent.o \
                         brcmCmFactoryBridge.o  brcmCmFactoryMib.o \
                         brcmCmEngrBridge.o     brcmCmEngrMib.o \
                         CmCabledataFactoryBridge.o
                     
BFC_CM_OSS_OBJECTS = CmEventLog.o   CmEventLogNonVolSettings.o \
                     CmSnmpAgent.o  CmSnmpNonVolSettings.o CpeSnmpAgent.o \
                     CmCfgVarBindEngine.o \
                     CmSnmpAgentCmDocsisStatusACT.o
                     
BFC_CM_SNMP_OBJECTS = docsBpiBridge.o           docsBpiMib.o \
                      cmDocsBpi2DraftBridge.o   cmDocsBpi2DraftMib.o \
                      cmDocsIfBridge.o          cmDocsIfMib.o \
                      igmpBridge.o              igmpMib.o \
                      qosBridge.o               qosMib.o \
                      docsIfExt2Bridge.o        docsIfExt2Mib.o \
                      cmDocsCableDeviceBridge.o \
                      cmIfBridge.o
                      
BFC_CM_UDC_SNMP_OBJECTS = cmDocsQos3Mib.o           cmDocsQos3Bridge.o
                      
BFC_CM_MULPI_SNMP_OBJECTS = cmDocsIetfBpi2Bridge.o    cmDocsIetfBpi2Mib.o \
                            cmDocsIf3Bridge.o         cmDocsIf3Mib.o \
                            hostResourcesMib.o hostResourcesBridge.o

BFC_CM_BRCM_SNMP_OBJECTS = brcmCmMgmtMib.o brcmCmMgmtBridge.o CpePortFilterSnoop.o

BFC_CM_TELNET_SNMP_OBJECTS = cmTelnetMib.o

BFC_CM_SSH_SNMP_OBJECTS = cmSshMgmtMib.o
                   
# Optional console support files; this is present in a slim load.
BFC_CM_CONSOLE_OBJECTS = CmDocsisCommandTable.o CmRemoteAccessCommandTable.o

# More optional console support files; these are not present in a slim load.
BFC_FAT_CM_CONSOLE_OBJECTS += DocsisCmDownstreamCalibrationNonVolSettingsCommandTable.o \
                         DocsisCmUpstreamCalibrationNonVolSettingsCommandTable.o \
                         CmDocsisNonVolSettingsCommandTable.o \
                         CmDocsis30NonVolSettingsCommandTable.o \
                         DynamicServiceTestCommandTable.o \
                         CmSnmpNonVolSettingsCommandTable.o \
                         CmDocsisHalCommandTable.o CmSnmpAgentConsole.o

# Propane console support files.
BFC_FAT_CM_CONSOLE_OBJECTS += \
                         CmPropaneCommandTable.o \
                         CmPropaneNonVolSettingsCommandTable.o

# Propane support files
BFC_CM_PROPANE_OBJECTS = CmPropaneCtlThread.o           CmPropaneNonVolSettings.o \
                         PropaneCmDocsisStatusACT.o     PropaneCmDsxCompletionHandlerACT.o \
                         PropaneCommunicationProtocol.o PropanePhs.o \
                         PropanePhsType43Settings.o     BcmCommunicationProtocol.o \
                         PropaneSysInterface.o

# Battery support files                         
BFC_CM_BATTERY_OBJECTS = 

# CM Vendor Extension support.
BFC_CM_VENDOR_EXTENSION_OBJECTS = VendorCmApplicationBase.o

# OS-specific CM support.
#
# UNFINISHED - clean this up.
BFC_Embedded_CM_OBJECTS = \
        BCM3415.o \
        BCM3422.o \
        BcmMac.o \
        BcmMacApi.o \
        CAM.o \
        cmhal.o \
        downstrm.o \
        HwPhsDriver.o \
        info.o \
        interrup.o \
        mode.o \
        rangeadj.o \
        tuner.o \
        upstream.o \
        UsPower.o \
        CmPowerSaveDiag.o \
        DownstreamBcm$(BFC_CmHal).o \
        UpstreamBcm$(BFC_CmHal).o	\
        CmHalConfigureUpstream.o \
        UsCalTableApi.o \
        $(BFC_CM_PROPANE_OBJECTS)

ifeq ($(buildProcessor), 3348)
    BFC_Embedded_CM_OBJECTS += Bcm3348Mib.o
endif

ifeq ($(buildProcessor), 3368)
    BFC_Embedded_CM_OBJECTS += Bcm3368Mib.o
endif

ifeq ($(buildProcessor), 3378)
    BFC_Embedded_CM_OBJECTS += Bcm3378Mib.o BcmInternalTuner.o
endif

ifeq ($(buildProcessor), 3379)
    BFC_Embedded_CM_OBJECTS += Bcm3378Mib.o BcmInternalTuner.o
endif

ifeq ($(buildProcessor), 3361)
    BFC_Embedded_CM_OBJECTS += Bcm3368Mib.o
endif

ifeq ($(buildProcessor), 3255)
    BFC_Embedded_CM_OBJECTS += Bcm3368Mib.o
endif

ifeq ($(buildProcessor), 7118)
    BFC_Embedded_CM_OBJECTS += Bcm3368Mib.o
endif

BFC_VxWorks_CM_OBJECTS = $(BFC_Embedded_CM_OBJECTS)

BFC_Win32_CM_OBJECTS =

BFC_Ecos_CM_OBJECTS = $(BFC_Embedded_CM_OBJECTS)

BFC_Qnx6_CM_OBJECTS = $(BFC_Embedded_CM_OBJECTS)


# These objects are temporary, only here to serve as stubs until the real
# objects are in place.  For BFC interim development.
BFC_CM_STUB_OBJECTS = BfcCmStubs.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_CM_DEFINES = \
        BFC_INCLUDE_CM_VENDOR_SUPPORT=$(BFC_INCLUDE_CM_VENDOR_SUPPORT)	\
        EMBEDDED_CM_SUPPORT=$(EMBEDDED_CM_SUPPORT) \
        BFC_INCLUDE_CM_SUPPORT=$(BFC_INCLUDE_CM_SUPPORT) \
        BFC_INCLUDE_CM_CONSOLE_SUPPORT=$(BFC_INCLUDE_CM_CONSOLE_SUPPORT) \
        BCM93380_DS_BONDING=$(BCM93380_DS_BONDING) \
        BFC_L2VPN_SUPPORT=$(BFC_L2VPN_SUPPORT) \
        BFC_MULTICAST30_SUPPORT=$(BFC_MULTICAST30_SUPPORT) \
        BFC_MULPI_SUPPORT=$(BFC_MULPI_SUPPORT) \
        BFC_DS_BONDING=$(BFC_DS_BONDING) \
        BFC_US_BONDING=$(BFC_US_BONDING) \
        BCM_NO_CMTS_D3_PARTIAL_SVC=$(BCM_NO_CMTS_D3_PARTIAL_SVC) \
        BFC_INCLUDE_CPE_PORT_FILTER_SUPPORT=$(BFC_INCLUDE_CPE_PORT_FILTER_SUPPORT) \
        BCM_RESET_EVENT_LOG_SUPPORT=1

# if MULPI is enabled, IPv6 is required
ifeq ($(BFC_MULPI_SUPPORT), 1)
    BFC_INCLUDE_IPV6_SUPPORT = 1
endif

# if IPv6 is enabled, this requires D3.0 style multicast support and UDC support
ifeq ($(BFC_INCLUDE_IPV6_SUPPORT), 1)
    BFC_MULTICAST30_SUPPORT = 1
    BFC_INCLUDE_CM_UDC_SUPPORT = 1
endif

BFC_CM_DEFINES += BFC_INCLUDE_CM_UDC_SUPPORT=$(BFC_INCLUDE_CM_UDC_SUPPORT)
        
ifeq ($(BFC_CM_FIREWALL_INCLUDED), 1)
    BFC_CM_DEFINES += BFC_CM_FIREWALL_INCLUDED=$(BFC_CM_FIREWALL_INCLUDED)
endif

# openSSL 0.9.8 support
ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
    BFC_CM_DEFINES += BFC_INCLUDE_OPENSSL098
endif

# openSSH support
ifeq ($(BFC_INCLUDE_OPENSSH_SUPPORT), 1)
    include $(BFC_MAKE_DIR)/BfcOpenSSH.mak
endif

ifneq ($(BFC_TARGET_OS), Win32)

    BFC_CM_DEFINES += \
        SOFTWARE_CRYPTO_LIB=1 \
        OPENSSL=1 \
        GLOBAL_FUNCTION_POINTERS=1 \
        SOFTWARE_CRYPTO_LIB_INCLUDED=1 \
        INCLUDE_DOCSIS_IGMP=1 \
        INCLUDE_SECURE_DOWNLOAD=1

endif


# If "OMS" support was specified, make sure the proper define is included.  Note
# that this is used in a #ifdef, so we can't define it at all if "OMS" is not
# included.
ifeq ($(BCM_VENDOR_OMS), 1)
    BFC_CM_DEFINES += BCM_VENDOR_OMS=1
endif

# If 'newleds' was specified, define the appropriate build variable; otherwise,
# don't define it at all.
ifeq ($(BCM_USE_NEW_VISUALIZATION_LEDS), 1)
    BFC_CM_DEFINES += BCM_USE_NEW_VISUALIZATION_LEDS=$(BCM_USE_NEW_VISUALIZATION_LEDS)
endif

# If DS bonding is specified for a non-bonded device, expect the build to blow up.
ifeq ($(BFC_DS_BONDING),1)
    BFC_CM_DEFINES += DOCSIS_CORE_BCM_DS_BONDED=1
endif

# Set the DOCSIS_CORE_BCMxxxx variable based on the processor type; unfortunately
# there isn't a direct mapping, so we have to do this the hard way.
ifeq ($(buildProcessor), 3348)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM3348
endif

ifeq ($(buildProcessor), 3368)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM3368
endif

ifeq ($(buildProcessor), 3361)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM3368
endif

ifeq ($(buildProcessor), 3378)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM3368
    BFC_CM_DEFINES += DOCSIS_CORE_BCM3378=1
endif

ifeq ($(buildProcessor), 3379)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM3368
    BFC_CM_DEFINES += DOCSIS_CORE_BCM3378=1
endif

# If the build processor is 3255 or 7118...
ifeq ($(filter 3255 7118,$(buildProcessor)), $(buildProcessor))
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM3368
    ifeq ($(BFC_DS_BONDING),1)
        BFC_CM_DEFINES += DOCSIS_CORE_BCM_DS_BONDED=1
    else	
        ifeq ($(BCM_SINGLE_DOWNSTREAM),1)
            BFC_CM_DEFINES += DOCSIS_CORE_BCM_SINGLE_DS_MAC_MULT_DS_PHY=1
        endif
    endif
endif

ifeq ($(buildProcessor), 3360)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM336x
endif

ifeq ($(buildProcessor), 3345)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM334x
endif

ifeq ($(buildProcessor), 3350)
    BFC_CM_DOCSIS_CORE = DOCSIS_CORE_BCM335x
endif

ifeq ($(BCM_NUM_SIDS),)
    BCM_NUM_SIDS = 4
endif

# OS-specific CM support.
BFC_Embedded_CM_DEFINES = \
        $(BFC_CM_DOCSIS_CORE)=1 \
        CM_HAL_INCLUDED=1 \
        ACKCEL_SUPPORT_INCLUDED=1 \
        PROPANE_SUPPORT=1 \
        BCM_NUM_SIDS=$(BCM_NUM_SIDS)

BFC_VxWorks_CM_DEFINES = $(BFC_Embedded_CM_DEFINES)

BFC_Win32_CM_DEFINES = 

BFC_Ecos_CM_DEFINES = $(BFC_Embedded_CM_DEFINES)

BFC_Qnx6_CM_DEFINES = $(BFC_Embedded_CM_DEFINES)

# If prereg_sets is turned on, then the define must be set up, otherwise
# don't define it at all.
ifeq ($(BCM_ALLOW_PREREG_SETS), 1)
    BFC_CM_DEFINES += BCM_ALLOW_PREREG_SETS=$(BCM_ALLOW_PREREG_SETS)
endif


##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_CM_INCLUDE_DIRS = \
        $(BFC_CM_DIR) \
        $(BFC_CM_NETWORKING_DIR) \
        $(BFC_CM_DOCSIS_DIR) \
        $(BFC_CM_BPI_DIR) \
        $(BFC_CM_DSX_DIR) \
        $(BFC_CM_OSS_DIR) \
        $(BFC_CM_OPENSSL_DIR) 

ifeq ($(BFC_INCLUDE_CM_STUB_SUPPORT), 1)

    BFC_CM_INCLUDE_DIRS += $(BFC_CM_STUB_DIR)

endif

ifeq ($(BFC_INCLUDE_CM_VENDOR_SUPPORT), 1)

    BFC_CM_INCLUDE_DIRS += $(BFC_CM_VENDOR_EXTENSION_DIR)
    
endif

# OS-specific CM support.
BFC_VxWorks_CM_INCLUDE_DIRS = $(BFC_CM_DRIVERS_DIR) $(BFC_CM_PROPANE_DIR)

BFC_Win32_CM_INCLUDE_DIRS = 

BFC_Ecos_CM_INCLUDE_DIRS = $(BFC_CM_DRIVERS_DIR) $(BFC_CM_PROPANE_DIR)

BFC_Qnx6_CM_INCLUDE_DIRS = $(BFC_CM_DRIVERS_DIR) $(BFC_CM_PROPANE_DIR)


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_CM_SOURCE_DIRS = $(BFC_CM_DIR) $(BFC_CM_NETWORKING_DIR) $(BFC_CM_DOCSIS_DIR) \
                     $(BFC_CM_BPI_DIR) $(BFC_CM_DSX_DIR) $(BFC_CM_OSS_DIR)

ifeq ($(BFC_INCLUDE_CM_STUB_SUPPORT), 1)

        BFC_CM_SOURCE_DIRS += $(BFC_CM_STUB_DIR)

endif

ifeq ($(BFC_INCLUDE_CM_VENDOR_SUPPORT), 1)

    BFC_CM_SOURCE_DIRS += $(BFC_CM_VENDOR_EXTENSION_DIR)
    
endif

# OS-specific CM support.
BFC_VxWorks_CM_SOURCE_DIRS = \
        $(BFC_CM_DRIVERS_DIR) \
        $(BFC_CM_PROPANE_DIR)

BFC_Win32_CM_SOURCE_DIRS = 

BFC_Ecos_CM_SOURCE_DIRS = \
        $(BFC_CM_DRIVERS_DIR) \
        $(BFC_CM_PROPANE_DIR)

BFC_Qnx6_CM_SOURCE_DIRS = \
        $(BFC_CM_DRIVERS_DIR) \
        $(BFC_CM_PROPANE_DIR)


##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_CM_LIBRARIES = 

ifeq ($(BFC_TARGET_OS), VxWorks)

    BFC_CM_ACKCEL_LIBRARY = Bfc/LibSupport/AckCel/AckCel.vxWorks.MIPS32.t20.a

    BFC_CM_LIBRARIES += Bfc/LibSupport/OpenSSL/libOpenSSLvxworks.a \
                        Bfc/LibSupport/OpenSSL/SecLibVxworks.a \
			$(BFC_CM_ACKCEL_LIBRARY) \
			Bfc/LibSupport/Propane/vxWorks/Propane05VxWorks.a

endif

ifeq ($(BFC_TARGET_OS), Ecos)

    BFC_CM_ACKCEL_LIBRARY = Bfc/LibSupport/AckCel/AckCel.eCos.a

        
    BFC_CM_LIBRARIES += $(BFC_CM_OPENSSL_LIB_DIR)/libOpenSSLecos.a \
                        $(BFC_CM_ACKCEL_LIBRARY) \
                        Bfc/LibSupport/Propane/eCos/Propane05eCos.a

    
endif

ifeq ($(BFC_TARGET_OS), Qnx6)

    BFC_CM_ACKCEL_LIBRARY = Bfc/LibSupport/AckCel/AckCel.qnx.a

    BFC_CM_LIBRARIES += Bfc/LibSupport/OpenSSL/libOpenSSLqnx.a \
                        Bfc/LibSupport/OpenSSL/SecLibqnx.a \
                        $(BFC_CM_ACKCEL_LIBRARY) \
                        Bfc/LibSupport/Propane/qnx/Propane05qnx.a

endif


# If the source for the AckCel library doesn't exist, then the only options is
# to extract the .o file from the library.  If the source does exist, then we
# can build the .o file and library.
ifeq ($(wildcard $(BFC_ROOT_DIR)/$(BFC_CM_BSP_COMMON_DIR)/CmHal/AckCelCmUpstream.c),)

# Special target to extract the CM AckCel object file from the library.  This
# is used for cache optimization (if enabled).  During the second link phase
# (after the list of object modules has been properly ordered), this file
# becomes explicitly listed as a link target.  As a result, the linker won't
# look for it in the library file, so we must extract it ourselves.
AckCelCmUpstream.o : $(BFC_ROOT_DIR)/$(BFC_CM_ACKCEL_LIBRARY)
	@ echo ==== Extracting AckCelCmUpstream.o...
	@ $(AR) x $(BFC_ROOT_DIR)/$(BFC_CM_ACKCEL_LIBRARY) AckCelCmUpstream.o

else

# This is actually a build target for the AckCel library, for the rare occasions
# when we need to rebuild it after modifying the AckCel source code.
AckCelLib : AckCelCmUpstream.o
	@ echo ==== Updating the AckCel library $(BFC_CM_ACKCEL_LIBRARY)...
	@ $(AR) cruv $(BFC_ROOT_DIR)/$(BFC_CM_ACKCEL_LIBRARY) AckCelCmUpstream.o
	@ $(RM) AckCelCmUpstream.o

endif


##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add Docsis CM components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_CM_SUPPORT), 1)

    # Core CM objects
    BFC_OBJECTS += $(BFC_CM_OBJECTS) $(BFC_$(BFC_TARGET_OS)_CM_OBJECTS)
    
    # Add stub objects if required
    ifeq ($(BFC_INCLUDE_CM_STUB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_STUB_OBJECTS)
    
    endif
    
    # Add CM Vendor Extension objects if needed.
    ifeq ($(BFC_INCLUDE_CM_VENDOR_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_VENDOR_EXTENSION_OBJECTS)
	
    endif
    
    # BPI, DSX, and OSS (for the time being, at least) are unconditionally 
    # included
    BFC_OBJECTS += $(BFC_CM_BPI_OBJECTS)
    
    BFC_OBJECTS += $(BFC_CM_IGMP_OBJECTS)

    BFC_OBJECTS += $(BFC_CM_DSX_OBJECTS)
    
    BFC_OBJECTS += $(BFC_CM_OSS_OBJECTS)
    
    # Add SNMP objects if we have SNMP support.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
    
                # if we have telnet support, add the special telnet MIB for CM
                ifeq ($(BFC_INCLUDE_TELNET_CONSOLE_SUPPORT), 1)
                        
                        BFC_CM_SNMP_OBJECTS += $(BFC_CM_TELNET_SNMP_OBJECTS)
                        
                endif
                
                # if we have SSH support, add the special ssh MIB for CM
                ifeq ($(BFC_INCLUDE_OPENSSH_SUPPORT), 1)
                        
                        BFC_CM_SNMP_OBJECTS += $(BFC_CM_SSH_SNMP_OBJECTS)
                        
                endif
        
        endif
        
    endif
        
    BFC_OBJECTS += $(BFC_CM_SNMP_OBJECTS)
    
    # Add CM console components, if support for CM and CM console 
    # are both included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_INCLUDE_CM_CONSOLE_SUPPORT), 1)

            # These are included in fat and slim images.
            BFC_OBJECTS += $(BFC_CM_CONSOLE_OBJECTS)

            # These are included only in fat images.
            ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)

                BFC_OBJECTS += $(BFC_FAT_CM_CONSOLE_OBJECTS)
	    
	    endif
	    
        endif

    endif
    
    # Add MULPI objects if we have MULPI support
    ifeq ($(BFC_MULPI_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_MULPI_SNMP_OBJECTS)
        
        # if we have MULPI support then we need the UDC objects
        # regardless if we have UDC support or not.  This is a bit inelegant
        # but works for now since the UDC objects are only the docsIetfQosMib
        # files anyway.  Might revisit this in the future if necessary.
        BFC_OBJECTS += $(BFC_CM_UDC_SNMP_OBJECTS)
    
    endif
    
    # Add UDC objects if we have UDC support
    ifeq ($(BFC_INCLUDE_CM_UDC_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_UDC_SNMP_OBJECTS)
    
    endif
    
    # Add factory objects if we have factory support
    ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_FACTORY_OBJECTS)
    
    endif
    
    # Add runtime objects if we have BRCM runtime MIB support
    ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_BRCM_SNMP_OBJECTS)
    
    endif
    
    ifeq ($(BFC_INCLUDE_BATTERY_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_CM_BATTERY_OBJECTS)

    endif
    
    ifeq ($(BFC_L2VPN_SUPPORT), 1)
        BFC_OBJECTS += $(BFC_L2VPN_OBJECTS)
    endif 
   
    BFC_COMPILER_DEFINES += $(BFC_CM_DEFINES) $(BFC_$(BFC_TARGET_OS)_CM_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_CM_INCLUDE_DIRS) $(BFC_$(BFC_TARGET_OS)_CM_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_CM_SOURCE_DIRS) $(BFC_$(BFC_TARGET_OS)_CM_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_CM_LIBRARIES)

endif


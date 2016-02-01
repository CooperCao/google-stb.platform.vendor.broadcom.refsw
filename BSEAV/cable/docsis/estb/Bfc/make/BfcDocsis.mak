##############################################################################
#
#  Copyright (c) 2003-2007  Broadcom Corporation
#  All Rights Reserved
#  No portions of this material may be reproduced in any form without the
#  written permission of:
#          Broadcom Corporation
#          16215 Alton Parkway
#          Irvine, California 92618
#  All information contained in this document is Broadcom Corporation
#  company private, proprietary, and trade secret.
#
##############################################################################
#  $Id$
#
#  Filename:       BfcDocsis.mak
#  Author:         Kevin O'Neal
#  Creation Date:  October 8, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for DOCSIS.
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

BFC_DOCSIS_DIR = Docsis

BFC_DOCSIS_BPI_DIR = $(BFC_DOCSIS_DIR)/BaselinePrivacy

BFC_DOCSIS_MACMGMT_DIR = $(BFC_DOCSIS_DIR)/MacManagementMessages

BFC_DOCSIS_STUB_DIR = $(BFC_DOCSIS_DIR)/Stubs

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.


BFC_DOCSIS_OBJECTS = BurstDescriptorSet.o Classifier.o ClassifierSetsList.o \
                     ClassifierSettings.o ClassOfService.o ClientMacAddressEncoding.o \
                     CmConfig.o \
                     CmDsChanConfig.o CmDsChanProfile.o CmDsRxModuleProfile.o \
                     Cmim.o CmIpInitSettings.o \
                     CosErrRsp.o \
                     CosReqSettings.o CosRspSettings.o BpiCmIdSettings.o \
                     DccCmJumpTimeSettings.o DccDownstreamSettings.o \
                     DccSubQosFlowSetsList.o DccSubQosFlowSettings.o \
		     DsActiveChanSettings.o DsChannelList.o DsChannelListEntry.o \
                     DutSettings.o ErrorSet.o ErrorSetList.o \
                     LinkedQosFlowSetsList.o \
                     LinkedQosFlowSettings.o LinkedQosFlowSettings2.o \
                     L2VpnSettings.o \
                     MacDomainSettings.o  ModemCapabilities.o \
                     Phs.o PhsSetsList.o PhsSettings.o \
                     QosDynamicServiceStats.o QosErrorEncoding.o \
                     QosErrorEncodingList.o QosFactory.o \
                     QosFlowSetsList.o QosFlowSettings.o \
                     ScdmaTimestampSnapshot.o ServiceFlow.o \
                     ServiceFlowEventACT.o ServiceFlowEventPublisher.o \
                     Snmp3KickstartValue.o Snmp3NotifyReceiver.o StaticMcast.o TLVCodes.o \
                     Type43VendorSettings.o UpstreamSettings.o \
                     V2BurstDescriptor.o V2MapInfoElement.o \
		     Dsid.o CmDsidMcastSettings.o CmDsidCommand.o SecurityAssociation.o \
		     DbcReqMsg.o DbcRspMsg.o DbcAckMsg.o CmDsidHelper.o CmDsidSettings.o


BFC_DOCSIS_BONDING_OBJECTS = CmControlRequestSettings.o CmControlResponseSettings.o CmCtrlReqMsg.o CmCtrlRspMsg.o \
	  	         CmStatusMsg.o CmStEvIdParams.o Cm3dfChSpEvRptEnableSettings.o CmStatusEventControlSettings.o \
		         CmMultiDsChanProfile.o CmMultiDsChanConfig.o CmRcpControlSettings.o \
		         CmDsidReseqSettings.o  CmRccErrorEncodings.o \
			 MdDsSvcGroupSettings.o 

ifeq ($(BFC_DS_BONDING), 1)
BFC_DOCSIS_OBJECTS += $(BFC_DOCSIS_BONDING_OBJECTS)
endif

BFC_DOCSIS_3_OBJECTS =   SnmpV1V2Coexistence.o SnmpV1V2Transport.o \
                         SnmpV3AccessView.o
                     
BFC_DOCSIS_BPI_OBJECTS = BpiAuthentInfoMsg.o BpiAuthInvalidMsg.o BpiAuthKeys.o BpiAuthRejectMsg.o \
                         BpiAuthReplyMsg.o BpiAuthRequestMsg.o BpiMsgCodes.o \
                         BpiSaDescriptor.o BpiSaMapRejectMsg.o \
                         BpiSaMapReplyMsg.o BpiSaMapRequestMsg.o \
                         BpiSecurityCapabilities.o BpiTekInvalidMsg.o \
                         BpiTekParameters.o BpiTekRejectMsg.o BpiTekReplyMsg.o \
                         BpiTekRequestMsg.o \
                         BpkmMessage.o CmBpkmSettings.o DigiCertificate.o \
                         ParseAsn1.o CmCodeSignerInfo.o \
                         CmCodeVerificationCertificate.o CmBpiNonVolSettings.o \
                         CmDownloadParseTlv.o

BFC_DOCSIS_MACMGMT_OBJECTS = DccAckMsg.o DccReqMsg.o DccRspMsg.o \
                             DsaAckMsg.o DsaReqMsg.o DsaRspMsg.o \
                             DscAckMsg.o DscReqMsg.o DscRspMsg.o \
                             DsdReqMsg.o DsdRspMsg.o \
                             DsxMsgBase.o \
                             MacManagementMessage.o \
                             MacMgtMsgHmacBase.o MapMsg.o MddMsg.o \
                             MacMgtMsgHmacMpBase.o \
                             RegAckMsg.o RegReqMsg.o RegRspMsg.o \
                             RegReqMsgMp.o RegRspMsgMp.o \
                             MacMgtMsgMultiPart.o \
                             RngReqMsg.o RngRspMsg.o \
                             TstReqMsg.o UCDMessage.o UccReqMsg.o UccRspMsg.o \
                             UpDisMsg.o
                              
ifeq ($(BCM_ANNEX_OTHER_NV_SUPPORT), 1)
BFC_COMPILER_DEFINES += BCM_ANNEX_OTHER_NV_SUPPORT=1
endif

# PR12948 - new object to handle some common things for REG-REQ messages.
BFC_DOCSIS_MACMGMT_OBJECTS += RegReqMsgHelper.o
                             

# Optional console support files.
BFC_DOCSIS_CONSOLE_OBJECTS = CmBpiNonVolSettingsCommandTable.o

# OS-specific DOCSIS support.
BFC_VxWorks_DOCSIS_OBJECTS = 

BFC_Win32_DOCSIS_OBJECTS =

BFC_Linux_DOCSIS_OBJECTS =

BFC_Ecos_DOCSIS_OBJECTS =

BFC_Qnx6_DOCSIS_OBJECTS =

# These objects are temporary, only here to serve as stubs until the real
# objects are in place.  For BFC interim development.
BFC_DOCSIS_STUB_OBJECTS = BfcDocsisStubs.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_DOCSIS_DEFINES = BFC_INCLUDE_DOCSIS_SUPPORT=$(BFC_INCLUDE_DOCSIS_SUPPORT) \
                     BFC_INCLUDE_DOCSIS_CONSOLE_SUPPORT=$(BFC_INCLUDE_DOCSIS_CONSOLE_SUPPORT) 

ifeq ($(BFC_INCLUDE_HPPC),1)
    BFC_DOCSIS_DEFINES += \
        BFC_INCLUDE_HPPC=1
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_DOCSIS_INCLUDE_DIRS = $(BFC_DOCSIS_DIR) $(BFC_DOCSIS_BPI_DIR) $(BFC_DOCSIS_MACMGMT_DIR)

ifeq ($(BFC_INCLUDE_DOCSIS_STUB_SUPPORT), 1)

        BFC_DOCSIS_INCLUDE_DIRS += $(BFC_DOCSIS_STUB_DIR)

endif

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_DOCSIS_SOURCE_DIRS = $(BFC_DOCSIS_DIR) $(BFC_DOCSIS_BPI_DIR) $(BFC_DOCSIS_MACMGMT_DIR)

ifeq ($(BFC_INCLUDE_DOCSIS_STUB_SUPPORT), 1)

        BFC_DOCSIS_SOURCE_DIRS += $(BFC_DOCSIS_STUB_DIR)

endif

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_DOCSIS_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add Docsis components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_DOCSIS_SUPPORT), 1)

    # Core DOCSIS objects
    BFC_OBJECTS += $(BFC_DOCSIS_OBJECTS) $(BFC_$(BFC_TARGET_OS)_DOCSIS_OBJECTS)
    
    BFC_OBJECTS += $(BFC_DOCSIS_BPI_OBJECTS)
    
    BFC_OBJECTS += $(BFC_DOCSIS_MACMGMT_OBJECTS)
    
    BFC_OBJECTS += $(BFC_DOCSIS_3_OBJECTS)

    # Add stub objects if required
    ifeq ($(BFC_INCLUDE_DOCSIS_STUB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_DOCSIS_STUB_OBJECTS)
    
    endif
    
    # Add DOCSIS console components, if support for Console and DOCSIS Console 
    # are both included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)
    
            ifeq ($(BFC_INCLUDE_DOCSIS_CONSOLE_SUPPORT), 1)

                BFC_OBJECTS += $(BFC_DOCSIS_CONSOLE_OBJECTS)

            endif

        endif
	
    endif

    BFC_COMPILER_DEFINES += $(BFC_DOCSIS_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_DOCSIS_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_DOCSIS_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_DOCSIS_LIBRARIES)

endif


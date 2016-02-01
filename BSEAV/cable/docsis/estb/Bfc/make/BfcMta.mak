#//****************************************************************************
#//
#// Copyright (c) 2004-2009 Broadcom Corporation
#//
#// This program is the proprietary software of Broadcom Corporation and/or
#// its licensors, and may only be used, duplicated, modified or distributed
#// pursuant to the terms and conditions of a separate, written license
#// agreement executed between you and Broadcom (an "Authorized License").
#// Except as set forth in an Authorized License, Broadcom grants no license
#// (express or implied), right to use, or waiver of any kind with respect to
#// the Software, and Broadcom expressly reserves all rights in and to the
#// Software and all intellectual property rights therein.  IF YOU HAVE NO
#// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#// SOFTWARE.  
#//
#// Except as expressly set forth in the Authorized License,
#//
#// constitutes the valuable trade secrets of Broadcom, and you shall use all
#// reasonable efforts to protect the confidentiality thereof, and to use this
#// information only in connection with your use of Broadcom integrated circuit
#// products.
#//
#// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#//
#// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#//
#//****************************************************************************
#
#  Filename:       BfcMta.mak
#  Author:         Sean Dunlap
#  Creation Date:  February 11, 2004
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the MTA subsystem, which
#	are common to all MTA platforms. 
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


# standalone MTA with RG features
ifeq ($(BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT),1)
include $(BFC_MAKE_DIR)/BfcEmta.mak
else
include $(BFC_MAKE_DIR)/BfcSmta.mak
endif

##############################################################################
#
# Paths - definitions of relative paths for this component.  These are used
#         below, and should be defined once, rather than using the actual
#         value in multiple places.
#
#         Note that the path needs to be specified relative to the "mount point"
#         of the parent project, not relative to the output directory.  The
#         make processing will specify the rest of the path for you.

BFC_MTA_DIR = Mta

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.

# Core MTA objects
BFC_MTA_OBJECTS = EmtaIf.o \
                  UDPStack.o \
                  EmtaNonVolSettings.o \
                  EmtaBlindDataNonVolSettings.o \
                  EmtaSecurityNonVolSettings.o \
                  MtaEventACT.o \
                  EmtaDhcpCompletionHandlerACT.o \
                  EmtaVoicePacketSnoop.o \
                  PacketSwitchBridgingHalIf.o  \
                  ArpFilterSnoop.o \
                  MtaMessageLogAPI.o \
                  MtaAnnouncementDownloadAPI.o \
                  MtaNonVolAPI.o \
                  AnnouncementDownloadThread.o \
                  EmtaFirewallSnoop.o \
                  MtaFirewallAPI.o \
                  MtaEventPublisher.o \
                  MtaEventPublisherAPI.o \
                  MtaSystemAPI.o \
                  MtaBatteryAPI.o \
                  EmtaDhcpStateMachineGlue.o \
                  MtaHalIfACT.o \
                  MtaIpSecAPI.o \
                  MtaTftpAPI.o \
                  MtaDhcpAPI.o \
                  MtaSecNonVolAPI.o \
                  MtaServiceFlowAPI.o \
                  MtaKeepAlivePingAgentAPI.o

ifeq ($(BFC_INCLUDE_IPV6_SUPPORT),1)
    BFC_MTA_OBJECTS +=  MtaDhcpV6API.o 
endif

# SNMP objects	
BFC_MTA_SNMP_OBJECTS = EmtaSnmpApi.o \
                    EmtaSnmpAgent.o \
                    EmtaIfBridge.o \
                    NetSnmpEmtaAgnt.o \
                    EmtaSnmpApiStubs.o \
                    EmtaCfgVarBindEngine.o \
                    EmtaSnmpNonVolSettings.o \
                    MtaSnmpAPI.o \
                    MtaSnmpNonVolAPI.o

BFC_MTA_FACTORY_OBJECTS = \
                    brcmEmtaFactoryBridge.o \
                    brcmEmtaFactoryMib.o \
                    emtaCabledataFactoryBridge.o \
                    EmtaFactorySnmpAgent.o
        
BFC_MTA_BRCM_SNMP_OBJECTS =
                     
BFC_EMTA_SNMP_CONSOLE_OBJECTS = EmtaSnmpAgentConsole.o

# Optional console MTA objects
BFC_MTA_CONSOLE_OBJECTS = EmtaCommandTable.o EmtaEndptCommandTable.o MtaCommandTableAPI.o EmtaConfigVoiceNonVolSettingsCommandTable.o \
                EmtaSecurityNonVolSettingsCommandTable.o EmtaNonVolSettingsCommandTable.o \
		EmtaSnmpNonVolSettingsCommandTable.o
		
# optional EMTA simulator objects (used for testing only)
BFC_MTA_CALL_SIM_OBJECTS = CallSimCommandTable.o EmtaCallSimulator.o 

# optional battery objects
BFC_MTA_BATTERY_OBJECTS = EmtaBatteryACT.o 

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_MTA_DEFINES += EMTA_SUPPORT=1 OS_ECOS=1

ifeq ($(BFC_INCLUDE_MTA_DYNAMIC_BOOTUP_ADDR_SUPPORT), 1)
   ifneq ($(DSP_BOOTUP_ADDR), )
      BFC_MTA_DEFINES += DSP_RAM_START_ADDR=$(DSP_BOOTUP_ADDR)
   endif
endif

ifeq ($(BFC_INCLUDE_EMTA_CONSOLE_SUPPORT), 1)
 BFC_MTA_DEFINES += BCM_EMTA_CONSOLE_SUPPORT=1
endif

ifeq ($(BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT),1)
	BFC_MTA_DEFINES += BCM_INCLUDE_MTA_CABLEMODEM_SUPPORT=1
else
	BFC_MTA_DEFINES += BCM_INCLUDE_MTA_CABLEMODEM_SUPPORT=0
endif

ifeq ($(BFC_INCLUDE_MTA_GATEWAY_SUPPORT),1)
	BFC_MTA_DEFINES += BCM_INCLUDE_MTA_GATEWAY_SUPPORT=1
else
	BFC_MTA_DEFINES += BCM_INCLUDE_MTA_GATEWAY_SUPPORT=0
endif

ifeq ($(BFC_INCLUDE_COMCAST_SUPPORT),1)
	BFC_MTA_DEFINES += COMCAST_SUPPORT=1
endif

ifeq ($(BFC_INCLUDE_TIME_WARNER_SUPPORT),1)
	BFC_MTA_DEFINES += TIMEWARNER_SUPPORT=1
endif

ifeq ($(BFC_INCLUDE_COX_SUPPORT),1)
	BFC_MTA_DEFINES += COX_SUPPORT=1
endif



##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_MTA_INCLUDE_DIRS = $(BFC_MTA_DIR) $(BFC_MTA_DIR)/system \
	                    $(BFC_MTA_DIR)/Snmp $(BFC_MTA_DIR)/Snmp/Stubs Snmp \
                     Bfc/IpHelpers Bfc/BfcApp \
                     Bfc/Networking/Ip $(BFC_MTA_DIR)/MtaApi \
				Bfc/IpHelpers/Arp Mta/Firewall


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_MTA_SOURCE_DIRS = $(BFC_MTA_DIR) $(BFC_MTA_DIR)/System \
		              $(BFC_MTA_DIR)/Snmp $(BFC_MTA_DIR)/Snmp/NetSnmp $(BFC_MTA_DIR)/Snmp/Stubs \
		              Snmp Docsis Bfc/IpHelpers Bfc/BfcApp $(BFC_MTA_DIR)/MtaApi \
					  Bfc/IpHelpers/Arp Mta/Firewall

ifeq ($(BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT),1)
	BFC_MTA_SOURCE_DIRS += CmDocsisSystem
endif

ifeq ($(BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT),0)
ifeq ($(BFC_INCLUDE_MTA_GATEWAY_SUPPORT),1)
	BFC_MTA_SOURCE_DIRS += SpsCableHomeSystem
endif
endif

ifeq ($(BFC_INCLUDE_MTA_CABLEMODEM_SUPPORT),0)
ifeq ($(BFC_INCLUDE_MTA_GATEWAY_SUPPORT),0)
	BFC_MTA_SOURCE_DIRS += SmtaSystem $(BFC_MTA_DIR)/RateShaping Smta
endif
endif


##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.


##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add MTA components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)

	# Add MTA components, if included by BfcSystemConfig.mak.
	BFC_OBJECTS += $(BFC_MTA_OBJECTS) $(BFC_MTA_CONSOLE_OBJECTS) $(BFC_MTA_CALL_SIM_OBJECTS)

	# Add Ping console components, if support for Console and EMTA Console
	# are both included.
	ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

		ifeq ($(BFC_INCLUDE_EMTA_CONSOLE_SUPPORT), 1)

			BFC_OBJECTS += $(BFC_EMTA_CONSOLE_OBJECTS)

			# we only need the call sim if we have EMTA console support
			ifeq ($(BFC_INCLUDE_EMTA_SIM_SUPPORT), 1)
				BFC_OBJECTS += $(BFC_MTA_CALL_SIM_OBJECTS)
			endif

		endif

	endif

	# Add SNMP objects, if SNMP support is included.
	ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

		BFC_OBJECTS += $(BFC_MTA_SNMP_OBJECTS)

		# If we have SNMP support AND console support, then include
		# SNMP console objects.
		ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
		    
                    ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE),0)
	              
                      BFC_OBJECTS += $(BFC_EMTA_SNMP_CONSOLE_OBJECTS)
                    
                    endif
		
                endif
                
                # Add factory objects if we have factory support
                ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)
                
                    BFC_OBJECTS += $(BFC_MTA_FACTORY_OBJECTS)
                
                endif
                
                # Add runtime objects if we have BRCM runtime MIB support
                ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
                
                    BFC_OBJECTS += $(BFC_MTA_BRCM_SNMP_OBJECTS)
                
                endif

	endif
        
        ifeq ($(BFC_INCLUDE_BATTERY_SUPPORT), 1)
        
                BFC_OBJECTS += $(BFC_MTA_BATTERY_OBJECTS)

        endif

	BFC_COMPILER_DEFINES += $(BFC_MTA_DEFINES)

	BFC_INCLUDE_DIRS += $(BFC_MTA_INCLUDE_DIRS)

	BFC_SOURCE_DIRS += $(BFC_MTA_SOURCE_DIRS)

endif


############################################################################
## MTA Customer Extension Support
############################################################################
ifeq ($(BFC_INCLUDE_MTA_VENDOR_SUPPORT), 1)
 include $(BFC_MAKE_DIR)/$(MtaVendorExtensionMakefile)
endif



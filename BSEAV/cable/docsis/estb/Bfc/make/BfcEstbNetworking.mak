#############################################################################
#
# Copyright (c) 2008-2009 Broadcom Corporation
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
#############################################################################
#  $Id$
#
#  Filename:       BfcNetworking.mak
#  Author:         Kevin O'Neal
#  Creation Date:  September 12, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the Networking subsystem.
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

ifndef BCM_MANAGED_SWITCH_INCLUDED
    #BCM_MANAGED_SWITCH_INCLUDED = 1
endif

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

BFC_NETWORKING_DIR = Bfc/Networking

BFC_NETWORKING_STUB_DIR = $(BFC_NETWORKING_DIR)/Stubs

BFC_NETWORKING_DSBMPC_DIR = $(BFC_NETWORKING_DIR)/DsBondMpc

# Directories where the networking drivers and support files exist.
BFC_NETWORKING_BSP_COMMON_DIR = cm_bsp_v2/bsp_common
BFC_NETWORKING_DRIVERS_DIR = \
        $(BFC_NETWORKING_BSP_COMMON_DIR)/nihal \
        $(BFC_NETWORKING_BSP_COMMON_DIR)/common_inc \
        $(BFC_NETWORKING_BSP_COMMON_DIR)/common_src

BFC_IPHELPERS_DIR = Bfc/IpHelpers

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

# Networking support files.
# UNFINISHED - should we allow nonvol to be optional?
BFC_NETWORKING_OBJECTS = \
    EthernetPacket.o \
    InterfaceCounters.o \
    StubHalIf.o \
    Snoop.o \
    NiHalIf.o \
    PacketLoggingSnoop.o \
    PacketLoggingIpFilter.o PacketLoggingMacFilter.o \
    PacketLoggingPortFilter.o HalIf.o PacketSwitch.o\
    #HalIfFactory.o  \
    #HalIfNonVolSettings.o   NiHalIf.o \
    #PacketLoggingSnoop.o \
    #PacketLoggingIpFilter.o PacketLoggingMacFilter.o \
    #PacketLoggingPortFilter.o \
    #PacketSwitch.o Forwarder.o \
    #StubHalIf.o

BFC_NETWORKING_STUB_OBJECTS = BfcNetworkingStubs.o

BFC_ESTB_NETWORKING_CONSOLE_OBJECTS = SnoopCommandTable.o 

ifeq ($(BFC_INCLUDE_IPHELPERS_SUPPORT), 1)
	BFC_NETWORKING_OBJECTS     += BcmSocket.o SocketFactory.o LnxSocket.o SocketSet.o #$(OS_PREFIX)Socket.o 
endif

ifeq (PTASKIRAN, 1)
ifeq ($(BFC_TARGET_OS), Linux)
    BFC_NETWORKING_OBJECTS += boardhal_ioctlwrapper.o
endif

ifeq ($(BCM_MANAGED_SWITCH_INCLUDED),1)
    BFC_NETWORKING_OBJECTS += \
        ManageSwitchSnoop.o \
        EthManageSwitchInterfaceCounters.o
ifneq ($(BFC_TARGET_OS), Linux)
	BFC_NETWORKING_OBJECTS += \
        EnetManageSwitch.o
endif
endif

ifeq ($(BCM93380_DS_BONDING), 1)
    BFC_NETWORKING_OBJECTS += \
        dsbMpcThread.o          dsbMpcSnoop.o \
        dsbIpAddrSnoop.o        dsbMpcCommandTable.o \
        MpcClientACT.o          MpcEventPublisher.o \
        MpcTestThread.o         MpcTestClientACT.o \
        dsbRemoteOidThread.o    dsbRemoteOidClientACT.o \
        CmDocsisIpcEventACT.o   CmDocsisIpcHelper.o \
        CmDocsisIpcHelperACT.o  CmDsChanProxy.o \
        dsbMpcIfEntryBridge.o   dsbMpcDocsIfBridge.o
endif

ifeq ($(BCM_HARDWARE_FREE_POOL_MANAGER_INCLUDED), 1)
ifneq ($(BFC_TARGET_OS), Linux)
    BFC_NETWORKING_OBJECTS += \
        FreePoolManagerDriver.o
endif
endif

# Battery support files
ifeq ($(BFC_INCLUDE_BATTERY_SUPPORT), 1)
ifeq ($(POWER_MGMT), 1)
    BFC_NETWORKING_OBJECTS += NiHalIfPowerMgmtACT.o
endif
endif

# Optional interface for linking two BcmPacketSwitch objects together.  Only
# necessary if more than one BcmPacketSwitch is created.
BFC_NETWORKING_PACKETSWITCH_BRIDGING_OBJECTS = PacketSwitchBridgingHalIf.o



# Console files for various networking objects.
# UNFINISHED - should we allow nonvol console to be optional?
BFC_NETWORKING_CONSOLE_OBJECTS = ForwarderCommandTable.o \
                                 HalIfNonVolSettingsCommandTable.o
				 



# SNMP files for networking
BFC_NETWORKING_SNMP_OBJECTS = ifMib.o ifBridge.o IfUnknownProtosSnoop.o \
                              dot1dMib.o dot1dBridge.o

# Add dot1dStaticSnoop only if we support the dot1dStaticTable.
ifeq ($(BFC_INCLUDE_DOT1DSTATICTABLE_SUPPORT), 1)

    BFC_NETWORKING_SNMP_OBJECTS += dot1dStaticSnoop.o

endif



# Additional SNMP files for BRCM Runtime and Factory MIB support
BFC_NETWORKING_BRCM_RUNTIME_SNMP_OBJECTS =
BFC_NETWORKING_BRCM_FACTORY_SNMP_OBJECTS =

# For embedded targets, these are the basic support files that are commonly
# used by specific drivers.
BFC_NETWORKING_DRIVER_SUPPORT_OBJECTS = bcmAlloc.o Activity.o crc.o bpdialog.o

# The driver files are only used for some target Operating Systems (although it
# really follows the target, not the OS).
BFC_NETWORKING_DRIVER_VxWorks_SUPPORT_OBJECTS = $(BFC_NETWORKING_DRIVER_SUPPORT_OBJECTS)
BFC_NETWORKING_DRIVER_Win32_SUPPORT_OBJECTS =
BFC_NETWORKING_DRIVER_Ecos_SUPPORT_OBJECTS = $(BFC_NETWORKING_DRIVER_SUPPORT_OBJECTS)
BFC_NETWORKING_DRIVER_Qnx6_SUPPORT_OBJECTS = $(BFC_NETWORKING_DRIVER_SUPPORT_OBJECTS)

ifeq ($(POWER_MGMT), 1)
    BFC_NETWORKING_DRIVER_Ecos_SUPPORT_OBJECTS += RamPowerSave.o
endif

endif
##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_NETWORKING_DEFINES = \
    POWER_MGMT=$(POWER_MGMT) \
    BFC_INCLUDE_NETWORKING_SUPPORT=$(BFC_INCLUDE_NETWORKING_SUPPORT) \
    BFC_INCLUDE_NETWORKING_CONSOLE_SUPPORT=$(BFC_INCLUDE_NETWORKING_CONSOLE_SUPPORT) \
    BFC_INCLUDE_IPV6_SUPPORT=$(BFC_INCLUDE_IPV6_SUPPORT)

ifeq ($(BCM_HARDWARE_FREE_POOL_MANAGER_INCLUDED), 1)
    BFC_NETWORKING_DEFINES += \
        BCM_HARDWARE_FREE_POOL_MANAGER=1

    ifeq ($(BCM_HARDWARE_FPM_SRAM_DESCRIPTORS_ONLY_INCLUDED), 1)
        BFC_NETWORKING_DEFINES += \
           BCM_HARDWARE_FPM_SRAM_DESCRIPTORS_ONLY=1
    endif

endif


ifeq ($(BFC_INCLUDE_DOT1DSTATICTABLE_SUPPORT), 1)

    BFC_NETWORKING_DEFINES += BFC_INCLUDE_DOT1DSTATICTABLE_SUPPORT=1

endif

ifeq ($(BCM_MANAGED_SWITCH_INCLUDED),1)
    BFC_NETWORKING_DEFINES += MANAGED_SWITCH_INCLUDED=1
endif

ifeq ($(BCM_IPC_INCLUDED),1)
    BFC_NETWORKING_DEFINES += BCM_IPC_INCLUDED=1
endif

# UNFINISHED - I'm not sure if this is the best place to do this kind of thing.
ifeq ($(buildProcessor), 3348)
    BFC_NETWORKING_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3368)
    BFC_NETWORKING_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3361)
    BFC_NETWORKING_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(buildProcessor), 3255)
    BFC_NETWORKING_DEFINES += INTEGRATED_LED_CONTROL=1
endif

ifeq ($(BCM93380_DS_BONDING), 1)
    BFC_NETWORKING_DEFINES += SUPPORT_SNMP_PDU_ANNOUNCEMENTS=1
endif


##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_NETWORKING_INCLUDE_DIRS = $(BFC_NETWORKING_DIR)

ifeq ($(BFC_INCLUDE_NETWORKING_STUB_SUPPORT), 1)

    BFC_NETWORKING_INCLUDE_DIRS += $(BFC_NETWORKING_STUB_DIR)

endif

ifeq ($(BCM93380_DS_BONDING), 1)
    BFC_NETWORKING_INCLUDE_DIRS += $(BFC_NETWORKING_DSBMPC_DIR)
endif

# OS-specific networking support.
BFC_VxWorks_NETWORKING_INCLUDE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)
BFC_Win32_NETWORKING_INCLUDE_DIRS =
BFC_Ecos_NETWORKING_INCLUDE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)
BFC_Linux_NETWORKING_INCLUDE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)
BFC_Qnx6_NETWORKING_INCLUDE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)

ifeq ($(BFC_INCLUDE_IPHELPERS_SUPPORT), 1)
    BFC_NETWORKING_INCLUDE_DIRS += $(BFC_IPHELPERS_DIR)
endif

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_NETWORKING_SOURCE_DIRS = $(BFC_NETWORKING_DIR)

ifeq ($(BFC_TARGET_OS), Linux)
BFC_NETWORKING_SOURCE_DIRS += Bfc/Networking/linux
endif

ifeq ($(BFC_INCLUDE_NETWORKING_STUB_SUPPORT), 1)

    BFC_NETWORKING_SOURCE_DIRS += $(BFC_NETWORKING_STUB_DIR)

endif

ifeq ($(BCM93380_DS_BONDING), 1)
    BFC_NETWORKING_SOURCE_DIRS += $(BFC_NETWORKING_DSBMPC_DIR)
endif

# OS-specific networking support.
BFC_VxWorks_NETWORKING_SOURCE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)
BFC_Win32_NETWORKING_SOURCE_DIRS =
BFC_Ecos_NETWORKING_SOURCE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR) $(BFC_ECOS_PATCHES_DIR)
BFC_Linux_NETWORKING_SOURCE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)
BFC_Qnx6_NETWORKING_SOURCE_DIRS = $(BFC_NETWORKING_DRIVERS_DIR)

ifeq ($(BFC_INCLUDE_IPHELPERS_SUPPORT), 1)
    BFC_NETWORKING_SOURCE_DIRS += $(BFC_IPHELPERS_DIR)
endif

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_NETWORKING_LIBRARIES =


##############################################################################
#
# More Includes - These are included late because they make use of the macros
# that are a part of this makefile.  All of the interface-specific stuff used
# to be in this file, but it became too hard to follow, with way too many
# nested ifeq's.

include $(BFC_MAKE_DIR)/BfcEthernetNetworking.mak
ifneq ($(buildProcessor), 1112)
ifneq ($(buildProcessor), 1115)
    include $(BFC_MAKE_DIR)/BfcUsbNetworking.mak
endif
endif
#include $(BFC_MAKE_DIR)/BfcIpStackNetworking.mak
#include $(BFC_MAKE_DIR)/BfcWifi80211Networking.mak
#include $(BFC_MAKE_DIR)/BfcHpnaNetworking.mak
#include $(BFC_MAKE_DIR)/BfcPciNetworking.mak
#include $(BFC_MAKE_DIR)/BfcScbNetworking.mak
ifeq ($(BFC_INCLUDE_NETWORKING_SUPPORT), 1)
    include $(BFC_MAKE_DIR)/BfcIpHelpers.mak
    include $(BFC_MAKE_DIR)/BfcDhcp.mak
    include $(BFC_MAKE_DIR)/BfcPing.mak
    include $(BFC_MAKE_DIR)/BfcTftp.mak
    include $(BFC_MAKE_DIR)/BfcHttpServer.mak
    include $(BFC_MAKE_DIR)/BfcDnsResolver.mak
    include $(BFC_MAKE_DIR)/BfcIpV6.mak
    include $(BFC_MAKE_DIR)/BfcDhcpV6.mak
endif

include $(BFC_MAKE_DIR)/BfcEstbIpStackNetworking.mak

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add networking components, if included by BfcSystemConfig.mak.

# Add stub objects if required
ifeq ($(BFC_INCLUDE_NETWORKING_STUB_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_NETWORKING_STUB_OBJECTS)

	BFC_NETWORKING_DEFINES += BFC_INCLUDE_NETWORKING_STUB_SUPPORT=1
    BFC_COMPILER_DEFINES += $(BFC_NETWORKING_DEFINES)
	

    BFC_INCLUDE_DIRS += $(BFC_NETWORKING_INCLUDE_DIRS) 

    BFC_SOURCE_DIRS += $(BFC_NETWORKING_SOURCE_DIRS) 


endif

# Optional console modules.
ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)

    BFC_OBJECTS += $(BFC_ESTB_NETWORKING_CONSOLE_OBJECTS)

endif

endif

#ifeq ($(BFC_INCLUDE_NETWORKING_SUPPORT), 1)

    # Core networking modules
    BFC_OBJECTS += $(BFC_NETWORKING_OBJECTS)

    # Optional bridging interface for multiple Packet Switch objects.
    ifeq ($(BFC_INCLUDE_PACKETSWITCH_BRIDGING_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_NETWORKING_PACKETSWITCH_BRIDGING_OBJECTS)

    endif

    # Add stub objects if required
    #ifeq ($(BFC_INCLUDE_NETWORKING_STUB_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_NETWORKING_STUB_OBJECTS)

    #endif

    # Optional console modules.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)

            BFC_OBJECTS += $(BFC_ESTB_NETWORKING_CONSOLE_OBJECTS)

        endif

    endif

    ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_NETWORKING_SNMP_OBJECTS)

        # Optional runtime MIB support.
        ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)

            BFC_OBJECTS += $(BFC_NETWORKING_BRCM_RUNTIME_SNMP_OBJECTS)

        endif

        # Optional factory MIB support.
        ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)

            BFC_OBJECTS += $(BFC_NETWORKING_BRCM_FACTORY_SNMP_OBJECTS)

        endif

    endif

    # Driver support modules
    BFC_OBJECTS += $(BFC_NETWORKING_DRIVER_$(BFC_TARGET_OS)_SUPPORT_OBJECTS)


    BFC_COMPILER_DEFINES += $(BFC_NETWORKING_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_NETWORKING_INCLUDE_DIRS) $(BFC_$(BFC_TARGET_OS)_NETWORKING_INCLUDE_DIRS)

    BFC_SOURCE_DIRS += $(BFC_NETWORKING_SOURCE_DIRS) $(BFC_$(BFC_TARGET_OS)_NETWORKING_SOURCE_DIRS)

    BFC_LIBRARIES += $(BFC_NETWORKING_LIBRARIES)

#endif

                                                                                     

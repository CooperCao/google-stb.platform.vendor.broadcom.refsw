##############################################################################
#
# Copyright (c) 2007-2010 Broadcom Corporation
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
############################################################################
##                 DEFINES
############################################################################ 

############################################################################
##                 PATHS
############################################################################ 

PWD_DIR	          = $(CURDIR)
CORE_V2_APP_DIR   = ../../../../../Bfc/BfcApp
V2_COMMON_DIR     = ../../../../../Bfc
USER_IFACE_DIR    = $(V2_COMMON_DIR)/UserInterface
IP_HELPERS_DIR    = $(V2_COMMON_DIR)/IpHelpers
V2_DHCP_DIR       = $(V2_COMMON_DIR)/IpHelpers/Dhcp
OS_WRAPPER_DIR    = $(V2_COMMON_DIR)/OperatingSystem
NETWORKING_DIR 	  = $(V2_COMMON_DIR)/Networking
IP_NETWORKING_DIR = $(V2_COMMON_DIR)/Networking/Ip
PING_DIR          = $(IP_HELPERS_DIR)/Ping
CONSOLE_DIR       = ../../../Common/Linux
VERSION_DIR       = ../../../../Common
BSPCOMMON_DIR     = ../../../../../cm_bsp_v2/bsp_common/common_inc

PORTABLE_APP_DIR  = ../../../EstbApp_SnmpAgent
TARGETOS_APP_DIR  = ../../../EstbApp_SnmpAgent/$(LC_TARGETOS)


############################################################################
##                 OBJECT FILES
############################################################################                                       

PORTABLE_APP_OBJS = BfcApplication.o \
                    EstbApplication.o \
                    EstbAppMain.o  \
                    BfcAppCompositeNonVolSettings.o \
		    BfcAppNonVolSettings.o \
		    MessageLogNonVolSettings.o \
		    DiskFileNonVolDevice.o

##   PORTABLE_APP_OBJS += AppCompositeNonVolCommandTable.o                    
                
V2_COMMON_OBJS    = IpAddress.o \
		    IpV4Address.o \
		    IpV6Address.o \
                    MacAddress.o \
                    OctetBuffer.o \
                    Utilities.o \
                    EventPublisher.o \
                    IpAddressList.o \
		    Counter64.o \
                    V2Main.o \
                    MsgACT.o \
		    MsgPublisherOneToOne.o

IP_HELPER_OBJS    = 
                    
OS_WRAPPER_OBJS   = CountingSemaphore.o \
                    Event.o \
                    EventSet.o \
                    MutexSemaphore.o \
                    OperatingSystem.o \
                    OperatingSystemFactory.o \
                    $(OS_PREFIX)CountingSemaphore.o \
                    $(OS_PREFIX)Event.o \
                    $(OS_PREFIX)EventSet.o \
                    $(OS_PREFIX)MutexSemaphore.o \
                    $(OS_PREFIX)OperatingSystem.o \
                    $(OS_PREFIX)Interface.o \
		    $(OS_PREFIX)Timer.o \
                    SystemTime.o \
                    Thread.o \
                    Timer.o \
                    TimerMux.o \
                    TimerMuxACT.o \
	            CompletionHandlerACT.o \
		    stringfind.o
                    #$(OS_PREFIX)ArpHelper.o \
                    #HeapMonitorThread.o


OS_WRAPPER_OBJS  += LnxTimeout.o    

        
USER_IFACE_OBJS   = MessageLog.o \
                    MessageLogSettings.o \
                    

## HARDWARE_OBJS     = EthernetPacket.o 

ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 0)
CORE_V2_OBJS =  $(PORTABLE_APP_OBJS) \
                $(V2_COMMON_OBJS) \
                $(IP_HELPER_OBJS) \
                $(OS_WRAPPER_OBJS) \
                $(USER_IFACE_OBJS) \
                $(HARDWARE_OBJS)
endif                

CORE_V2_OBJS += EstbAppVersionInfo.o

############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################ 

EXTRA_INCLUDE += -I$(V2_COMMON_DIR) \
		 -I$(OS_WRAPPER_DIR) \
		 -I$(USER_IFACE_DIR) \
		 -I$(IP_HELPERS_DIR) \
		 -I$(NETWORKING_DIR) \
		 -I$(IP_NETWORKING_DIR) \
	         -I$(BSPCOMMON_DIR) \
		 -I$(PORTABLE_APP_DIR) \
		 -I$(V2_DHCP_DIR) \


ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 0)
EXTRA_INCLUDE += -I$(CORE_V2_APP_DIR) \
		 -I$(TARGETOS_APP_DIR) \
	         -I$(PING_DIR) \
	         -I$(CONSOLE_DIR) \
	         -I$(VERSION_DIR) 
endif		 
                 
############################################################################
##                 VPATH - source path
############################################################################

VPATH += $(NETWORKING_DIR):\
         $(IP_HELPERS_DIR):\
	 $(V2_COMMON_DIR):\
	 $(PORTABLE_APP_DIR):\

ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 0)
VPATH += $(CORE_V2_APP_DIR):\
         $(TARGETOS_APP_DIR):\
         $(V2_COMMON_DIR):\
         $(OS_WRAPPER_DIR):\
         $(USER_IFACE_DIR):\
         $(V2_DHCP_DIR):\
	 $(CONSOLE_DIR):
endif

############################################################################
##                 MACH_EXTRA - object files
############################################################################                                       
  
#ifeq ($(CORE_ESTBSNMP_BUILT_AS_LIB), 0)                 
MACH_EXTRA    += $(CORE_V2_OBJS)
#endif

############################################################################
##                 LIB_EXTRA - libraries
############################################################################                                       

############################################################################
##                 Special targets
############################################################################                                       



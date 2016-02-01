#****************************************************************************
#
# Copyright (c) 2007-2008 Broadcom Corporation
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
############################################################################
##                 V2 COMPONENT INCLUDES 
############################################################################

# Ip Helpers
BCM_SOCKET_SUPPORT=1
BCM_TFTP_SUPPORT=0
BCM_TOD_SUPPORT=0
BCM_CONSOLE_SUPPORT=1
BCM_DHCPS_SUPPORT=0
BCM_PING_SUPPORT=0
BCM_PING_CONSOLE_SUPPORT=0

ifeq ($(BCM_REDUCED_IMAGE_SIZE), 1)
  BCM_PING_CONSOLE_SUPPORT=0
  BCM_CONSOLE_SUPPORT=0
endif


# Non Vol
BCM_NONVOL_SUPPORT=1
BCM_NONVOLDEV_SUPPORT=1

# Forwarder
BCM_FORWARDER_SUPPORT=0

# Operating System 
BCM_MSGQ_SUPPORT=1

# HAL Support
BCM_BASICHALIF_SUPPORT=0
BCM_IPHALIF_SUPPORT=0
ENET_HAL_INCLUDED=0

# Program Store
BCM_PROGRAMSTORE_SUPPORT=0

# Miscellaneous
BCM_LOGO_SUPPORT=1


ifeq ($(BCM_BASICHALIF_SUPPORT),1)
  MACH_EXTRA     += HalIf.o HalIfFactory.o NiHalIf.o Snoop.o \
                    PacketLoggingSnoop.o InterfaceCounters.o \
                    PacketSwitchBridgingHalIf.o
  ADDED_CFLAGS   += -DBCM_BASICHALIF_SUPPORT=1
endif

ifeq ($(BPI_SUPPORT_HARDWARE),1)
  MACH_EXTRA     += SecureHalIf.o                         
endif

ifeq ($(BCM_IPHALIF_SUPPORT),1)
  MACH_EXTRA     += BaseIpHalIf.o $(OS_PREFIX)IpHalIf.o IpStackManager.o \
                    IpRoutingFilterSnoop.o
  ADDED_CFLAGS   += -DBCM_IPHALIF_SUPPORT=1
endif

ifeq ($(BCM_SOCKET_SUPPORT),1)
  MACH_EXTRA     += BcmSocket.o $(OS_PREFIX)Socket.o SocketFactory.o SocketSet.o 
  ADDED_CFLAGS   += -DBCM_SOCKET_SUPPORT=1
endif

ifeq ($(BCM_PING_SUPPORT),1)
  MACH_EXTRA     += PingThread.o  
  ADDED_CFLAGS   += -DBCM_PING_SUPPORT=1
endif

ifeq ($(BCM_DHCPC_SUPPORT),1)
  ADDED_CFLAGS   +=  -DBCM_DHCPC_SUPPORT=1
endif

ifeq ($(BCM_TOD_SUPPORT),1)
  MACH_EXTRA     +=  TimeOfDayThread.o
  ADDED_CFLAGS   +=  -DBCM_TOD_SUPPORT=1
endif

ifeq ($(BCM_TFTP_SUPPORT),1)
    MACH_EXTRA     +=  $(OS_PREFIX)Tftp.o Tftp.o
  ADDED_CFLAGS   +=  -DBCM_TFTP_SUPPORT=1
endif

ifeq ($(BCM_TELNET_SUPPORT),1)
  MACH_EXTRA     += TelnetThread.o TelnetThreadIpStackEventACT.o
  ADDED_CFLAGS   +=  -DBCM_TELNET_SUPPORT=1
endif

ifeq ($(BCM_NULL_CONSOLE),1)
EXTRA_INCLUDE += -I../../../Common/UserInterface
VPATH += ../../../Common/UserInterface
endif

ADDED_CFLAGS   +=  -DBCM_KERBEROS_SUPPORT=$(BCM_KERBEROS_SUPPORT)






############################################################################
##                 CONSOLE SUPPORT  
############################################################################

MACH_EXTRA +=  TerminalConsoleThread.o \
        ConsoleThread.o \
        Command.o \
        CommandTable.o \
        MainCommandTable.o  \
        BuiltInCommandTable.o \
        BitmaskCommandParameter.o \
        BoolCommandParameter.o \
        FlagCommandParameter.o \
        IpAddressCommandParameter.o \
        MacAddressCommandParameter.o \
        SignedIntCommandParameter.o \
        StringCommandParameter.o \
        UnsignedIntCommandParameter.o \
        CommandParameter.o \
        CommandParameterList.o \
        StringSetCommandParameter.o \
        FloatCommandParameter.o

ifeq ($(BCM_NULL_CONSOLE),1)
  MACH_EXTRA +=  NullTerminalConsoleThread.o
else
  MACH_EXTRA +=  LnxTerminalConsoleThread.o
endif

ifeq ($(BCM_TELNET_SUPPORT),1)
    MACH_EXTRA += LnxRemoteTerminalConsoleThread.o \
                  LnxTelnetHelperThread.o
endif

ifeq ($(BCM_CONSOLE_SUPPORT),1)
  MACH_EXTRA +=  CallTraceCommandTable.o \
        MsgLogSettingsCommandTable.o  \
        EstbAppCompositeNonVolCommandTable.o \
        NonVolSettingsCommandTable.o
  
        # MessageLogNonVolSettingsCommandTable.o \  
        # EstbAppNonVolCommandTable.o
          
  ifeq ($(BCM_BASICHALIF_SUPPORT),1)
    MACH_EXTRA += HalIfNonVolSettingsCommandTable.o \
        SnoopCommandTable.o
  endif
                      
  ifeq ($(BCM_IPHALIF_SUPPORT),1)
    MACH_EXTRA     += IpHalCommandTable.o     
  endif
  
  ifeq ($(BCM_FORWARDER_SUPPORT),1)
    MACH_EXTRA     += ForwarderCommandTable.o     
  endif
  
  ifeq ($(BCM_TELNET_SUPPORT),1)
    MACH_EXTRA     += RemoteAccessCommandTable.o
  endif
  
  ifeq ($(BCM_PING_CONSOLE_SUPPORT),1)
    MACH_EXTRA     += PingCommandTable.o 
    ADDED_CFLAGS   += -DBCM_PING_CONSOLE_SUPPORT=1
  endif
  
  ADDED_CFLAGS   +=  -DBCM_CONSOLE_SUPPORT=1 

endif


############################################################################
##                 END CONSOLE SUPPORT  
############################################################################

ifeq ($(BCM_NONVOL_SUPPORT),1)
  MACH_EXTRA     +=  EstbAppCompositeNonVolSettings.o \
        EstbAppNonVolSettings.o \
        CompositeNonVolSettings.o \
        UnrecognizedNonVolSettings.o \
        NonVolSettings.o 

  ADDED_CFLAGS   += -DBCM_NONVOL_SUPPORT=1
endif

ifeq ($(BCM_FORWARDER_SUPPORT),1)
  MACH_EXTRA     += PacketSwitch.o \
        Forwarder.o
  ADDED_CFLAGS   += -DBCM_FORWARDER_SUPPORT=1
endif

ifeq ($(BCM_MSGQ_SUPPORT),1)
  MACH_EXTRA     += MessageQueue.o \
        $(OS_PREFIX)MessageQueue.o 
  ADDED_CFLAGS   += -DBCM_MSGQ_SUPPORT=1
endif


ifeq ($(BCM_NONVOLDEV_SUPPORT),1)
  MACH_EXTRA     +=  NonVolDevice.o \
		     NonVolSettingsAccessLock.o
  ADDED_CFLAGS   += -DBCM_NONVOLDEV_SUPPORT=1
endif

ifeq ($(BCM_LOGO_SUPPORT),1)
  MACH_EXTRA     +=  CompanyLogo.o  
  ADDED_CFLAGS   += -DBCM_LOGO_SUPPORT=1
endif

############################################################################
##                 END V2 COMPONENT INCLUDES 
############################################################################



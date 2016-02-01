#############################################################################
# (c) 2003-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################

############################################################################
##                 V2 COMPONENT INCLUDES
############################################################################

# Ip Helpers
BCM_SOCKET_SUPPORT=1
BCM_TFTP_SUPPORT=0
BCM_DHCPC_SUPPORT=0
BCM_TOD_SUPPORT=0
BCM_CONSOLE_SUPPORT=0
BCM_DHCPS_SUPPORT=0
BCM_PING_SUPPORT=0
BCM_PING_CONSOLE_SUPPORT=0
BCM_PACKET_GENERATOR_SUPPORT=0
BCM_V2_DHCP_SUPPORT=0

ifeq ($(BCM_REDUCED_IMAGE_SIZE), 1)
  BCM_PACKET_GENERATOR_SUPPORT=0
  BCM_PING_CONSOLE_SUPPORT=0
  BCM_CONSOLE_SUPPORT=0
endif


# Non Vol
ifeq ($(BCM_NONVOL_SUPPORT),1)
  ADDED_CFLAGS   += -DBCM_NONVOL_SUPPORT=1
  BCM_NONVOLDEV_SUPPORT=1
endif

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
  DSGCC_OBJS     += $(TARGETDIR)/HalIf.o $(TARGETDIR)/HalIfFactory.o $(TARGETDIR)/NiHalIf.o $(TARGETDIR)/Snoop.o \
                    $(TARGETDIR)/PacketLoggingSnoop.o $(TARGETDIR)/InterfaceCounters.o \
                    $(TARGETDIR)/PacketSwitchBridgingHalIf.o
  ADDED_CFLAGS   += -DBCM_BASICHALIF_SUPPORT=1
endif

ifeq ($(BPI_SUPPORT_HARDWARE),1)
  DSGCC_OBJS     += $(TARGETDIR)/SecureHalIf.o
endif

ifeq ($(BCM_IPHALIF_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/BaseIpHalIf.o $(TARGETDIR)/$(OS_PREFIX)IpHalIf.o $(TARGETDIR)/IpStackManager.o \
                    $(TARGETDIR)/IpRoutingFilterSnoop.o
  ADDED_CFLAGS   += -DBCM_IPHALIF_SUPPORT=1
endif

ifeq ($(USB_HAL_INCLUDED),1)
  DSGCC_OBJS     += $(TARGETDIR)/UsbNiHalIf.o
  ADDED_CFLAGS   += -DUSB_HAL_INCLUDED=1
endif

ifeq ($(ENET_HAL_INCLUDED),1)
  DSGCC_OBJS     += $(TARGETDIR)/EthernetNiHalIf.o
  ADDED_CFLAGS   += -DENET_HAL_INCLUDED=1
endif

ifeq ($(HPNA_HAL_INCLUDED),1)
  DSGCC_OBJS     += $(TARGETDIR)/HpnaNiHalIf.o
  ADDED_CFLAGS   += -DHPNA_HAL_INCLUDED=1
endif

ifeq ($(BCM_SOCKET_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/BcmSocket.o $(TARGETDIR)/$(OS_PREFIX)Socket.o $(TARGETDIR)/SocketFactory.o $(TARGETDIR)/SocketSet.o
  ADDED_CFLAGS   += -DBCM_SOCKET_SUPPORT=1
endif

ifeq ($(BCM_PING_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/PingThread.o
  ADDED_CFLAGS   += -DBCM_PING_SUPPORT=1
endif

ifeq ($(BCM_DHCPC_SUPPORT),1)
  ADDED_CFLAGS   +=  -DBCM_DHCPC_SUPPORT=1
endif

ifeq ($(MANAGE_SWITCH_INCLUDED),1)
  DSGCC_OBJS     += $(TARGETDIR)/ManageSwitchSnoop.o $(TARGETDIR)/EthManageSwitchInterfaceCounters.o
  ADDED_CFLAGS   += -DMANAGED_SWITCH_INCLUDED=1
endif

#DPullen - I don't think we can add/remove this conditionally
#because the IP stacks require it.  Maybe this just needs to
#be carried along with BCM_IPHALIF_SUPPORT...
ifeq ($(BCM_V2_DHCP_SUPPORT),1)
  DSGCC_OBJS     +=  $(TARGETDIR)/DhcpLeaseSettings.o \
        $(TARGETDIR)/DhcpPacket.o \
        $(TARGETDIR)/DhcpClientId.o \
        $(TARGETDIR)/DhcpClientLease.o \
        $(TARGETDIR)/DhcpClientIf.o \
        $(TARGETDIR)/DhcpClientThread.o \
        $(TARGETDIR)/DhcpClientSnoop.o \
	$(TARGETDIR)/DhcpOptionsRfc2132.o
  ADDED_CFLAGS   +=  -DBCM_V2_DHCP_SUPPORT=1
endif

ifeq ($(BCM_TOD_SUPPORT),1)
  DSGCC_OBJS     +=  $(TARGETDIR)/TimeOfDayThread.o
  ADDED_CFLAGS   +=  -DBCM_TOD_SUPPORT=1
endif

ifeq ($(BCM_TFTP_SUPPORT),1)
  DSGCC_OBJS     +=  $(TARGETDIR)/$(OS_PREFIX)Tftp.o $(TARGETDIR)/Tftp.o
  ADDED_CFLAGS   +=  -DBCM_TFTP_SUPPORT=1
endif


ifeq ($(BCM_TELNET_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/TelnetThread.o $(TARGETDIR)/TelnetThreadIpStackEventACT.o
  ADDED_CFLAGS   +=  -DBCM_TELNET_SUPPORT=1
endif

ADDED_CFLAGS   +=  -DBCM_KERBEROS_SUPPORT=$(BCM_KERBEROS_SUPPORT)






############################################################################
##                 CONSOLE SUPPORT
############################################################################
ifeq ($(BCM_CONSOLE_SUPPORT),1)
DSGCC_OBJS +=  $(TARGETDIR)/TerminalConsoleThread.o \
        $(TARGETDIR)/$(OS_PREFIX)TerminalConsoleThread.o \
        $(TARGETDIR)/ConsoleThread.o \
        $(TARGETDIR)/Command.o \
        $(TARGETDIR)/CommandTable.o \
        $(TARGETDIR)/MainCommandTable.o  \
        $(TARGETDIR)/BuiltInCommandTable.o \
        $(TARGETDIR)/BitmaskCommandParameter.o \
        $(TARGETDIR)/BoolCommandParameter.o \
        $(TARGETDIR)/FlagCommandParameter.o \
        $(TARGETDIR)/IpAddressCommandParameter.o \
        $(TARGETDIR)/MacAddressCommandParameter.o \
        $(TARGETDIR)/SignedIntCommandParameter.o \
        $(TARGETDIR)/StringCommandParameter.o \
        $(TARGETDIR)/UnsignedIntCommandParameter.o \
        $(TARGETDIR)/CommandParameter.o \
        $(TARGETDIR)/CommandParameterList.o \
        $(TARGETDIR)/StringSetCommandParameter.o \
        $(TARGETDIR)/FloatCommandParameter.o

ifeq ($(BCM_TELNET_SUPPORT),1)
  ifeq "$(TARGETOS)" "vxWorks"
    DSGCC_OBJS += $(TARGETDIR)/VxRemoteTerminalConsoleThread.o
  endif
endif

ifeq ($(BCM_CONSOLE_SUPPORT),1)
  DSGCC_OBJS +=  $(TARGETDIR)/CallTraceCommandTable.o \
        $(TARGETDIR)/MsgLogSettingsCommandTable.o


   ifeq ($(BCM_NONVOL_SUPPORT),1)
	  DSGCC_OBJS +=  $(TARGETDIR)/CmAppCompositeNonVolCommandTable.o \
					 $(TARGETDIR)/CmAppNonVolCommandTable.o \
					 $(TARGETDIR)/NonVolSettingsCommandTable.o
	endif

  ifeq ($(BCM_BASICHALIF_SUPPORT),1)
    DSGCC_OBJS += vHalIfNonVolSettingsCommandTable.o \
        $(TARGETDIR)/SnoopCommandTable.o
  endif

  ifeq ($(ENET_HAL_INCLUDED),1)
    DSGCC_OBJS     += $(TARGETDIR)/EnetHalCommandTable.o
  endif

  ifeq ($(WIFI80211_HAL_INCLUDED),1)
    DSGCC_OBJS     += $(TARGETDIR)/WiFi80211CommandTable.o
  endif

  ifeq ($(USB_HAL_INCLUDED),1)
    DSGCC_OBJS     += $(TARGETDIR)/UsbHalCommandTable.o
  endif

  ifeq ($(HPNA_HAL_INCLUDED),1)
    DSGCC_OBJS     += $(TARGETDIR)/HpnaHalCommandTable.o
  endif

  ifeq ($(BCM_IPHALIF_SUPPORT),1)
    DSGCC_OBJS     += $(TARGETDIR)/IpHalCommandTable.o
  endif

  ifeq ($(BCM_FORWARDER_SUPPORT),1)
    DSGCC_OBJS     += $(TARGETDIR)/ForwarderCommandTable.o
  endif

  ifeq ($(BCM_TELNET_SUPPORT),1)
    DSGCC_OBJS     += $(TARGETDIR)/RemoteAccessCommandTable.o
  endif

  ifeq ($(BCM_PACKET_GENERATOR_SUPPORT),1)
    DSGCC_OBJS     += $(TARGETDIR)/PacketGeneratorHalCommandTable.o
  endif

  ifeq ($(BCM_PING_CONSOLE_SUPPORT),1)
    DSGCC_OBJS     += $(TARGETDIR)/PingCommandTable.o
    ADDED_CFLAGS   += -DBCM_PING_CONSOLE_SUPPORT=1
  endif

  ADDED_CFLAGS   +=  -DBCM_CONSOLE_SUPPORT=1

endif
endif

############################################################################
##                 END CONSOLE SUPPORT
############################################################################

ifeq ($(BCM_NONVOL_SUPPORT),1)
  DSGCC_OBJS     +=  $(TARGETDIR)/CmAppCompositeNonVolSettings.o \
        $(TARGETDIR)/CmAppNonVolSettings.o \
        $(TARGETDIR)/CompositeNonVolSettings.o \
        $(TARGETDIR)/UnrecognizedNonVolSettings.o \
        $(TARGETDIR)/NonVolSettings.o
  ADDED_CFLAGS   += -DBCM_NONVOL_SUPPORT=1
endif

ifeq ($(BCM_FORWARDER_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/PacketSwitch.o \
        $(TARGETDIR)/Forwarder.o
  ADDED_CFLAGS   += -DBCM_FORWARDER_SUPPORT=1
endif

ifeq ($(BCM_MSGQ_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/MessageQueue.o \
        $(TARGETDIR)/$(OS_PREFIX)MessageQueue.o
  ADDED_CFLAGS   += -DBCM_MSGQ_SUPPORT=1
endif


ifeq ($(BCM_PROGRAMSTORE_SUPPORT),1)
  DSGCC_OBJS += $(TARGETDIR)/ProgramStoreDevice.o \
        $(TARGETDIR)/ProgramStoreDeviceDriverBridge.o \
        $(TARGETDIR)/BootloaderStoreDevice.o \
        $(TARGETDIR)/BootloaderStoreDeviceDriverBridge.o
  ADDED_CFLAGS   += -DBCM_PROGRAMSTORE_SUPPORT=1
  ADDED_CFLAGS   += -DPROGRAM_STORE_SIGNATURE=$(PROGRAM_STORE_SIGNATURE)
endif

ifeq ($(BCM_NONVOLDEV_SUPPORT),1)
  DSGCC_OBJS     +=  $(TARGETDIR)/NonVolDevice.o \
		     $(TARGETDIR)/NonVolSettingsAccessLock.o
  ADDED_CFLAGS   += -DBCM_NONVOLDEV_SUPPORT=1
endif

ifeq ($(BCM_LOGO_SUPPORT),1)
  DSGCC_OBJS     +=  $(TARGETDIR)/CompanyLogo.o
  ADDED_CFLAGS   += -DBCM_LOGO_SUPPORT=1
endif

ifeq ($(BCM_PACKET_GENERATOR_SUPPORT),1)
  DSGCC_OBJS     += $(TARGETDIR)/PacketGeneratorThread.o \
        $(TARGETDIR)/PacketGeneratorHalIf.o
  ADDED_CFLAGS    += -DBCM_PACKET_GENERATOR_SUPPORT=1
endif

############################################################################
##                 END V2 COMPONENT INCLUDES
############################################################################



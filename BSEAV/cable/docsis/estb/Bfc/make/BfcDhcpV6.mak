##############################################################################
#
# Copyright (c) 2006-2008 Broadcom Corporation
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
#  Filename:       BfcDhcpV6.mak
#  Author:         David Pullen
#  Creation Date:  April 25, 2006
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the DHCPv6 Client and Server
#	subsystem.  Both of these components are optional.
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
# Paths - definitions of relative paths for this component.  These are used
#         below, and should be defined once, rather than using the actual
#         value in multiple places.
#
#         Note that the path needs to be specified relative to the "mount point"
#         of the parent project, not relative to the output directory.  The
#         make processing will specify the rest of the path for you.

BFC_DHCPV6_DIR = Bfc/IpHelpers/DhcpV6

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.

# Modules that are common for both the server and client.
BFC_DHCPV6_OBJECTS = DhcpV6Constants.o DhcpV6Packet.o

BFC_DHCPV6_OBJECTS += DhcpV6Option.o DhcpV6OptionWithOptions.o \
                      DhcpV6OptionsList.o

BFC_DHCPV6_OBJECTS += DhcpV6Uint8Option.o DhcpV6Uint16Option.o \
                      DhcpV6Uint32Option.o DhcpV6Int32Option.o \
		      DhcpV6VariableLengthBufferOption.o

BFC_DHCPV6_OBJECTS += DhcpV6RapidCommitOption.o	DhcpV6ReconfigureAcceptOption.o \
                      DhcpV6PreferenceOption.o DhcpV6ReconfigureMessageOption.o \
		      DhcpV6ElapsedTimeOption.o DhcpV6DocsisTimeOffsetOption.o \
		      DhcpV6StatusCodeOption.o DhcpV6DocsisCfgFileNameOption.o \
                      DhcpV6OptionRequestOption.o DhcpV6Duid.o \
		      DhcpV6DocsisOptionRequestOption.o \
		      DhcpV6DocsisTlv5EncodingOption.o \
		      DhcpV6DocsisCableLabsClientConfigOption.o \
            DhcpV6DocsisCableLabsClientConfigOptionV6.o \
		      DhcpV6MacAddressListOption.o \
		      DhcpV6DocsisDeviceIdentifierOption.o \
                      DhcpV6DocsisRelayAgentCmMacAddressOption.o \
		      DhcpV6IpV6AddressListOption.o \
		      DhcpV6ServerUnicastOption.o \
		      DhcpV6DnsServersOption.o \
		      DhcpV6DocsisTftpServerAddressesOption.o \
		      DhcpV6DocsisSyslogServerAddressesOption.o \
		      DhcpV6DocsisTimeProtocolServersOption.o \
		      DhcpV6OptionWithClassData.o DhcpV6UserClassOption.o \
                      DhcpV6VendorClassOption.o DhcpV6AuthenticationOption.o \
                      DhcpV6IaAddress.o DhcpV6Ia.o DhcpV6StatusCodeOption.o \
		      DhcpV6VendorSpecificInfoOption.o \
                      DhcpV6CableLabsVendorSpecificInfoOption.o \
                      DhcpV6DocsisCmVendorClassOption.o \
		      DhcpV6DocsisDeviceTypeOption.o \
		      DhcpV6DocsisEmbeddedComponentsListOption.o \
		      DhcpV6DocsisDeviceSerialNumberOption.o \
		      DhcpV6DocsisHardwareVersionNumberOption.o \
                      DhcpV6DocsisSoftwareVersionNumberOption.o \
                      DhcpV6DocsisBootRomVersionNumberOption.o \
                      DhcpV6DocsisOuiOption.o \
                      DhcpV6DocsisModelNumberOption.o \
                      DhcpV6DocsisVendorNameOption.o \
		      DhcpV6DomainListOption.o		     

# Modules specific to the client.
BFC_DHCPV6_CLIENT_OBJECTS = DhcpV6ClientIf.o DhcpV6ClientSnoop.o \
                            DhcpV6ClientThread.o DhcpV6ClientSoftTimer.o \
			    DhcpV6ClientNeighborDiscoveryACT.o

# Modules specific to the server.
BFC_DHCPV6_SERVER_OBJECTS = 

ifeq ($(BCM_STANDALONE_DHCP_CLIENT), 0)   
	BFC_DHCPV6_OBJECTS += DhcpV6IaPrefix.o
endif

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_DHCPV6_DEFINES = BFC_INCLUDE_DHCPV6_SUPPORT=$(BFC_INCLUDE_DHCPV6_SUPPORT) \
		     BFC_INCLUDE_DHCPV6_SERVER_SUPPORT=$(BFC_INCLUDE_DHCPV6_SERVER_SUPPORT)

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_DHCPV6_INCLUDE_DIRS = $(BFC_DHCPV6_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_DHCPV6_SOURCE_DIRS = $(BFC_DHCPV6_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_DHCPV6_LIBRARIES = 


# Add DHCPv6 components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_DHCPV6_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_DHCPV6_OBJECTS) $(BFC_DHCPV6_CLIENT_OBJECTS) 
 
    ifeq ($(BFC_INCLUDE_DHCPV6_SERVER_SUPPORT), 1)
        BFC_OBJECTS += $(BFC_DHCPV6_SERVER_OBJECTS)
    endif
    
    # Add Dhcp console components, if support for Console and DHCPv6 Console
    # are both included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_INCLUDE_DHCPV6_CONSOLE_SUPPORT), 1)

            BFC_OBJECTS += $(BFC_DHCPV6_CONSOLE_OBJECTS)

        endif
	
    endif

    BFC_COMPILER_DEFINES += $(BFC_DHCPV6_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_DHCPV6_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_DHCPV6_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_DHCPV6_LIBRARIES)
    
endif

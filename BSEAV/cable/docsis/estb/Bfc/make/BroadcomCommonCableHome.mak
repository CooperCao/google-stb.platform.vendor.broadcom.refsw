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
#  Filename:       BroadcomCommonCableHome.mak
#  Author:         John McQueen
#  Creation Date:  Jan 22, 2004
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the Broadcom CH Vendor Extension.
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

# This product is a CableHome device, so it uses files from the Common directory and
# the CommonCableHome directory.

BFC_BCM_CABLEHOME_EXT_DIR_BASE =CableHome/ChExtensionApi

BFC_BCM_CABLEHOME_EXT_DIR += \
        $(BFC_BCM_CABLEHOME_EXT_DIR_BASE)/Broadcom/CommonCableHome \
        $(BFC_BCM_CABLEHOME_EXT_DIR_BASE)/Broadcom/Common \
        $(BFC_BCM_CABLEHOME_EXT_DIR_BASE)/Broadcom/Common/Oss
        

BFC_BCM_CM_EXT_DIR_BASE = Cm/CmExtensionApi

BFC_BCM_CM_EXT_DIR += \
        $(BFC_BCM_CM_EXT_DIR_BASE)/Broadcom/CommonDataOnly \
        $(BFC_BCM_CM_EXT_DIR_BASE)/Broadcom/Common \
        $(BFC_BCM_CM_EXT_DIR_BASE)/Broadcom/Common/SNMP
        

# OS-specific directories (if any).
ifeq ($(BFC_TARGET_OS), VxWorks)

    BFC_BCM_CABLEHOME_EXT_DIR += $(BFC_BCM_CABLEHOME_EXT_DIR_BASE)/Broadcom/CommonCableHome/vxWorks

endif

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

BFC_BCM_COMMON_CABLEHOME_OBJECTS = \
        VendorChApplicationBase.o \
        VendorChApplication.o \
        VendorChAppFactory.o \
        VendorExtensionChStatusACT.o \
    IpTosSnoop.o \
    LocalNetworkInterfaceSnoop.o \
    LocalNetworkInterface.o \
    LocalNetworkInterfaceTable.o \
    LocalNetworkPassthroughIfTable.o \
        LocalNetworkTable.o \
    LocalNetworkTableEntry.o \
    LocalNetworkInterfaceManager.o \
    LocalNetworkInterfaceManagerACTHandler.o \
    LocalNetworkDhcpServerPool.o \
    LocalNetworkDhcpServerPoolTable.o \
    LocalNetworkDhcpServerPoolEventACT.o \
        VendorHttpSpoofResponses.o

# SNMP / OSS related vendor extension modules
BFC_BCM_COMMON_CABLEHOME_SNMP_OBJECTS += \
        CableHomeVendorEventLog.o \
        CableHomeVendorSnmpAgent.o \
        brcmRgControlMib.o \
        brcmRgControlBridge.o

ifeq ($(BFC_INCLUDE_HTTP_SERVER_SUPPORT),1)        

        # Additional files for the HTTP Server extension.
        ifeq ($(BCM_80211N), 1)
            ifeq ($(BCM_WIFI_HOTSPOT_SUPPORT),1)
               BFC_BCM_COMMON_CABLEHOME_OBJECTS +=  HttpFormHandlerwlanHotspotPgpPublicKey.o
            endif
        endif
        
        
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += \
                        RgHttpServerThread.o \
                        HttpFormHandlerRgOptions.o \
                        HttpFormHandlerRgIpFiltering.o \
                        HttpFormHandlerRgMacFiltering.o \
                        HttpFormHandlerRgPortFiltering.o \
                        HttpFormHandlerRgForwarding.o \
                        HttpFormHandlerRgPortTriggering.o \
                        HttpFormHandlerRgDmzHost.o \
                        HttpFormHandlerRgRipSetup.o \
                        HttpFormHandlerRgSetup.o \
                        HttpFormHandlerRgDhcp.o \
                        HttpFormHandlerRgDiagnostics.o \
                        HttpFormRegistrarRgBasic.o \
                        HttpFormRegistrarRgAdvanced.o
                
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += \
                        HttpFormHandlerRgBackup.o \
                        HttpFormHandlerRgExportConfig.o \
                        HttpExportManager.o
        
        ifeq ($(BCM_HTTP_UPGRADE_SUPPORT), 1)
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerRgUpgrade.o
        endif
        
        ifeq ($(BCM_PARENTALCTRL_ENABLED), 1)
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormRegistrarParentalControl.o
        endif 
        
        ifeq ($(BCM_DDNS_SUPPORT), 1)
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerRgDdns.o
        endif
        
        ifeq ($(BCM_VPN_SUPPORT), 1)
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerRgVpnBasic.o HttpFormHandlerRgVpnConfigure.o HttpFormHandlerRgVpnEventLog.o HttpFormHandlerRgVpn.o \
                HttpFormRegistrarVpn.o
        
        ifeq ($(BCM_L2TP_SUPPORT),1)
        ifeq ($(BCM_PPTP_SUPPORT),1)
                BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerRgVpnL2tpPptp.o 
                endif
        endif
        
        endif
        
        ifeq ($(BFC_INCLUDE_HOMEPLUG_SUPPORT),1)
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormRegistrarHomePlug.o
        endif
        
        ifeq ($(BFC_CABLEHOME_STANALONE_PS), 1)
        
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormRegistrarSps.o HttpFormRegistrarStatusSps.o
                
        ifeq ($(BCM_IPC_INCLUDED),1)
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerConnectProxy.o HttpFormHandlerSwInfoProxy.o
        else
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerConnectSps.o HttpFormHandlerSwInfoSps.o
        endif
        
        else
        BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormRegistrarEps.o 
	ifneq ($(BFC_INCLUDE_COMCAST_SUPPORT),1)
		BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormRegistrarStatusEps.o HttpFormRegistrarCm.o HttpFormRegistrarStatusCm.o
	endif
        endif
        
        ifeq ($(BCM_80211N),1) 
              BFC_BCM_COMMON_CABLEHOME_OBJECTS += HttpFormHandlerwlanGuestNetwork.o HttpFormManagerWiFiMbss.o WiFi80211MbssHelper.o
        endif
      
        
endif # HTTP_SUPPORT

        ifeq ($(BCM_80211N),1)
	      BFC_BCM_COMMON_CABLEHOME_OBJECTS += WiFi80211GuestNetworkNonVolSettings.o \
	 				    WiFi80211GuestSettings.o WiFi80211GuestSettingsCache.o \
                                                  WiFi80211GuestNetworkNonVolSettingsCommandTable.o \
                                                  WiFi80211GuestNetworkIpServiceAppIf.o
        endif

BFC_BCM_COMMON_DATA_ONLY_CM_OBJECTS += \
        VendorCmAppFactory.o \
        VendorCmApplicationBase.o \
        VendorExtensionCmStatusACT.o 
                                       
# SNMP / OSS related vendor extension modules
BFC_BCM_COMMON_DATA_ONLY_CM_OBJECTS += \
        CmVendorSnmpAgent.o \
        CpeVendorSnmpAgent.o \
        CmVendorEventLog.o \
        BroadcomVendorCmApplication.o 
        


# WiFi hotspot support
ifeq ($(BCM_WIFI_HOTSPOT_SUPPORT),1)

BFC_BCM_COMMON_CABLEHOME_OBJECTS += WiFi80211HotspotThread.o WiFi80211HotspotSnoop.o pgpdsa.o \
                                    WiFi80211HotspotNonVolSettings.o WiFi80211HotspotNonVolSettingsCommandTable.o \
                                    WiFi80211HotspotIpServiceAppIf.o

endif

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_BCM_COMMON_CABLEHOME_DEFINES = BFC_COMMON_VENDOR_SUPPORT=1 BFC_CABLEHOME_VENDOR_SUPPORT=1 WEBS_PAGE_ROM=1

ifeq ($(BCM_HTTP_UPGRADE_SUPPORT), 1)
BFC_BCM_COMMON_CABLEHOME_DEFINES += BCM_HTTP_UPGRADE_SUPPORT=1
endif

ifeq ($(BRCM_HTTP_SSL_SUPPORT), 1)

BFC_BCM_COMMON_CABLEHOME_DEFINES += BRCM_HTTP_SSL_SUPPORT=1

	ifeq ($(BRCM_HTTP_SSL_STRONG_CIPHER), 1)
		BFC_BCM_COMMON_CABLEHOME_DEFINES += BRCM_HTTP_SSL_STRONG_CIPHER=1
	else
		BFC_BCM_COMMON_CABLEHOME_DEFINES += BRCM_HTTP_SSL_STRONG_CIPHER=0
	endif

endif

ifeq ($(BCM_80211N),1)
      BFC_BCM_COMMON_CABLEHOME_DEFINES += BCM_GUEST_NETWORK_SUPPORT=1
endif

ifeq ($(BCM_WIFI_HOTSPOT_SUPPORT),1)
BFC_BCM_COMMON_CABLEHOME_DEFINES += BCM_WIFI_HOTSPOT_SUPPORT=1
endif

# Additional defines for HTTP Server extension; the web pages are compiled
# (as though in a ROM, though they are in the code) rather than read from
# nonvol settings.


##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_BCM_COMMON_CABLEHOME_INCLUDE_DIRS = \
        $(BFC_BCM_CM_EXT_DIR) \
        $(BFC_BCM_CM_EXT_DIR_BASE) \
        $(BFC_BCM_CABLEHOME_EXT_DIR) \
        $(BFC_BCM_CABLEHOME_EXT_DIR_BASE)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_BCM_COMMON_CABLEHOME_SOURCE_DIRS = \
        $(BFC_BCM_CM_EXT_DIR) \
        $(BFC_BCM_CM_EXT_DIR_BASE) \
        $(BFC_BCM_CABLEHOME_EXT_DIR) \
        $(BFC_BCM_CABLEHOME_EXT_DIR_BASE)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_BCM_COMMON_CABLEHOME_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add Broadcom CM Vendor Extension component.  I don't need any conditional
# stuff for most of it; this makefile will only be included if the vendor
# extension is enabled.

BFC_OBJECTS += $(BFC_BCM_COMMON_CABLEHOME_OBJECTS)

 
ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

BFC_OBJECTS += $(BFC_BCM_COMMON_CABLEHOME_SNMP_OBJECTS)  

endif

 
ifeq ($(BFC_CABLEHOME_STANALONE_PS), 1)        
else
    BFC_OBJECTS += $(BFC_BCM_COMMON_DATA_ONLY_CM_OBJECTS)
endif 
       

BFC_COMPILER_DEFINES += $(BFC_BCM_COMMON_CABLEHOME_DEFINES)

BFC_INCLUDE_DIRS += $(BFC_BCM_COMMON_CABLEHOME_INCLUDE_DIRS)
    
BFC_SOURCE_DIRS += $(BFC_BCM_COMMON_CABLEHOME_SOURCE_DIRS)
    
BFC_LIBRARIES += $(BFC_BCM_COMMON_CABLEHOME_LIBRARIES)



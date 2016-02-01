##############################################################################
#
# Copyright (c) 2005-2008 Broadcom Corporation
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
#  Filename:       BfcHttpWebUiBrcm.mak
#  Author:         Tyler White
#  Creation Date:  Aug 23, 2005
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the HTTP Server subsystem.
#       This subsystem is optional.
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

BRCM_HTTP_SERVER_DIR = Bfc/IpHelpers/Https

BRCM_HTTP_WEB_RESOURCES_DIR = Bfc/IpHelpers/Https/WebResources

BRCM_HTTP_PROXY_DIR = Bfc/IpHelpers/Https/HttpProxy

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

# These files are the actual GUI Server.
BRCM_HTTP_SERVER_OBJECTS = HttpConfigurationServerAgent.o \
                HttpWebDirectory.o \
                WebResourceLib.o \
                VendorWebResourceLib.o \
		VendorAspHelper.o \
		VendorCgiHelper.o \
                HttpFormHandler.o \
		HttpFormManagerWiFi.o
                
# include login/logout forms if form authorization is enabled
ifeq ($(BCM_HTTP_FORM_AUTHORIZATION),1)
BRCM_HTTP_SERVER_OBJECTS += HttpFormHandlerlogin.o HttpFormHandlerlogout.o
endif

ifeq ($(BCM_IPC_INCLUDED),1)
BRCM_HTTP_SERVER_OBJECTS += HttpFormHandlerSecurityProxy.o HttpProxyHelper.o

ifeq ($(BFC_INCLUDE_DOCSIS_SUPPORT), 1)
# RG proxy forms
BRCM_HTTP_SERVER_OBJECTS += HttpFormHandlerRgSetupProxy.o \
                                HttpFormHandlerRgOptionsProxy.o \
                                HttpFormHandlerRgContentFilterProxy.o \
                                HttpFormHandlerParentalBasicProxy.o \
                                HttpFormHandlerwlanBasicProxy.o \
                                HttpFormHandlerRgVpnBasicProxy.o
endif

else
BRCM_HTTP_SERVER_OBJECTS += HttpFormHandlerSecurity.o
endif

ifeq ($(BFC_INCLUDE_DOCSIS_SUPPORT), 1)
BRCM_HTTP_SERVER_OBJECTS += HttpFormHandlerConnect.o \
                HttpFormHandlerSwInfo.o \
                HttpFormHandlerEventLog.o \
                HttpFormRegistrarStatusCm.o \
                HttpFormRegistrarCm.o \
                WebUiHelperDocsis.o
endif

ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)
BRCM_HTTP_SERVER_OBJECTS += HttpFormRegistrarMta.o \
                WebUiHelperMta.o \
                HttpFormHandlerMtaStatus.o \
                HttpFormHandlerMtaDhcp.o \
                HttpFormHandlerMtaQos.o \
                HttpFormHandlerMtaProvisioning.o \
                HttpFormHandlerMtaEventLog.o
endif

ifeq ($(BFC_INCLUDE_BATTERY_SUPPORT),1)
BRCM_HTTP_SERVER_OBJECTS += HttpFormRegistrarBattery.o \
                HttpFormHandlerBatController.o \
                HttpFormHandlerBatUps.o \
                HttpFormHandlerBatInterfaceDelay.o \
                WebUiHelperBattery.o
endif

ifeq ($(BFC_INCLUDE_WIFI80211_SUPPORT), 1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormHandlerWiFi80211.o \
                                          HttpFormHandlerwlanRadio.o \
                                          HttpFormHandlerwlanPrimaryNetwork.o \
                                          HttpFormHandlerwlanAdvanced.o \
                                          HttpFormHandlerwlanAccess.o \
                                          HttpFormHandlerwlanBridging.o \
                                          HttpFormRegistrarWiFi.o
ifeq ($(BCM_80211N),1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormHandlerwlanWmm.o \
				HttpFormHandlerwlanWps.o
endif

ifeq ($(HAL80211_SES_INCLUDED),1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormHandlerwlanSesPopup.o
endif

endif

ifeq ($(BFC_INCLUDE_HOMEPLUG_SUPPORT),1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormHandlerHomePlugSetup.o
endif

# OK - now the form registrars
BRCM_HTTP_SERVER_OBJECTS += HttpFormRegistrar.o

ifeq ($(BFC_INCLUDE_HOMEPLUG_SUPPORT),1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormRegistrarHomePlug.o
endif

# Optional objects if SNMP is supported, for management of HTTP things
BRCM_HTTP_BRCM_RUNTIME_SNMP_OBJECTS = brcmHttpMgmtMib.o brcmHttpMgmtBridge.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)
        ifeq ($(BFC_MTA_LIBRARY_SUFFIX),_pktc15)
        BRCM_HTTP_SERVER_DEFINES = BRCM_HTTP_PKTC15_EVENTLOG=1
        endif
        # TRW - if building from CxC source
        ifeq ($(CXC_PKTC_VERSION),15)
        BRCM_HTTP_SERVER_DEFINES = BRCM_HTTP_PKTC15_EVENTLOG=1
        endif
        
        ifeq ($(CXC_PKTC_VERSION),20)
        BRCM_HTTP_SERVER_DEFINES = BRCM_HTTP_PKTC20_EVENTLOG=1
        endif
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BRCM_HTTP_SERVER_INCLUDE_DIRS = $(BFC_HTTP_SERVER_DIR)

ifeq ($(BCM_IPC_INCLUDED),1)
BRCM_HTTP_SERVER_INCLUDE_DIRS += $(BFC_HTTP_PROXY_DIR)
endif

ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)
BRCM_HTTP_SERVER_INCLUDE_DIRS += Mta/Http
endif


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BRCM_HTTP_SERVER_SOURCE_DIRS = $(BFC_HTTP_SERVER_DIR)

ifeq ($(BCM_IPC_INCLUDED),1)
BRCM_HTTP_SERVER_SOURCE_DIRS += $(BFC_HTTP_PROXY_DIR)
endif

ifeq ($(BFC_INCLUDE_EMTA_SUPPORT), 1)
BRCM_HTTP_SERVER_SOURCE_DIRS += Mta/Http
endif

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BRCM_HTTP_SERVER_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add HTTP Server components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_HTTP_SERVER_SUPPORT), 1)

    BFC_OBJECTS += $(BRCM_HTTP_SERVER_OBJECTS)
    
    # Add Snmp components, if included by BfcSystemConfig.mak.
    ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)
    
        # Add BRCM private MIB modules, if we have support for BRCM private
        # MIBs
        ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
        
             BFC_OBJECTS += $(BRCM_HTTP_BRCM_RUNTIME_SNMP_OBJECTS)
        
        endif
    
    endif
 
    BFC_COMPILER_DEFINES += $(BRCM_HTTP_SERVER_DEFINES)

    BFC_INCLUDE_DIRS += $(BRCM_HTTP_SERVER_INCLUDE_DIRS) $(BRCM_HTTP_WEB_RESOURCES_DIR)
    
    BFC_SOURCE_DIRS += $(BRCM_HTTP_SERVER_SOURCE_DIRS) $(BRCM_HTTP_WEB_RESOURCES_DIR)
    
    BFC_LIBRARIES += $(BRCM_HTTP_SERVER_LIBRARIES)

endif

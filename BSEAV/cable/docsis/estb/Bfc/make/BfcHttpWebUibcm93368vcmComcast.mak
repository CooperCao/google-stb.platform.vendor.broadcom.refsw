##############################################################################
#
#  Copyright (c) 2006  Broadcom Corporation
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
#  Filename:       BfcHttpWebUibcm93368vcmComcast.mak
#  Author:         Tyler White
#  Creation Date:  Feb 23, 2006
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

BRCM_HTTP_WEB_PAGES_DIR = $(BrcmMsoHttpExtensionDir)

BRCM_HTTP_WEB_RESOURCES_DIR = $(BrcmMsoHttpExtensionDir)/WebResources

ifeq ($(BFC_INCLUDE_WIFI80211_SUPPORT),1)
BRCM_HTTP_WEB_RESOURCES_DIR += Bfc/IpHelpers/Https/WebResources
endif


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
                VendorWebResourceLibComcastEmta.o \
                HttpFormRegistrar.o \
		VendorAspHelper.o \
		VendorCgiHelper.o \
                HttpFormHandler.o \
                WebUiHelperDocsis.o \
                WebUiHelperMta.o \
                WebUiHelperRemoteMgmt.o \
                HttpPasswordOfTheDay.o \
                HttpPasswordOfTheDayHelper.o

ifeq ($(BFC_INCLUDE_BATTERY_SUPPORT), 1)
BRCM_HTTP_SERVER_OBJECTS += 			HttpFormHandlerBasicBattery.o \
		WebUiHelperBattery.o
endif
                
BRCM_HTTP_SERVER_OBJECTS += HttpFormRegistrarComcastEmta.o

ifeq ($(BFC_INCLUDE_WIFI80211_SUPPORT),1)
BRCM_HTTP_SERVER_OBJECTS += HttpFormRegistrarComcastWvg.o \
                VendorWebResourceLib.o \
                HttpFormHandlerWiFi80211.o \
				          HttpFormHandlerwlanRadio.o \
                                          HttpFormHandlerwlanPrimaryNetwork.o \
                                          HttpFormHandlerwlanAdvanced.o \
                                          HttpFormHandlerwlanAccess.o \
                                          HttpFormHandlerwlanBridging.o \
                                          HttpFormRegistrarWiFi.o \
					  HttpFormManagerWiFi.o
ifeq ($(BCM_80211N),1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormHandlerwlanWmm.o \
				HttpFormHandlerwlanWps.o
endif

ifeq ($(HAL80211_SES_INCLUDED),1)
BRCM_HTTP_SERVER_OBJECTS +=     HttpFormHandlerwlanSesPopup.o
endif

endif

# Vendor-specific web pages here
BRCM_HTTP_SERVER_OBJECTS += HttpFormHandlerlogin.o \
                        HttpFormHandlerlogout.o \
                        HttpFormHandlerBasicCmState.o \
                        HttpFormHandlerBasicEvents.o \
                        HttpFormHandlerBasicHardware.o \
                        HttpFormHandlerBasicStatus.o \
                        HttpFormHandlerAdvConfig.o \
                        HttpFormHandlerAdvDhcp.o \
                        HttpFormHandlerAdvEntrance.o \
                        HttpFormHandlerAdvMta.o \
                        HttpFormHandlerAdvProduct.o \
                        HttpFormHandlerAdvQos.o \
                        HttpFormHandlerAdvEvents.o \
                        HttpFormHandlertechsupport.o
                        

# Optional objects if SNMP is supported, for management of HTTP things
BRCM_HTTP_BRCM_RUNTIME_SNMP_OBJECTS = brcmHttpMgmtMib.o brcmHttpMgmtBridge.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BRCM_HTTP_SERVER_DEFINES = BCM_HTTP_FORM_AUTHORIZATION=1 BCM_HTTP_CUSTOM_WEBUI=1 BCM_TELNET_SSH_POTD=1

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BRCM_HTTP_SERVER_INCLUDE_DIRS = $(BFC_HTTP_SERVER_DIR) $(BRCM_HTTP_WEB_RESOURCES_DIR) $(BRCM_HTTP_WEB_PAGES_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BRCM_HTTP_SERVER_SOURCE_DIRS = $(BFC_HTTP_SERVER_DIR) $(BRCM_HTTP_WEB_RESOURCES_DIR) $(BRCM_HTTP_WEB_PAGES_DIR)

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

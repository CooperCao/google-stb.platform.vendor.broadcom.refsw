##############################################################################
#
#  Copyright (c) 2003  Broadcom Corporation
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
#
#  Filename:       BfcParentalControl.mak
#  Author:         Pinar Taskiran
#  Creation Date:  February 17, 2004
#
##############################################################################
#  Description:
#
#	Component makefile for Parental Controls system/application-specific files.  It
#	adds the files and other directives necessary to build this project.
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
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BCM_ONLINE_LOOKUP_SERVICE=0
BCM_PARENTALCTRL_ENABLED=0
BCM_POPUPBLOCKER_ENABLED=0
ENABLED=0

ifeq ($(BFC_PARENTAL_CONTROL_ENABLED),1)
	BFC_PARENTALCTRL_DEFINES = BCM_PARENTALCTRL_ENABLED=1
	BFC_PARENTALCTRL_DEFINES += BCM_ONLINE_LOOKUP_SERVICE=1
	BFC_PARENTALCTRL_DEFINES += BFC_DNS_SUPPORT=0

	BCM_PARENTALCTRL_ENABLED=1
	BCM_ONLINE_LOOKUP_SERVICE=1
	ENABLED=1
endif

# NOT YET
ifeq ($(BCM_FIREWALL_ENABLED),1)
	BFC_PARENTALCTRL_DEFINES += BCM_PARENTALCTRL_ENABLED=1
	BCM_PARENTALCTRL_ENABLED=1
else
	ifeq ($(BFC_POPUPBLOCKER_ENABLED),1)
		BFC_PARENTALCTRL_DEFINES += BCM_POPUPBLOCKER_ENABLED=1
		
		BCM_POPUPBLOCKER_ENABLED=1
		ENABLED=1
	endif		
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

BFC_RSLIB_SUPPORT_DIR = Bfc/LibSupport/ParentalCtrl
BFC_CBLIB_SUPPORT_DIR = Bfc/LibSupport/ParentalCtrl/Cerberian
BFC_PARENTAL_CONTROL_DIR = Bfc/IpHelpers/ParentalControl

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.
# Common objects for Parental Control and Pop-up Blocker
ifeq ($(ENABLED), 1) 
    BFC_PARENTAL_CONTROL_OBJECTS   += \
				ParentalControlApplication.o 
				
    ifneq ($(BCM_FIREWALL_ENABLED),1)
    BFC_PARENTAL_CONTROL_OBJECTS   += \
				ContentFilterSnoop.o \
				FirewallNonVolSettings.o 
    endif
    
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT),1)				
	ifneq ($(BCM_FIREWALL_ENABLED), 1)
	BFC_PARENTAL_CONTROL_OBJECTS   += \
				FirewallNonVolSettingsCommandTable.o   
	endif
	
    endif

endif

# Pop-Up Blocker ONLY objects
ifeq ($(BCM_POPUPBLOCKER_ENABLED), 1)
    BFC_PARENTAL_CONTROL_OBJECTS   += \
				HttpPopUpBlockerSnoop.o
				
        ifeq ($(BFC_INCLUDE_HTTP_SERVER_SUPPORT),1)        
            BFC_PARENTAL_CONTROL_OBJECTS   += \
                                HttpPopUpBlockerFormHandler.o 
        endif
                                
endif

# Parental Control ONLY objects
ifeq ($(BCM_PARENTALCTRL_ENABLED), 1)

    BFC_PARENTAL_CONTROL_OBJECTS   += \
				ParentalControlSnoop.o \
				ParentalControlApplication.o \
				ParentalControlNonVolSettings.o \
				ParentalControlStatusACT.o \
				ParentalCtlThread.o \
				HttpContentRuleSettings.o \
				UserProfile.o \
				HttpCategory.o \
				ParentalCtrlPacket.o \
				SessionManager.o \
				UrlCache.o \
				ContentPolicyList.o \
				PolicyManager.o \
				UserProfileList.o 
        
        ifeq ($(BFC_INCLUDE_HTTP_SERVER_SUPPORT),1)        
            BFC_PARENTAL_CONTROL_OBJECTS   += \
				HttpFormHandlerParentalBasic.o \
                HttpFormHandlerParentalPassword.o \
                HttpFormHandlerParentalUserSetup.o \
                HttpFormHandlerParentalEL.o \
                HttpFormHandlerTodFilter.o \
                HttpFormHandlerParentalUserLogin.o \
                HttpFormRegistrarParentalControl.o
        endif
       			       			
    # These objects are only added if online lookup service is enabled			
    ifeq ($(BCM_ONLINE_LOOKUP_SERVICE), 1)

    BFC_PARENTAL_CONTROL_OBJECTS   += \
				ParentalCtlService.o \
				CBParentalCtlService.o \
				ParentalCtrlV2Interface.o \
				brcmParentalControlMgmtBridge.o \
				brcmParentalControlMgmtMib.o \
				RSParentalCtlService.o \
				ParentalControlSoapHelper.o \
				Customer.o
		
        ifeq ($(BFC_INCLUDE_HTTP_SERVER_SUPPORT),1)        
            BFC_PARENTAL_CONTROL_OBJECTS   += \
				HttpFormHandlerParentalAdvanced.o \
				HttpFormHandlerParentalRetail.o
        endif
		
    endif
								
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT),1)
        BFC_PARENTAL_CONTROL_OBJECTS   += \
				ParentalCtrlNonVolSettingsCommandTable.o				
    endif
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_PARENTAL_CONTROL_INCLUDE_DIRS = $(BFC_PARENTAL_CONTROL_DIR)	$(BFC_RSLIB_SUPPORT_DIR) $(BFC_CBLIB_SUPPORT_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_PARENTAL_CONTROL_SOURCE_DIRS = $(BFC_PARENTAL_CONTROL_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.
# For Broadcom use only.
CREATE_PARENTAL_LIBRARY=0

ifeq ($(CREATE_PARENTAL_LIBRARY), 1)
	include $(BFC_MAKE_DIR)/Cerberian.mak
	include $(BFC_MAKE_DIR)/Rulespace.mak
else
	ifeq ($(BCM_ONLINE_LOOKUP_SERVICE), 1)

		BFC_PARENTAL_CONTROL_LIBRARIES = $(BFC_RSLIB_SUPPORT_DIR)/librulespace_$(BFC_TARGET_OS).a \
						 $(BFC_CBLIB_SUPPORT_DIR)/libcerberian_$(BFC_TARGET_OS).a						 
	endif
endif

 
##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

BFC_OBJECTS += $(BFC_PARENTAL_CONTROL_OBJECTS)

BFC_COMPILER_DEFINES += $(BFC_PARENTALCTRL_DEFINES)

BFC_INCLUDE_DIRS += $(BFC_PARENTAL_CONTROL_INCLUDE_DIRS)

BFC_SOURCE_DIRS += $(BFC_PARENTAL_CONTROL_SOURCE_DIRS)

BFC_LIBRARIES += $(BFC_PARENTAL_CONTROL_LIBRARIES)




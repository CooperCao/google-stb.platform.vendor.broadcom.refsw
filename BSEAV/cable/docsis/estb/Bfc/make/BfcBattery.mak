##############################################################################
#
#  Copyright (c) 2005  Broadcom Corporation
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
#  Filename:       BfcBattery.mak
#  Author:         Kevin O'Neal
#  Creation Date:  January 13, 2005
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for BFC battery management.
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

BFC_BATTERY_DIR = Bfc/Battery

BFC_BATTERY_STUB_DIR = $(BFC_BATTERY_DIR)/Stubs

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.


BFC_BATTERY_OBJECTS = BatteryThread.o BatterySoftTimer.o \
                      BatteryNonVolSettings.o BatteryController.o \
                      TemperatureCalibrator.o batteryCommon.o \
                      battery.o
                   
# Optional console support files.
BFC_BATTERY_CONSOLE_OBJECTS = BatteryCommandTable.o BatteryNonVolSettingsCommandTable.o

# Optional SNMP support files.
BFC_BATTERY_SNMP_OBJECTS = upsMib.o upsBridge.o upsTrap.o \
                           upsAlarmBatteryACT.o
                           
BFC_BATTERY_FACTORY_OBJECTS = 
                           
BFC_BATTERY_BRCM_SNMP_OBJECTS = brcmBatteryMgmtMib.o brcmBatteryMgmtBridge.o

# OS-specific battery support.
BFC_VxWorks_BATTERY_OBJECTS = 

BFC_Win32_BATTERY_OBJECTS =

BFC_Linux_BATTERY_OBJECTS =

BFC_Ecos_BATTERY_OBJECTS =

BFC_Qnx6_BATTERY_OBJECTS =

# These objects are temporary, only here to serve as stubs until the real
# objects are in place.  For BFC interim development.
BFC_BATTERY_STUB_OBJECTS = StubBattery.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_BATTERY_DEFINES = BCM_BATTERY_SUPPORT=1 \
                      BFC_INCLUDE_BATTERY_SUPPORT=1

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_BATTERY_INCLUDE_DIRS = $(BFC_BATTERY_DIR)

ifeq ($(BFC_INCLUDE_BATTERY_STUB_SUPPORT), 1)

        BFC_BATTERY_INCLUDE_DIRS += $(BFC_BATTERY_STUB_DIR)

endif

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_BATTERY_SOURCE_DIRS = $(BFC_BATTERY_DIR)

ifeq ($(BFC_INCLUDE_BATTERY_STUB_SUPPORT), 1)

        BFC_BATTERY_SOURCE_DIRS += $(BFC_BATTERY_STUB_DIR)

endif

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_BATTERY_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add battery components, if battery support is included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_BATTERY_SUPPORT), 1)

    # Core battery objects
    BFC_OBJECTS += $(BFC_BATTERY_OBJECTS) $(BFC_$(BFC_TARGET_OS)_BATTERY_OBJECTS)
    
    # Add stub objects if required
    ifeq ($(BFC_INCLUDE_BATTERY_STUB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_BATTERY_STUB_OBJECTS)
    
    endif
    
    # Add battery console components, if support for Console is included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)
    
            BFC_OBJECTS += $(BFC_BATTERY_CONSOLE_OBJECTS)
	
        endif
	
    endif
    
    # Add battery SNMP components, if support for SNMP is included.
    ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_BATTERY_SNMP_OBJECTS)
        
        # Add factory objects if we have factory support
        ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)
        
            BFC_OBJECTS += $(BFC_BATTERY_FACTORY_OBJECTS)
        
        endif
        
        # Add runtime objects if we have BRCM runtime MIB support
        ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
        
            BFC_OBJECTS += $(BFC_BATTERY_BRCM_SNMP_OBJECTS)
        
        endif
	
    endif

    BFC_COMPILER_DEFINES += $(BFC_BATTERY_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_BATTERY_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_BATTERY_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_BATTERY_LIBRARIES)
    
endif


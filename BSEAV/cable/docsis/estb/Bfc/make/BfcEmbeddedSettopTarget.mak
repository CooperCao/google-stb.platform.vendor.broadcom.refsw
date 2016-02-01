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
#  Filename:       BfcEmbeddedSettopTarget.mak
#  Author:         Pinar Taskiran
#  Creation Date:  November 27, 2006
#
##############################################################################
#  Description:
#
#       This is a variation of file BfcStdEmbeddedTarget.cpp.
# 	This file contains the build "stuff" for the nonStandard Embedded Target
#       files.
#       When the target is configured as PCI-Client mode, No EBI bus device, such
#       as flash, exist.  Either local serial flash or remote PCI-flash will be used
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

# set this to 1 to get a test thread that can be used to 
# simulate applications that are CPU intensive. 
# use the console command waste_time to kick the thread off. 


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



##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

# Include a different file if build for PCI client mode target, coz there are no ebi devices.
ifeq ($(BFC_INCLUDE_PCI_CLIENT_SUPPORT), 1)
include $(BFC_MAKE_DIR)/BfcStdPciClientTarget.mak
endif

# Include a different file if build for PCI client mode target, coz there are no ebi devices.
ifeq ($(BFC_INCLUDE_UBUS_CLIENT_SUPPORT), 1)
include $(BFC_MAKE_DIR)/BfcStdUbusClientTarget.mak
endif

# If neither in PCI client mode or UBUS client mode, then we'll include the standard embedded
# target support.
ifneq ($(BFC_INCLUDE_PCI_CLIENT_SUPPORT), 1)
ifneq ($(BFC_INCLUDE_UBUS_CLIENT_SUPPORT), 1)
include $(BFC_MAKE_DIR)/BfcStdEmbeddedTarget.mak
endif
endif

BFC_INCLUDE_DIRS += $(BFC_STD_EMBEDDED_TARGET_INCLUDE_DIRS)

BFC_SOURCE_DIRS += $(BFC_STD_EMBEDDED_TARGET_SOURCE_DIRS)


BFC_OBJECTS  += StbProgramStoreDeviceDriver.o \
                StbProgramStoreDeviceDriverBridge.o

ifeq ($(buildProcessor), 3348)
include $(BFC_MAKE_DIR)/BfcStdEmbeddedTarget.mak
BFC_OBJECTS += Bfc497xEmbeddedTarget.o 
endif

ifeq ($(buildProcessor), 3255)
BFC_OBJECTS += Bfc745xEmbeddedTarget.o 
endif

ifeq ($(buildProcessor), 7118)
BFC_OBJECTS += Bfc745xEmbeddedTarget.o 
endif
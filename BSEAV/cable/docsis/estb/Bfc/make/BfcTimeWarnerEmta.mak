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
#  $Id: BfcTimeWarnerEmta.mak 1.1 2006/07/10 19:24:03Z sdunlap Release $
#
#  Filename:       BfcTimeWarnerEmta.mak
#  Author:         Sean Dunlap
#  Creation Date:  July 10, 2006
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for IPv6 support
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

BFC_TIME_WARNER_DIR = Mta/TimeWarner

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.

# Modules for Time Warner EMTA

BFC_TIME_WARNER_OBJECTS = TimeWarnerDhcpStateMachineGlue.o


##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

ifeq ($(BFC_INCLUDE_TIME_WARNER_SUPPORT), 1)
	BFC_TIME_WARNER_DEFINES = BCM_TIME_WARNER_SUPPORT=1 
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_TIME_WARNER_INCLUDE_DIRS = $(BFC_TIME_WARNER_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_TIME_WARNER_SOURCE_DIRS = $(BFC_TIME_WARNER_DIR)

# Add IPV6 components, if included by BfcSystemConfig.mak.

ifeq ($(BFC_INCLUDE_TIME_WARNER_SUPPORT), 1)
BFC_OBJECTS += $(BFC_TIME_WARNER_OBJECTS) 
endif
 
ifeq ($(BFC_INCLUDE_TIME_WARNER_SUPPORT), 1)
    BFC_OBJECTS += $(BFC_TIME_WANRER_OBJECTS)
endif

BFC_COMPILER_DEFINES += $(BFC_TIME_WARNER_DEFINES)

BFC_INCLUDE_DIRS += $(BFC_TIME_WARNER_INCLUDE_DIRS)
    
BFC_SOURCE_DIRS += $(BFC_TIME_WARNER_SOURCE_DIRS)
    

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
#  $Id: BfcEdocsisSled.mak 1.4 2004/08/16 18:48:51Z dpullen Release $
#
#  Filename:       BfcEdocsisSled.mak
#  Author:         David Pullen
#  Creation Date:  December 5, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for eDOCSIS SLED.
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

BFC_EDOCSIS_SLED_DIR = Docsis/eDOCSIS

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.


BFC_EDOCSIS_SLED_OBJECTS = Sled.o SledLoopbackSnoop.o SledPacketGeneratorThread.o

# SNMP support objects.
BFC_EDOCSIS_SLED_SNMP_OBJECTS = sledBridge.o sledMib.o

# Debugging support objects; normally, this will be excluded.
BFC_EDOCSIS_SLED_DEBUG_OBJECTS = SledPacketCatcherThread.o

# Optional console support files.
#
# UNFINISHED - this hasn't been implemented due to time constraints.  I may get
#              back to it later on.  Don't try to compile it.
#BFC_EDOCSIS_SLED_CONSOLE_OBJECTS = SledCommandTable.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_EDOCSIS_SLED_DEFINES = BFC_INCLUDE_EDOCSIS_SLED_SUPPORT=1

# Set this to 1 to enable SLED debug; normally you want this set to 0.
BFC_EDOCSIS_SLED_DEFINES += BFC_INCLUDE_EDOCSIS_SLED_DEBUG_SUPPORT=0

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_EDOCSIS_SLED_INCLUDE_DIRS = $(BFC_EDOCSIS_SLED_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_EDOCSIS_SLED_SOURCE_DIRS = $(BFC_EDOCSIS_SLED_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_EDOCSIS_SLED_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add Docsis OSS components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_EDOCSIS_SLED_SUPPORT), 1)

    # Core eDOCSIS SLED objects
    BFC_OBJECTS += $(BFC_EDOCSIS_SLED_OBJECTS)

    ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_EDOCSIS_SLED_SNMP_OBJECTS)

    endif

    ifeq ($(BFC_INCLUDE_EDOCSIS_SLED_DEBUG_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_EDOCSIS_SLED_DEBUG_OBJECTS)

    endif

    # Add eDOCSIS SLED console components, if support for Console and eDOCSIS
    # SLED Console are both included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_INCLUDE_EDOCSIS_SLED_CONSOLE_SUPPORT), 1)

            BFC_OBJECTS += $(BFC_EDOCSIS_SLED_CONSOLE_OBJECTS)

        endif
	
    endif

    BFC_COMPILER_DEFINES += $(BFC_EDOCSIS_SLED_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_EDOCSIS_SLED_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_EDOCSIS_SLED_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_EDOCSIS_SLED_LIBRARIES)

endif


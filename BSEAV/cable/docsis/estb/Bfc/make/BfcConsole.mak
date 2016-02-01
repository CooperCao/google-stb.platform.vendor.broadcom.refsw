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
#  $Id: BfcConsole.mak 1.10 2005/09/03 21:06:32Z sdunlap Release dpullen $
#
#  Filename:       BfcConsole.mak
#  Author:         David Pullen
#  Creation Date:  March 28, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the Console subsystem.  The
#	console can be completely removed, or can be completely present.
#	Different application components can optionally add their own command
#	tables and/or commands.
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

BFC_CONSOLE_DIR = Bfc/UserInterface \
                  Bfc/UserInterface/RemoteAccessCommandTables

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

# Core console support files.
BFC_CONSOLE_OBJECTS = ConsoleThread.o TerminalConsoleThread.o \
                      CommandTable.o BuiltInCommandTable.o \
		      Command.o ConsoleThreadFactory.o

# Command parameter files.
BFC_CONSOLE_OBJECTS += CommandParameterList.o CommandParameter.o \
		       BitmaskCommandParameter.o BoolCommandParameter.o \
		       FlagCommandParameter.o FloatCommandParameter.o \
		       IpAddressCommandParameter.o MacAddressCommandParameter.o \
		       SignedIntCommandParameter.o StringCommandParameter.o \
		       StringSetCommandParameter.o UnsignedIntCommandParameter.o


# OS-specific console support.
BFC_VxWorks_CONSOLE_OBJECTS = vxTerminalConsoleThread.o

BFC_Win32_CONSOLE_OBJECTS = Win32TerminalConsoleThread.o

BFC_Linux_CONSOLE_OBJECTS = LnxTerminalConsoleThread.o

BFC_Ecos_CONSOLE_OBJECTS = ecosTerminalConsoleThread.o

BFC_Qnx6_CONSOLE_OBJECTS = QnxTerminalConsoleThread.o


##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_CONSOLE_DEFINES = BFC_INCLUDE_CONSOLE_SUPPORT=$(BFC_INCLUDE_CONSOLE_SUPPORT)

# Translate the BFC configuration variable into the old-school V2 compiler
# define.  This gets the majority of the console stuff excluded from slim
# builds.
ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 1)
    BFC_CONSOLE_DEFINES += BCM_CONSOLE_SUPPORT=0
else
    BFC_CONSOLE_DEFINES += BCM_CONSOLE_SUPPORT=1
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_CONSOLE_INCLUDE_DIRS = $(BFC_CONSOLE_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_CONSOLE_SOURCE_DIRS = $(BFC_CONSOLE_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_CONSOLE_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add Console components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_CONSOLE_OBJECTS) $(BFC_$(BFC_TARGET_OS)_CONSOLE_OBJECTS)
    
    BFC_COMPILER_DEFINES += $(BFC_CONSOLE_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_CONSOLE_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_CONSOLE_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_CONSOLE_LIBRARIES)
    
endif



##############################################################################
# 
#  Copyright (c) 2003-2009 Broadcom Corporation
# 
#  This program is the proprietary software of Broadcom Corporation and/or
#  its licensors, and may only be used, duplicated, modified or distributed
#  pursuant to the terms and conditions of a separate, written license
#  agreement executed between you and Broadcom (an "Authorized License").
#  Except as set forth in an Authorized License, Broadcom grants no license
#  (express or implied), right to use, or waiver of any kind with respect to
#  the Software, and Broadcom expressly reserves all rights in and to the
#  Software and all intellectual property rights therein.  IF YOU HAVE NO
#  AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#  AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#  SOFTWARE.  
# 
#  Except as expressly set forth in the Authorized License,
# 
#  1.     This program, including its structure, sequence and organization,
#  constitutes the valuable trade secrets of Broadcom, and you shall use all
#  reasonable efforts to protect the confidentiality thereof, and to use this
#  information only in connection with your use of Broadcom integrated circuit
#  products.
# 
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
# 
##############################################################################
#  $Id$
#
#  Filename:       BfcMessageLogging.mak
#  Author:         David Pullen
#  Creation Date:  March 28, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the BFC Message Logging code,
#	which is used throughout pretty much all BFC components and
#	applications.  This component is not optional; it is specified
#	separately just for convenience and clarity.
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

BFC_MESSAGE_LOGGING_DIR = Bfc/UserInterface

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

BFC_MESSAGE_LOGGING_OBJECTS = MessageLog.o MessageLogSettings.o	VersionBanner.o

# Optional console files.
BFC_MESSAGE_LOGGING_CONSOLE_OBJECTS = CallTraceCommandTable.o MsgLogSettingsCommandTable.o

ifneq ($(BCM_ESTB_STANDALONE_APP_SUPPORT), 1)
# Optional nonvol files.
BFC_MESSAGE_LOGGING_NONVOL_OBJECTS = MessageLogNonVolSettings.o \
                                     SystemEventLogNonVolSettings.o \
				     ResetSystemEventLogNonVolSettings.o 			     

# Optional nonvol console files.
BFC_MESSAGE_LOGGING_NONVOL_CONSOLE_OBJECTS = MessageLogNonVolSettingsCommandTable.o
endif
##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_MESSAGE_LOGGING_DEFINES = BFC_CONFIG_MESSAGE_LOGGING_OSTREAM_DEFAULT_NULL=$(BFC_CONFIG_MESSAGE_LOGGING_OSTREAM_DEFAULT_NULL) \
                              BFC_CONFIG_MESSAGE_LOGGING_OSTREAM_DEFAULT=$(BFC_CONFIG_MESSAGE_LOGGING_OSTREAM_DEFAULT)

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_MESSAGE_LOGGING_INCLUDE_DIRS = $(BFC_MESSAGE_LOGGING_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_MESSAGE_LOGGING_SOURCE_DIRS = $(BFC_MESSAGE_LOGGING_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_MESSAGE_LOGGING_LIBRARIES = 

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

BFC_OBJECTS += $(BFC_MESSAGE_LOGGING_OBJECTS)

# Add Message Logging console stuff, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)

    ifneq ($(BFC_CONFIG_MINIMAL_CONSOLE), 1)

        BFC_OBJECTS += $(BFC_MESSAGE_LOGGING_CONSOLE_OBJECTS)

    endif
    
endif

# Add Message Logging nonvol stuff (and nonvol console stuff), if included by
# BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_NONVOL_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_MESSAGE_LOGGING_NONVOL_OBJECTS)
    
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)
    
            ifeq ($(BFC_INCLUDE_NONVOL_CONSOLE_SUPPORT), 1)
	
	        BFC_OBJECTS += $(BFC_MESSAGE_LOGGING_NONVOL_CONSOLE_OBJECTS)
	    
            endif
	
        endif
	
    endif
    
endif


BFC_COMPILER_DEFINES += $(BFC_MESSAGE_LOGGING_DEFINES)

BFC_INCLUDE_DIRS += $(BFC_MESSAGE_LOGGING_INCLUDE_DIRS)

BFC_SOURCE_DIRS += $(BFC_MESSAGE_LOGGING_SOURCE_DIRS)

BFC_LIBRARIES += $(BFC_MESSAGE_LOGGING_LIBRARIES)



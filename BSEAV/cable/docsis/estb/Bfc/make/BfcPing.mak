#//****************************************************************************
#//
#// Copyright (c) 2003-2009 Broadcom Corporation
#//
#// This program is the proprietary software of Broadcom Corporation and/or
#// its licensors, and may only be used, duplicated, modified or distributed
#// pursuant to the terms and conditions of a separate, written license
#// agreement executed between you and Broadcom (an "Authorized License").
#// Except as set forth in an Authorized License, Broadcom grants no license
#// (express or implied), right to use, or waiver of any kind with respect to
#// the Software, and Broadcom expressly reserves all rights in and to the
#// Software and all intellectual property rights therein.  IF YOU HAVE NO
#// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
#// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
#// SOFTWARE.  
#//
#// Except as expressly set forth in the Authorized License,
#//
#// constitutes the valuable trade secrets of Broadcom, and you shall use all
#// reasonable efforts to protect the confidentiality thereof, and to use this
#// information only in connection with your use of Broadcom integrated circuit
#// products.
#//
#// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
#// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
#// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
#// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
#// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
#// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#//
#// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
#// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
#// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
#// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
#// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
#// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
#// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
#// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#//
#//****************************************************************************
#  $Id$
#
#  Filename:       BfcPing.mak
#  Author:         David Pullen
#  Creation Date:  March 28, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the Ping subsystem, which
#	allows the system to generate ping packets of various sizes, etc.
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

BFC_PING_DIR = Bfc/IpHelpers/Ping

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.

# Core ping objects
BFC_PING_OBJECTS = PingThread.o KeepAlivePingWorker.o KeepAlivePingAgent.o

# Optional console ping objects
BFC_PING_CONSOLE_OBJECTS = PingCommandTable.o

# Optional SNMP objects
BFC_PING_SNMP_OBJECTS = 

BFC_PING_FACTORY_OBJECTS = 

BFC_PING_BRCM_SNMP_OBJECTS = brcmPingMgmtMib.o brcmPingMgmtBridge.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_PING_DEFINES = BCM_PING_SUPPORT=1 \
                   BFC_INCLUDE_PING_SUPPORT=$(BFC_INCLUDE_PING_SUPPORT) \
		   BFC_INCLUDE_PING_CONSOLE_SUPPORT=$(BFC_INCLUDE_PING_CONSOLE_SUPPORT)

ifeq ($(BFC_INCLUDE_PING_CONSOLE_SUPPORT), 1)

    BFC_PING_DEFINES += BCM_PING_CONSOLE_SUPPORT=1
    
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_PING_INCLUDE_DIRS = $(BFC_PING_DIR)

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_PING_SOURCE_DIRS = $(BFC_PING_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_PING_LIBRARIES = 


# Add Ping components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_PING_SUPPORT), 1)

    BFC_OBJECTS += $(BFC_PING_OBJECTS)

    # Add Ping console components, if support for Console and PING Console
    # are both included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0) 
    
            ifeq ($(BFC_INCLUDE_PING_CONSOLE_SUPPORT), 1) 


                BFC_OBJECTS += $(BFC_PING_CONSOLE_OBJECTS)

            endif

        endif	
	
    endif

	ifeq ($(BFC_INCLUDE_TELNET_CONSOLE_SUPPORT), 1)
                BFC_OBJECTS += $(BFC_PING_CONSOLE_OBJECTS)
	endif
        
    # Include SNMP support.
    ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_PING_SNMP_OBJECTS)
        
        # Add factory objects if we have factory support
        ifeq ($(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT), 1)
        
            BFC_OBJECTS += $(BFC_PING_FACTORY_OBJECTS)
        
        endif
        
        # Add runtime objects if we have BRCM runtime MIB support
        ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
        
            BFC_OBJECTS += $(BFC_PING_BRCM_SNMP_OBJECTS)
        
        endif

    endif

    BFC_COMPILER_DEFINES += $(BFC_PING_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_PING_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_PING_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_PING_LIBRARIES)

endif

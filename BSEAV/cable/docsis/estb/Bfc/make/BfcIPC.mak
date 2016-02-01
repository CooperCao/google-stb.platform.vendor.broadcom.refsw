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
#  Filename:       BfcIpc.mak
#  Author:         Chris Zacker
#  Creation Date:  December 23, 2004
#
##############################################################################
#  Description:
#
#     This file contains the build "stuff" for the TLS and OpenSSL components.
#
##############
#
# External Variables (input/output):
#
#    BFC_OBJECTS - the list of object files that need to be built for the
#              project based on the current configuration.
#
#    BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#              to the compiler.
#
#    BFC_INCLUDE_DIRS - list of directories where header files can be found;
#              this will be passed to the compiler.
#
#    BFC_SOURCE_DIRS - list of directories where source files can be found;
#              this is used by the Make utility.
#
#    BFC_LIBRARIES - list of libraries that are to be used when linking the
#              application.
#
# External Constants (input only; do not modify these):
#
#    BFC_TARGET_OS - the OS that the project is being built for (i.e. Win32,
#              VxWorks, Ecos).  This can be used when making decisions
#              about which files/directories/libararies to include.
#
#    BFC_TARGET_PROCESSOR - the processor that the project is being built
#              for (i.e. Intel, MIPS32, etc).  This can be used
#              when making decisions about which
#              files/directories/libararies to include.
#
#    BFC_ROOT_DIR - the "mount point" for the source code, relative to the
#              output directory for the project.  This is defined in
#              the top-level makefile for the OS/target.
#
#    BFC_MAKE_DIR - this is essentially $(BFC_ROOT_DIR)/Bfc/make, and is
#              the directory where the core BFC component makefiles
#              are located.
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

BFC_IPC_DIR = Bfc/IpHelpers/Ipc


##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.
ifeq ($(BCM_IPC_INCLUDED),1)

ifeq ($(BFC_CABLEHOME_STANALONE_PS),1)
BFC_IPC_OBJECTS+=  IPCSnoop.o \
                   IPCThread.o \
                   IPCThreadCableHomeStatusClientACT.o 
else
BFC_IPC_OBJECTS += InterProcessComm.o \
                   IpcTestBridge.o \
                   IpcTestMib.o \
                   ipc_capMib.o \
                   ipc_cdpMib.o \
                   ipc_psdevMib.o \
                   ipc_ctpMib.o \
                   ipc_secMib.o \
                   ipc_clab_defMib.o \
                   ipc_chQos2Mib.o \
                   ipc_capBridge.o \
                   ipc_cdpBridge.o \
                   ipc_psdevBridge.o \
                   ipc_ctpBridge.o \
                   ipc_secBridge.o \
                   ipc_clab_defBridge.o \
                   ipc_chQos2Bridge.o
endif                   
       
BFC_IPC_OBJECTS += IpcSnmpAgent.o                  
 
endif
                  

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_IPC_DEFINES = 

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_IPC_INCLUDE_DIRS = $(BFC_IPC_DIR) 

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_IPC_SOURCE_DIRS += $(BFC_IPC_DIR) 

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_IPC_LIBRARIES = 

    BFC_OBJECTS += $(BFC_IPC_OBJECTS)
 
    BFC_COMPILER_DEFINES += $(BFC_IPC_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_IPC_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_IPC_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_IPC_LIBRARIES)


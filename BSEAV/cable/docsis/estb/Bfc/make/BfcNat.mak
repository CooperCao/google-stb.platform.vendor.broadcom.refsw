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
#  $Id: BfcNat.mak 1.6 2006/09/08 00:11:01Z jmcqueen-atl Release $
#
#  Filename:       BfcNat.mak
#  Author:         John McQueen
#  Creation Date:  November 24, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build components for Nat.
#
##############
#
# External Variables (input/output):
#
#	BFC_OBJECTS - the list of object files that need to be built for the
#		      project based on the current configuration.
#
#	BFC_COMPILER_DEFINES - the list of definitions that need to be passed
#			       to the compiler.  This may be deprecated in the
#			       future, in favor of just using BfcSystemConfig.h.
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

BFC_NAT_DIR = $(BFC_RESIDENTIAL_GATEWAY_DIR)/Nat


##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.




ifeq ($(BCM_NAT_SUPPORT), 1)
    
BFC_NAT_OBJECTS += \
        NatIpList.o \
        NatPortForwardingList.o \
        NatRgAPI.o \
        NatUtilc.o \
        NatUtil.o \
        NatInfo.o \
        NatTcpUdp.o \
        NatExpectationTable.o \
        NatIcmp.o \
        NatAlgList.o \
        NatFragmentationList.o \
        NatTimerThread.o \
        NatAlgApis.o \
        NatGeneric.o \
        NatAlgFtp.o  \
        NatAlgtftp.o \
        NatAlgIcq.o \
        NatAlgIrc.o \
        NatAlgIke.o \
        NatUnknownDirection.o \
        NatPortTriggerList.o \
        NatAlgH323.o \
        NatAlgRsvp.o \
        NatFilterList.o \
        NatAlgMsn.o \
        NatAlgPptp.o \
        NatAlgNet2Phone.o \
        NatAlgRtsp.o \
        NatAlgSip.o \
        NatCablehomeAPI.o \
        NatPacketFilterHook.o
endif        


ifeq ($(BCM_LANNATSNOOP_SUPPORT), 1)

    BFC_NAT_OBJECTS += lanNatSnoop.o LanNatIpServiceAppIf.o

endif
                   

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.



ifeq ($(BCM_NAT_SUPPORT), 1)

BFC_NAT_COMPILER_DEFINES += BCM_NATSTENTRY_SUPPORT=1 BCM_BROADCOM_NAT=1

endif


ifeq ($(BCM_LANNATSNOOP_SUPPORT), 1)

    BFC_NAT_COMPILER_DEFINES += BCM_LANNATSNOOP_SUPPORT=1

endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_NAT_INCLUDE_DIRS = $(BFC_NAT_DIR) 


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_NAT_SOURCE_DIRS = $(BFC_NAT_DIR)


##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.


##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

BFC_OBJECTS += $(BFC_NAT_OBJECTS) 

BFC_INCLUDE_DIRS += $(BFC_NAT_INCLUDE_DIRS)

BFC_SOURCE_DIRS += $(BFC_NAT_SOURCE_DIRS)

BFC_COMPILER_DEFINES += $(BFC_NAT_COMPILER_DEFINES)



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
#  Filename:       BfcEstbDhcpV6.mak
#  Author:         Pinar Taskiran
#  Creation Date:  June 11, 2008
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the ESTB subsystem, which
#	are common to all ESTB platforms. 
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
# Paths - definitions of relative paths for this component.  These are used
#         below, and should be defined once, rather than using the actual
#         value in multiple places.
#
#         Note that the path needs to be specified relative to the "mount point"
#         of the parent project, not relative to the output directory.  The
#         make processing will specify the rest of the path for you.

BFC_ESTB_DHCPV6_LIBDIB_DIR = Bfc/LibSupport/dibbler
BFC_ESTB_DHCPV6_LIBPCAP_DIR = Bfc/LibSupport/libpcap

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.

BFC_ESTB_DHCPv6_OBJECTS += GenericDhcpV6ClientIf.o \
                           IpHelperDhcpV6ClientEventACT.o \
			   CmDhcpV6MsgACT.o DhcpV6SocketIf.o
                         
BFC_ESTB_DHCPv6_LIB_SUPPORT_OBJECTS = addrpack.o ICMPv6SnoopThread.o

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.
  
BFC_ESTB_DHCPV6_DEFINES += BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT=1

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_ESTB_DHCPv6_INCLUDE_DIRS += $(BFC_ESTB_DHCPV6_LIBDIB_DIR) \
			      $(BFC_ESTB_DHCPV6_LIBPCAP_DIR) 

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_ESTB_DHCPv6_SOURCE_DIRS += $(BFC_ESTB_DHCPV6_LIBDIB_DIR)

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

# if no local enviroinment variable defines the location of the CxC libraries, 
# then use the default libraries in the BFC distribution

BFC_ESTB_DHCPv6_LIBRARIES =  $(BFC_ESTB_DHCPV6_LIBDIB_DIR)/libLowLevel.a \
			     $(BFC_ESTB_DHCPV6_LIBPCAP_DIR)/libpcap.a

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add DHCPv6 components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_DHCPV6_SUPPORT), 1)

	BFC_OBJECTS += $(BFC_ESTB_DHCPv6_OBJECTS) $(BFC_ESTB_DHCPv6_LIB_SUPPORT_OBJECTS)

	BFC_COMPILER_DEFINES += $(BFC_ESTB_DHCPV6_DEFINES)

	BFC_INCLUDE_DIRS += $(BFC_ESTB_DHCPv6_INCLUDE_DIRS)

	BFC_SOURCE_DIRS += $(BFC_ESTB_DHCPv6_SOURCE_DIRS)

	BFC_LIBRARIES += $(BFC_ESTB_DHCPv6_LIBRARIES)

	BFC_LIBRARIES_ABS_PATH += 
endif

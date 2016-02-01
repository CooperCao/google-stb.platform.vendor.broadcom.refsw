##############################################################################
#
#  Copyright (c) 2004  Broadcom Corporation
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
#  Filename:       BfcVpn.mak
#  Author:         Rich Schwartz
#  Creation Date:  March 8, 2004
#
##############################################################################
#  Description:
#
# 	This file contains the build components for UPnP.
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

ifeq ($(BCM_VPN_SUPPORT), 1)
BFC_VPN_INC =  XpressVPN/ike
endif

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

ifeq ($(BCM_VPN_SUPPORT), 1)
BFC_VPN_DIR =  Bfc/IpHelpers/Vpn
endif



##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

ifeq ($(BCM_VPN_SUPPORT), 1)

ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
BFC_VPN_OBJECTS +=  VpnCommandTable.o \
                    VpnNonVolSettingsCommandTable.o 
endif	     

BFC_VPN_OBJECTS +=   \
        IkeCmDocsisStatusClientACT.o \
        IkeThread.o \
        VpnIpServiceAppIf.o \
        VpnNonVolSettings.o \
        XpressVpnDecapsulateSnoop.o \
        XpressVpnDecapsulateSnoop.o \
        XpressVpnEncapsulateSnoop.o           
endif   
   
##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.
ifeq ($(BCM_VPN_SUPPORT), 1)
BFC_VPN_COMPILER_DEFINES = BCM_VPN_SUPPORT=1 

# if the 5365 is the platform, enable hardware assist
ifeq ($(BCM_VPN_SUPPORTHARDWARE), 1)
BFC_VPN_COMPILER_DEFINES += BCM_5365_IPSEC_HWACCEL_SUPPORT=1 
else
BFC_VPN_COMPILER_DEFINES += BCM_5365_IPSEC_HWACCEL_SUPPORT=0
endif

endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.



##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.



##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.
ifeq ($(BCM_VPN_SUPPORT), 1)

# if the 5365 is the platform, then use the hardware assist versions of the libraries
ifeq ($(BCM_VPN_SUPPORTHARDWARE), 1)
ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
        BFC_CABLEHOME_LIBRARIES += Bfc/LibSupport/Vpn/libXpressIpsecEcos5365_098.a
else 
        BFC_CABLEHOME_LIBRARIES += Bfc/LibSupport/Vpn/libXpressIpsecEcos5365.a
endif        
		BFC_CABLEHOME_LIBRARIES += Bfc/LibSupport/Vpn/lib5365SecurityAccelEcos.a
else
ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
        BFC_CABLEHOME_LIBRARIES += Bfc/LibSupport/Vpn/libXpressIpsecEcos_098.a
else
        BFC_CABLEHOME_LIBRARIES += Bfc/LibSupport/Vpn/libXpressIpsecEcos.a
endif        
endif

endif

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

BFC_OBJECTS += $(BFC_VPN_OBJECTS) 

BFC_INCLUDE_DIRS += $(BFC_VPN_DIR)  $(BFC_VPN_INC)

BFC_SOURCE_DIRS += $(BFC_VPN_DIR)

BFC_COMPILER_DEFINES += $(BFC_VPN_COMPILER_DEFINES)  






##############################################################################
#
#  Copyright (c) 2003-2004  Broadcom Corporation
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
#  $Id: Bfc.mak 1.34 2006/10/26 21:44:42Z msieweke Release $
#
#  Filename:       Bfc.mak
#  Author:         David Pullen
#  Creation Date:  March 28, 2003
#
##############################################################################
#  Description:
#
# 	This is the main BFC makefile that brings all the individual component
#	makefiles together.  It just simplifies the main makefile, since it
#	doesn't have to include or deal with these individual files.
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
# NOTE: This file is geared toward the GNU Make utility, and may use any
#       syntax that is valid for recent-vintage GNU Make.
#
##############################################################################


##############################################################################
#
# Includes - include other makefiles here (if necessary).  You must specify
#	     the full path to the makefile, using either BFC_ROOT_DIR or
#	     BFC_MAKE_DIR as appropriate.

include $(BFC_MAKE_DIR)/BfcCore.mak
include $(BFC_MAKE_DIR)/BfcMessageLogging.mak
include $(BFC_MAKE_DIR)/BfcOsWrapper.mak
include $(BFC_MAKE_DIR)/BfcConsole.mak
include $(BFC_MAKE_DIR)/BfcTelnetConsole.mak
include $(BFC_MAKE_DIR)/BfcNonvol.mak
include $(BFC_MAKE_DIR)/BfcNetworking.mak
include $(BFC_MAKE_DIR)/BfcHomePlugNetworking.mak
include $(BFC_MAKE_DIR)/BfcTod.mak
include $(BFC_MAKE_DIR)/BfcHeapManager.mak
include $(BFC_MAKE_DIR)/BfcBattery.mak
include $(BFC_MAKE_DIR)/BfcParentalControl.mak
include $(BFC_MAKE_DIR)/BfcThermalMonitor.mak

ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)
        include $(BFC_MAKE_DIR)/BfcSnmp.mak
        include $(BFC_MAKE_DIR)/BfcFactory.mak 
        include $(BFC_MAKE_DIR)/BfcTls.mak
        include $(BFC_MAKE_DIR)/BfcIPC.mak
endif
        




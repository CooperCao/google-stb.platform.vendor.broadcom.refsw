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
#  $Id: BfcUsbNetworking.mak 1.9 2006/05/18 17:20:27Z karthikb Release $
#
#  Filename:       BfcUsbNetworking.mak
#  Author:         David Pullen
#  Creation Date:  November 5, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the USB interface part
#       of the Networking subsystem.
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

BFC_USB_DIR = $(BFC_NETWORKING_DIR)/Usb


##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

BFC_USB_NETWORKING_OBJECTS = UsbNiHalIf.o

BFC_USB_NETWORKING_CONSOLE_OBJECTS = UsbHalCommandTable.o

BFC_USB_NETWORKING_SNMP_OBJECTS = usbMib.o usbBridge.o

BFC_USB_NETWORKING_BRCM_RUNTIME_SNMP_OBJECTS = 

BFC_USB_NETWORKING_BRCM_FACTORY_SNMP_OBJECTS = brcmUsbFactoryBridge.o \
					       brcmUsbFactoryMib.o
ifeq ($(BFC_USB20_INCLUDED),1)

ifeq ($(BFC_INTERNAL_USB20),1)
BFC_NETWORKING_DRIVERS_DIR += $(BFC_NETWORKING_BSP_COMMON_DIR)/nihal/InternalUSB20
BFC_NETWORKING_INCLUDE_DIRS += $(BFC_NETWORKING_BSP_COMMON_DIR)/nihal/InternalUSB20
BFC_USB_NETWORKING_DRIVER_OBJECTS = Usb20_NiDriverEntry.o Usb20_NiInfo.o \
                                    Usb20_NiInit.o Usb20_NiMode.o \
				    Usb20_NiRcv.o Usb20_NiSend.o \
				    Usb20_CtrlRcv.o Usb20_CtrlSend.o
else
BFC_NETWORKING_DRIVERS_DIR += $(BFC_NETWORKING_BSP_COMMON_DIR)/nihal/ExternalUSB20
BFC_NETWORKING_INCLUDE_DIRS += $(BFC_NETWORKING_BSP_COMMON_DIR)/nihal/ExternalUSB20
BFC_USB_NETWORKING_DRIVER_OBJECTS = Usb20_NiDriverEntry.o Usb20_NiInfo.o \
                                    Usb20_NiInit.o Usb20_NiMode.o Usb20_NiRcv.o \
				    Usb20_NiSend.o \
                    USB20_Loopback.o \
                    USB20_NcDevice.o \
                    USB20_NcFwApi.o \
                    USB20_NcHal.o \
                    USB20_System.o \
                    USB20_NcDebug.o
endif

else
BFC_USB_NETWORKING_DRIVER_OBJECTS = UsbNiDriverEntry.o UsbNiInfo.o \
                                    UsbNiInit.o UsbNiMode.o UsbNiRcv.o \
				    UsbNiSend.o
endif                    
                    
                    

ifeq ($(BFC_INCLUDE_USB_RNDIS_SUPPORT), 1)

ifeq ($(BFC_USB20_INCLUDED),1)
BFC_USB_NETWORKING_DRIVER_OBJECTS += Usb20_NiDescriptors.o Usb20_NiRndis.o
else
BFC_USB_NETWORKING_DRIVER_OBJECTS += UsbNiDescriptors.o UsbNiRndis.o
endif
endif

# The driver files are only used for some target Operating Systems (although it
# really follows the target, not the OS).
BFC_USB_NETWORKING_VxWorks_DRIVER_OBJECTS = $(BFC_USB_NETWORKING_DRIVER_OBJECTS)
BFC_USB_NETWORKING_Win32_DRIVER_OBJECTS = 
BFC_USB_NETWORKING_Ecos_DRIVER_OBJECTS = $(BFC_USB_NETWORKING_DRIVER_OBJECTS)
BFC_USB_NETWORKING_Qnx6_DRIVER_OBJECTS = $(BFC_USB_NETWORKING_DRIVER_OBJECTS)


##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_USB_NETWORKING_DEFINES = USB_HAL_INCLUDED=1	\
                             BFC_INCLUDE_USB_SUPPORT=$(BFC_INCLUDE_USB_SUPPORT) \
                             BFC_USB20_INCLUDED=$(BFC_USB20_INCLUDED) \
                             BFC_INTERNAL_USB20=$(BFC_INTERNAL_USB20)


##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_USB_NETWORKING_INCLUDE_DIRS = $(BFC_USB_DIR)
                              

##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_USB_NETWORKING_SOURCE_DIRS = $(BFC_USB_DIR)
                             

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

BFC_USB_NETWORKING_LIBRARIES = 


##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Note that I add my local files to my parent's macros rather than the global
# macros; my stuff should only be added if top level networking support is
# also enabled, and the parent makefile handles this.
ifeq ($(BFC_INCLUDE_USB_SUPPORT), 1)

    # Core networking modules
    BFC_NETWORKING_OBJECTS += $(BFC_USB_NETWORKING_OBJECTS)

    # Console modules
    BFC_NETWORKING_CONSOLE_OBJECTS += $(BFC_USB_NETWORKING_CONSOLE_OBJECTS)

    # SNMP modules
    BFC_NETWORKING_SNMP_OBJECTS += $(BFC_USB_NETWORKING_SNMP_OBJECTS)
    
    # Runtime and Factory MIB support modules.
    BFC_NETWORKING_BRCM_RUNTIME_SNMP_OBJECTS += $(BFC_USB_NETWORKING_BRCM_RUNTIME_SNMP_OBJECTS)
    BFC_NETWORKING_BRCM_FACTORY_SNMP_OBJECTS += $(BFC_USB_NETWORKING_BRCM_FACTORY_SNMP_OBJECTS)

    # Driver modules
    BFC_NETWORKING_DRIVER_$(BFC_TARGET_OS)_SUPPORT_OBJECTS += $(BFC_USB_NETWORKING_$(BFC_TARGET_OS)_DRIVER_OBJECTS)


    BFC_NETWORKING_DEFINES += $(BFC_USB_NETWORKING_DEFINES)

    BFC_NETWORKING_INCLUDE_DIRS += $(BFC_USB_NETWORKING_INCLUDE_DIRS)

    BFC_NETWORKING_SOURCE_DIRS += $(BFC_USB_NETWORKING_SOURCE_DIRS)

    BFC_NETWORKING_LIBRARIES += $(BFC_USB_NETWORKING_LIBRARIES)

endif

                                                                                     

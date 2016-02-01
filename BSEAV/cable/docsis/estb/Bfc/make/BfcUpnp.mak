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
#  $Id: BfcUpnp.mak 1.17 2006/09/20 19:43:29Z jmcqueen-atl Release $
#
#  Filename:       BfcUpnp.mak
#  Author:         John McQueen
#  Creation Date:  November 24, 2003
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

BFC_UPNP_DIR = $(BFC_RESIDENTIAL_GATEWAY_DIR)/Upnp

BFC_UPNP_IGD_DIR += $(BFC_UPNP_DIR)/Igd

BFC_UPNP_QOS_DIR += $(BFC_UPNP_DIR)/Qos


##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.


BFC_UPNP_OBJECTS += \
    UpnpThread.o \
    UpnpSnoop.o \
    UpnpIf.o \
    UpnpDevice.o \
    UpnpSpecVersion.o \
    HttpUpnpAgent.o \
    UpnpDeviceList.o \
    UpnpService.o \
    UpnpServiceList.o \
    ServiceAction.o \
    ServiceActionList.o \
    ServiceArgument.o \
    ServiceArgumentList.o \
    ServiceStateTable.o \
    ServiceStateVariable.o \
    AllowedValueRange.o \
    AllowedValue.o \
    AllowedValueList.o \
    UpnpServiceBridge.o  \
    UpnpServiceBridgeDatabase.o \
    UpnpServiceSubscription.o \
    UpnpServiceSubscriptionList.o \
    UpnpDiscoveryHttpHeader.o \
    UpnpDiscoveryDevice.o \
    UpnpDiscoveryDeviceList.o \
    UpnpIfManager.o \
    UpnpRootDevice.o \
    UpnpErrorCode.o \
    UpnpNatIf.o \
    UpnpCableHomeStatusClientACT.o
    
ifeq ($(BCM_CABLEHOME_SUPPORT),1)
    BFC_UPNP_OBJECTS += UpnpIpServiceAppIf.o
    
    BFC_UPNP_CONSOLE_OBJECTS += UpnpCommandTable.o
endif

    
              
BFC_UPNP_IGD_OBJECTS += \
    InternetGatewayDevice_1.o \
    InternetGatewayDeviceIf.o  \
    WanDevice.o \
    WanConnectionDevice.o \
    Layer3Forwarding.o \
    WanCommonInterfaceConfig.o \
    WanIpConnection.o \
    Layer3ForwardingServiceBridge.o \
    WanCommonInterfaceConfigBridge.o \
    WanIpConnectionBridge.o \
    IgdUpnpErrorCodes.o
    
BFC_UPNP_QOS_OBJECTS += \
    QosManagerService.o \
    QosManagerServiceBridge.o \
    QosPolicyHolderService.o \
    QosPolicyHolderServiceBridge.o \
    QosDeviceIf.o \
    QosBasicDevice.o \
    QosBasicDeviceList.o \
    QosTrafficPolicy.o \
    QosTrafficPolicyTable.o \
    QosTrafficId.o \
    QosTrafficDescriptor.o \
    QosTrafficSpec.o \
    QosTrafficSpecList.o \
    QosDeviceService.o \
    QosDeviceServiceBridge.o \
    QosDeviceState.o \
    QosDeviceCapabilities.o \
    QosDevicePathInformation.o \
    QosDeviceInformation.o \
    QosDeviceInformationInterface.o \
    QosTrafficDescriptorsPerInterface.o \
    QosAnyTextObjectList.o \
    QosPolicyHolderDevice.o \
    QosUpnpErrorCodes.o
    
     
                   

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_UPNP_DEFINES += BCM_UPNP_SUPPORT=1 


ifeq ($(BCM_UPNP_QOS_SUPPORT), 1)

        BFC_UPNP_DEFINES += BCM_UPNP_QOS_SUPPORT=1
         
endif




##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_UPNP_INCLUDE_DIRS = $(BFC_UPNP_DIR) $(BFC_UPNP_IGD_DIR) $(BFC_UPNP_QOS_DIR)


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_UPNP_SOURCE_DIRS = $(BFC_UPNP_DIR) $(BFC_UPNP_IGD_DIR) $(BFC_UPNP_QOS_DIR)


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


ifeq ($(BCM_UPNP_SUPPORT), 1)

        BFC_OBJECTS += $(BFC_UPNP_OBJECTS) $(BFC_UPNP_IGD_OBJECTS)

        # These are included only in fat images.
        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)
        
        BFC_OBJECTS += $(BFC_UPNP_CONSOLE_OBJECTS)
        
        endif
 

        ifeq ($(BCM_UPNP_QOS_SUPPORT), 1)
        
            BFC_OBJECTS += $(BFC_UPNP_QOS_OBJECTS)
        
        endif
        
        BFC_COMPILER_DEFINES += $(BFC_UPNP_DEFINES)
        
        BFC_INCLUDE_DIRS += $(BFC_UPNP_INCLUDE_DIRS)
        
        BFC_SOURCE_DIRS += $(BFC_UPNP_SOURCE_DIRS)

endif




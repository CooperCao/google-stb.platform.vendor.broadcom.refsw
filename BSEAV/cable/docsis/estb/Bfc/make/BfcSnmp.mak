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
#  $Id: BfcSnmp.mak 1.53 2007/05/15 21:00:30Z koneal Release $
#
#  Filename:       BfcSnmp.mak
#  Author:         Kevin O'Neal
#  Creation Date:  Sept 10, 2003
#
##############################################################################
#  Description:
#
# 	This file contains the build "stuff" for the SNMP subsystem.  This
#	component can be completely removed, or can be completely present; it
#	also has optional console support, which can be removed or added.
#	Different application components can optionally add their own SNMP
#	objects.
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

BFC_SNMP_DIR = Bfc/IpHelpers/Snmp

# Directories for agent cores.

# If the agent core has not been defined, then assign it based on OS.
# vxWorks: Net-SNMP
# eCos: Net-SNMP
# Other: stub
ifeq ($(BFC_AGENT_CORE),)
    BFC_AGENT_CORE = STUBAGENT
    ifeq ($(BFC_TARGET_OS), VxWorks)
        BFC_AGENT_CORE = NETSNMP
    endif
    ifeq ($(BFC_TARGET_OS), Ecos)
        BFC_AGENT_CORE = NETSNMP
    endif
    ifeq ($(BFC_TARGET_OS), Qnx6)
        BFC_AGENT_CORE = NETSNMP
    endif
endif
# Verify legal agent values...
ifneq ($(BFC_AGENT_CORE), NETSNMP)
    ifneq ($(BFC_AGENT_CORE), STUBAGENT)
        junk := $(shell echo "Error: BFC_AGENT_CORE ($(BFC_AGENT_CORE)) not defined correctly!" 1>&2)
        bogus
    endif
endif

# Stub agent - no actual agent, just stubs out APIs for buildability.
BFC_STUBAGENT_DIRS = $(BFC_SNMP_DIR)/Stubs

# NetSNMP
BFC_NETSNMP_DIRS = $(BFC_SNMP_DIR)/NetSnmp Bfc/LibSupport/NetSnmp/NetSnmp/include

##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.  These must all have .o as the
#		 file extension.  The top level make processing will change it
#		 if necessary.

#
# Here are the objects in the V2 Snmp.mk file.  This is what we're working
# towards.  As objects are added to this makefile, I remove them from this
# list...
#
# StubHalIf.o
#
# ifMib.o 
# ifBridge.o
# dot1dMib.o 
# dot1dBridge.o
# etherLikeMib.o 
# etherLikeBridge.o
# usbMib.o 
# usbBridge.o
#
# NetToMediaSnoop.o 
# IfUnknownProtosSnoop.o 
#
# SLog.o
#

BFC_SNMP_OBJECTS = ObjectIdentifier.o VariableBinding.o BerObjs.o \
                   MibObjs.o MibObjList.o MibBridge.o SnmpPdu.o \
                   SnmpApi.o BcmSnmpAgent.o SnmpV3Agent.o SnmpThread.o \
                   SnmpAgentIpStackEventACT.o \
                   BcmSyslog.o BcmTrap.o standardTrap.o SnmpTrapThread.o \
                   snmpV2Mib.o snmpV2Bridge.o SLog.o 
                   
BFC_SNMPV3_OBJECTS = snmpDH.o DhKickstartThread.o \
                     communityMib.o communityBridge.o \
                     usmDhMib.o usmDhBridge.o SecCryptoHelper.o
                     
BFC_BRCM_RUNTIME_OBJECTS = brcmSnmpMgmtMib.o brcmSnmpMgmtBridge.o \
                           brcmBfcMgmtMib.o brcmBfcMgmtBridge.o

BFC_BRCM_FACTORY_OBJECTS = 

# Optional console support files.
BFC_SNMP_CONSOLE_OBJECTS = SnmpCommandTable.o BcmSnmpAgentConsole.o \
                           SnmpV3AgentConsole.o

BFC_VxWorks_SNMP_OBJECTS = vxIpBridge.o vxUdpBridge.o

BFC_Win32_SNMP_OBJECTS = win32IpBridge.o

BFC_Linux_SNMP_OBJECTS = lnxIpBridge.o lnxUdpBridge.o

BFC_Ecos_SNMP_OBJECTS = ecosIpBridge.o ecosUdpBridge.o ecosUdpPort0Snoop.o

BFC_Qnx6_SNMP_OBJECTS = qnxIpBridge.o qnxUdpBridge.o

# These objects are temporary, only here to serve as stubs until the real
# objects are in place.  For BFC interim development.
BFC_STUBAGENT_OBJECTS = BfcSnmpStubs.o
BFC_STUBAGENT_V3_OBJECTS =

# Objects for NetSNMP agent core
BFC_NETSNMP_OBJECTS = AgentUtils.o BcmV2Glue.o BrcmDomain.o NetsnmpAgnt.o 
BFC_NETSNMP_V3_OBJECTS = Kickstart.o NotifyAgnt.o VacmAgnt.o AgentCoreHelper.o \
                         snmpNotificationMib.o snmpNotificationBridge.o \
                         snmpTargetMib.o snmpTargetBridge.o \
                         snmpUsmMib.o snmpUsmBridge.o \
                         snmpVacmMib.o snmpVacmBridge.o \
                         snmpGroupBridge.o

ifeq ($(BFC_INCLUDE_IPV6_SUPPORT), 1)
BFC_NETSNMP_OBJECTS += BrcmDomainV6.o
endif                      

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

BFC_SNMP_DEFINES = SNMP_SUPPORT=3 \
                   BFC_INCLUDE_SNMP_SUPPORT=$(BFC_INCLUDE_SNMP_SUPPORT) \
		   BFC_INCLUDE_SNMP_CONSOLE_SUPPORT=$(BFC_INCLUDE_SNMP_CONSOLE_SUPPORT) \
                   BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT=${BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT} \
                   BCM_FACTORY_SUPPORT=$(BFC_INCLUDE_BRCM_FACTORY_MIB_SUPPORT) \
                   BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT=${BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT} \
                   BCM_RUNTIMEMIB_SUPPORT=${BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT} \
                   BFC_SUPPORT_SNMP_OBJECT_NAMES=${BFC_SUPPORT_SNMP_OBJECT_NAMES}

# BFC only supports NetSNMP; this turns off WindManage/Epilogue support for
# VxWorks (and possibly others).
BFC_SNMP_DEFINES += SNMP_NETSNMP=1

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BFC_SNMP_INCLUDE_DIRS = $(BFC_SNMP_DIR)

# Add the directory/directories for our agent core
BFC_SNMP_INCLUDE_DIRS += $(BFC_$(BFC_AGENT_CORE)_DIRS)


##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BFC_SNMP_SOURCE_DIRS = $(BFC_SNMP_DIR)

# Add the directory/directories for our agent core
BFC_SNMP_SOURCE_DIRS += $(BFC_$(BFC_AGENT_CORE)_DIRS)


##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.

NETSNMP_OS = $(BFC_TARGET_OS)
ifeq ($(BFC_TARGET_OS),Ecos)
    NETSNMP_OS = ecos
endif


ifeq ($(BFC_INCLUDE_OPENSSL098), 1)
	BFC_NETSNMP_LIBRARIES = Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmp_$(NETSNMP_OS)_openssl098.a \
                                Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmpagent_$(NETSNMP_OS).a \
                                Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmphelpers_$(NETSNMP_OS).a \
                                Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmpmibs_$(NETSNMP_OS).a
else
	BFC_NETSNMP_LIBRARIES = Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmp_$(NETSNMP_OS).a \
                                Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmpagent_$(NETSNMP_OS).a \
                                Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmphelpers_$(NETSNMP_OS).a \
                                Bfc/LibSupport/NetSnmp/NetSnmp/libnetsnmpmibs_$(NETSNMP_OS).a
endif

				

BFC_SNMP_LIBRARIES = $(BFC_$(BFC_AGENT_CORE)_LIBRARIES)

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add Snmp components, if included by BfcSystemConfig.mak.
ifeq ($(BFC_INCLUDE_SNMP_SUPPORT), 1)

    # BFC, agent core, and OS/Target specific SNMP modules
    BFC_OBJECTS += $(BFC_SNMP_OBJECTS) $(BFC_$(BFC_AGENT_CORE)_OBJECTS) $(BFC_$(BFC_TARGET_OS)_SNMP_OBJECTS)
    
    # For now, unconditionally support SNMPv3.  Do we need to support just
    # SNMPv1/v2c any more?
    BFC_OBJECTS += $(BFC_SNMPV3_OBJECTS) $(BFC_$(BFC_AGENT_CORE)_V3_OBJECTS)
    
    # Add Snmp console components, if support for Console and SNMP Console
    # are both included.
    ifeq ($(BFC_INCLUDE_CONSOLE_SUPPORT), 1)
    
        ifeq ($(BFC_CONFIG_MINIMAL_CONSOLE), 0)
    
            ifeq ($(BFC_INCLUDE_SNMP_CONSOLE_SUPPORT), 1)

                BFC_OBJECTS += $(BFC_SNMP_CONSOLE_OBJECTS)

            endif
	
        endif
    
    endif
    
    # Add BRCM private MIB modules, if we have support for BRCM private
    # MIBs
    ifeq ($(BFC_INCLUDE_BRCM_RUNTIME_MIB_SUPPORT), 1)
    
        BFC_OBJECTS += $(BFC_BRCM_RUNTIME_OBJECTS)
	
    endif

    BFC_COMPILER_DEFINES += $(BFC_SNMP_DEFINES)

    BFC_INCLUDE_DIRS += $(BFC_SNMP_INCLUDE_DIRS)
    
    BFC_SOURCE_DIRS += $(BFC_SNMP_SOURCE_DIRS)
    
    BFC_LIBRARIES += $(BFC_SNMP_LIBRARIES)

endif


#############################################################################
#
# Copyright (c) 2009 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
# AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
# SOFTWARE.  
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################
#
#  Filename:       DsgCC.mak
#  Author:         Pinar Taskiran Cyr
#  Creation Date:  Jan 11, 2009
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

BCM_DSGCC_DIR = StbHostApps/DsgCC  
BCM_DSGCC_APP_DIR = StbHostApps/DsgCC/DsgCC
#STB_DSG_DIR2 = ../../../DsgCC

#STB_DOCSIS_API_OS_DIR =  ../../../Stb/StbDocsisApi/$(TARGETOS)/

#STB_RI_DIR = ../../../DsgCC/RemoteInterface
STB_CLIENT_DIR = StbHostApps/DsgCC/DsgCC/ClientInterface
STB_DOCSIS_DIR  = Docsis
STB_DOCSIS_STB_DIR  = Stb
STB_DSG_COMMON  = $(STB_DOCSIS_STB_DIR)/Dsg
STB_DSG_API = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/Dsg
STB_DSG_OS = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/$(TARGETOS)
#STB_BSP_COMMON_SRC  = ../../../../../cm_bsp_v2/bsp_common/common_src
#STB_BSP_COMMON_INC  = ../../../../../cm_bsp_v2/bsp_common/common_inc

DSGCC_SHM_DIR = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/SharedMemory


##############################################################################
#
# Object files - One or more lists of object files that are built with this
#                component.  The level of granularity depends on the build
#                options that are supported.
		
BCM_DSGCC_OBJECTS +=	DsgccApplication.o \

BCM_DSGCC_OBJECTS +=  DcdMsg.o \
DsgClassifierSettings.o DsgClient.o DsgClientCtlInit.o DsgClientCtlThread.o \
DsgClientId.o DsgClientManager.o DsgClientRegistrar.o  \
DsgConfigSettings.o DsgEstbTunnelSettings.o \
DsgEstbTunnelSettingsList.o DsgTunnelSettings.o ErrorSet.o \
IpClassifierSettings.o \
Type43VendorSettings.o  TLVCodes.o 


ifeq ($(BCM_CONSOLE_SUPPORT),1) 
	BCM_DSGCC_OBJECTS += 	DsgCommandTable.o \

endif

ifeq ($(DSG_RPC_INCLUDED),1)
	BCM_DSGCC_OBJECTS	+=	dsgccClientRegistration_rpcServer_Stub.o \
				dsgccClientRegistration_rpc_xdr.o \
				dsgccClientCallback_rpcClient.o \
				dsgccClientCallback_rpc_xdr.o \
				dsgccClientRegistration_rpcServer.o
else
	BCM_DSGCC_OBJECTS +=   	dsgcc_clnt_if.o 
endif

ifeq ($(DCAS_BT_SUPPORT),1)
	BCM_DSGCC_OBJECTS += DsgBroadcastTunnelHeader.o DsgBroadcastTunnelPktReassembler.o
endif

ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC),1)
	BCM_DSGCC_OBJECTS +=   	EstbRemoteIfDataHandlerThread.o \
				EstbRemoteInterfaceDataStub.o \
				BcmSharedMemory.o
else
	ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
	BCM_DSGCC_OBJECTS += BcmSharedMemory.o
	endif
endif

ifeq ($(BCM_DUAL_DOWNLOAD_SUPPORT),1)
	BCM_DSGCC_OBJECTS	+=	DsgDownloadCtlThread.o \
				DsgTftpServer.o
endif

ifeq ($(BCM_NONVOL_SUPPORT),1)
	BCM_DSGCC_OBJECTS     +=  DsgccNonVolSettings.o \
			       EstbTestNonVolSettings.o

        ifeq ($(BCM_CONSOLE_SUPPORT),1)
		BCM_DSGCC_OBJECTS     +=  DsgccNonVolSettingsCommandTable.o \
				       EstbTestNonVolSettingsCommandTable.o
        endif
endif

ifeq ($(CABLECARD_SUPPORT),1)
        BCM_DSGCC_OBJECTS     += DsgccCableCardIf.o DsgCableCardClient.o
endif

##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.

ifeq ($(BCM_DSG_DUAL_PROCESSOR_INTERFACE),1)
	BCM_DSGCC_DEFINES    += BCM_DSG_DUAL_PROCESSOR_INTERFACE

	ifeq ($(BCM_DUAL_DOWNLOAD_SUPPORT),1)
		BCM_DSGCC_DEFINES    += BCM_DUAL_DOWNLOAD_SUPPORT
	endif

	# eCM-eSTB communication method in DUAL CPU design
	ifeq ($(USE_PRIVATE_NETWORK),1)
		BCM_DSGCC_DEFINES    += USE_PRIVATE_NETWORK
	else
		BCM_DSGCC_DEFINES    += USE_MULTICAST_SOCKET
	endif
endif

ifeq ($(BCM_DSG_DUAL_PROCESS_INTERFACE),1)
	BCM_DSGCC_DEFINES    += BCM_DSG_DUAL_PROCESS_INTERFACE
	ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC), 1)
	    BCM_DSGCC_DEFINES += BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC=1
	endif
endif

ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
        BCM_DSGCC_DEFINES += BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC=1
endif

ifeq ($(CLIENT_CONTROLLER_SUPPORT),1)
	BCM_DSGCC_DEFINES    += CLIENT_CONTROLLER_SUPPORT=1
endif

ifeq ($(CLIENT_INTERFACE_SUPPORT),1)
	BCM_DSGCC_DEFINES    += CLIENT_INTERFACE_SUPPORT=1
endif

ifeq ($(DSG_RPC_INCLUDED),1)
	BCM_DSGCC_DEFINES    += DSG_RPC_INCLUDED=1
endif

ifeq ($(DCAS_BT_SUPPORT), 1)
        BCM_DSGCC_DEFINES    += DCAS_BT_SUPPORT=1
endif

ifeq ($(CABLECARD_SUPPORT), 1)
        BCM_DSGCC_DEFINES    += CABLECARD_SUPPORT=1
endif

ifeq ($(BCM_CONSOLE_SUPPORT), 1)
        BCM_DSGCC_DEFINES    += BCM_CONSOLE_SUPPORT=1
endif

ifeq ($(BCM_NONVOL_SUPPORT), 1)
        BCM_DSGCC_DEFINES    += BCM_NONVOL_SUPPORT=1
endif

##############################################################################
#
# Include path - This is the list of paths that will be used by make and/or the
#                compiler to find the include files associated with this
#                component.  These are generally passed to the compiler with the
#                -I flag.  Note that you do not specify the -I here; this is
#                done for you by the top level make processing.

BCM_DSGCC_INCLUDE_DIRS += $(BCM_DSGCC_DIR) \
			  $(BCM_DSGCC_APP_DIR) \
			  $(STB_CLIENT_DIR) \
			  $(STB_DOCSIS_DIR) \
			  $(STB_DOCSIS_STB_DIR) \
			  $(STB_DSG_COMMON) \
			  $(STB_DSG_API) \
			  $(STB_DSG_OS) \
			  Bfc/IpHelpers \
			  
ifeq ($(CLIENT_INTERFACE_SUPPORT), 1)
	BCM_DSGCC_INCLUDE_DIRS += $(STB_CLIENT_DIR)/SampleApp/$(LC_TARGETOS) \
				  $(STB_CLIENT_DIR)/$(LC_TARGETOS)
endif


ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC),1)
    BCM_DSGCC_INCLUDE_DIRS += $(DSGCC_SHM_DIR)    
else
    ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)        
        BCM_DSGCC_INCLUDE_DIRS += $(DSGCC_SHM_DIR)        
    endif
endif

ifeq ($(DSG_RPC_INCLUDED),1)
	BCM_DSGCC_INCLUDE_DIRS += $(STB_CLIENT_DIR)/RpcInterface \
			 $(STB_CLIENT_DIR)/RpcInterface/DsgCC \
			 $(STB_CLIENT_DIR)/RpcInterface/DsgClient \
			 $(STB_CLIENT_DIR)/RpcInterface/Common			 
else
	BCM_DSGCC_INCLUDE_DIRS += $(STB_CLIENT_DIR)/FuncInterface
endif

ifeq ($(CABLECARD_SUPPORT),1)
	BCM_DSGCC_INCLUDE_DIRS += $(STB_CLIENT_DIR)/CableCardInterface
endif
			  
##############################################################################
#
# Source path - This is the list of paths that will be used by make and/or the
#               compiler to find the source code (.c, .cpp, etc) associated with
#               this component.

BCM_DSGCC_SOURCE_DIRS +=       $(BCM_DSGCC_DIR) \
                               $(BCM_DSGCC_APP_DIR) \
				  $(STB_CLIENT_DIR) \
				  $(STB_DOCSIS_DIR) \
				  $(STB_DOCSIS_STB_DIR) \
				  $(STB_DSG_COMMON) \
				  $(STB_DSG_API) \
				  $(STB_DSG_OS) \
				  Bfc/IpHelpers \
	  
ifeq ($(CLIENT_INTERFACE_SUPPORT), 1)
    BCM_DSGCC_SOURCE_DIRS += 	$(STB_CLIENT_DIR)/SampleApp/$(LC_TARGETOS) \
				$(STB_CLIENT_DIR)/$(LC_TARGETOS)
endif
      		  
ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC),1)
    BCM_DSGCC_SOURCE_DIRS += 	$(DSGCC_SHM_DIR)
else
    ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)        
        BCM_DSGCC_SOURCE_DIRS += $(DSGCC_SHM_DIR)
    endif
endif

ifeq ($(DSG_RPC_INCLUDED),1)       		 
	BCM_DSGCC_SOURCE_DIRS += $(STB_CLIENT_DIR)/RpcInterface \
				 $(STB_CLIENT_DIR)/RpcInterface/DsgCC \
				 $(STB_CLIENT_DIR)/RpcInterface/DsgClient \
				 $(STB_CLIENT_DIR)/RpcInterface/Common
else
	BCM_DSGCC_SOURCE_DIRS  += $(STB_CLIENT_DIR)/FuncInterface
endif

ifeq ($(CABLECARD_SUPPORT),1)
	BCM_DSGCC_SOURCE_DIRS += $(STB_CLIENT_DIR)/CableCardInterface
endif

##############################################################################
#
# Libraries - This is the list of libraries that need to be used by the linker
#             which are associated with this component.
#
# if no local enviroinment variable defines the location of the CxC libraries, 
# then use the default libraries in the BFC distribution
#
##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add DSGCC components, if included by BfcSystemConfig.mak.
ifeq ($(BCM_DSGCC_INCLUDED), 1)

	BFC_OBJECTS += $(BCM_DSGCC_OBJECTS) 

	BFC_COMPILER_DEFINES += $(BCM_DSGCC_DEFINES)

	BFC_INCLUDE_DIRS += $(BCM_DSGCC_INCLUDE_DIRS) 

	BFC_SOURCE_DIRS += $(BCM_DSGCC_SOURCE_DIRS) 

	BFC_LIBRARIES += 

	BFC_LIBRARIES_ABS_PATH += 
endif

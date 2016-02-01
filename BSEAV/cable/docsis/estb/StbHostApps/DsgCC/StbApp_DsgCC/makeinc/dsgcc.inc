#############################################################################
# (c) 2003-2015 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its
# licensors, and may only be used, duplicated, modified or distributed pursuant
# to the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and Broadcom
# expressly reserves all rights in and to the Software and all intellectual
# property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
#    constitutes the valuable trade secrets of Broadcom, and you shall use all
#    reasonable efforts to protect the confidentiality thereof, and to use
#    this information only in connection with your use of Broadcom integrated
#    circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
#    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
#    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
#    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
#    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
#    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
#    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
#    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
#    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
#    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
#    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
#    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#############################################################################

############################################################################
##                 FLAGS
############################################################################

# DSG-CC build uses some files that are included in DOCSIS Cable Modem build
# as well. BCM_STANDALONE_DSGCC compile flag is used in these files and
# MUST be ON for DSG-CC builds. IT will be OFF for Cable Modem builds.
BCM_STANDALONE_DSGCC=1

ADDED_CFLAGS    += -DBCM_DSGCC_INCLUDED=1

ifeq ($(BCM_DSG_DUAL_PROCESSOR_INTERFACE),1)
	ADDED_CFLAGS    += -DBCM_DSG_DUAL_PROCESSOR_INTERFACE

	ifeq ($(BCM_DUAL_DOWNLOAD_SUPPORT),1)
		ADDED_CFLAGS    += -DBCM_DUAL_DOWNLOAD_SUPPORT
	endif

	# eCM-eSTB communication method in DUAL CPU design
	ifeq ($(USE_PRIVATE_NETWORK),1)
		ADDED_CFLAGS    += -DUSE_PRIVATE_NETWORK
	else
		ADDED_CFLAGS    += -DUSE_MULTICAST_SOCKET
	endif

	ifeq ($(USE_INTERFACE_ETH2),1)
		ADDED_CFLAGS    += -DUSE_INTERFACE_ETH2
	endif

	ifeq ($(DSG_RECOVER_SUPPORT),1)
		ADDED_CFLAGS    += -DDSG_RECOVER_SUPPORT
	endif


endif

ifeq ($(BCM_DSG_DUAL_PROCESS_INTERFACE),1)
	ADDED_CFLAGS    += -DBCM_DSG_DUAL_PROCESS_INTERFACE
	ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC), 1)
	    ADDED_CFLAGS += -DBCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC=1
	endif
endif

ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
        ADDED_CFLAGS += -DBCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC=1
endif

ifeq ($(BCM_STANDALONE_DSGCC),1)
	ADDED_CFLAGS    += -DBCM_STANDALONE_DSGCC
endif

ifeq ($(CLIENT_INTERFACE_SUPPORT),1)
	ADDED_CFLAGS    += -DCLIENT_INTERFACE_SUPPORT=1
endif

ifeq ($(DSG_RPC_INCLUDED),1)
	ADDED_CFLAGS    += -DDSG_RPC_INCLUDED
endif

ifeq ($(BCM_SETTOP_SUPPORT), 1)
	ADDED_CFLAGS    += -DBCM_SETTOP_SUPPORT
endif

ifeq ($(BCM_DSG_API_TEST), 1)
	ADDED_CFLAGS    += -DBCM_DSG_API_TEST=1
endif

ifeq ($(DSGCC_BUILT_AS_LIB), 1)
	ADDED_CFLAGS    += -DDSGCC_BUILT_AS_LIB=1
endif

ifeq ($(CABLECARD_SUPPORT), 1)
        ADDED_CFLAGS    += -DCABLECARD_SUPPORT=1
endif

ifeq ($(DCAS_BT_SUPPORT), 1)
        ADDED_CFLAGS    += -DDCAS_BT_SUPPORT=1
endif

ifeq ($(ECM_STUB_API), 1)
        ADDED_CFLAGS    += -DECM_STUB_API=1
endif

ifeq ($(DSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM), 1)
	ADDED_CFLAGS    += -DDSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM=1
endif

ifeq ($(BFC_INCLUDE_TAGGING_CHECK), 1)
	ADDED_CFLAGS    += -DBFC_INCLUDE_TAGGING_CHECK=1
else
	ADDED_CFLAGS    += -DBFC_INCLUDE_TAGGING_CHECK=0
endif


############################################################################
##                 V2 COMPONENT INCLUDES
############################################################################


############################################################################
##                 PATHS
############################################################################


:: Standalone CM build does not include these directories

  STB_DSG_DIR2 = $(BUILD_DIR)/../../DsgCC

  STB_DOCSIS_API_OS_DIR =  $(BUILD_DIR)/../../../../Stb/StbDocsisApi/$(TARGETOS)/

  STB_RI_DIR = $(BUILD_DIR)/../../DsgCC/RemoteInterface
  STB_CLIENT_DIR = $(BUILD_DIR)/../../DsgCC/ClientInterface
  STB_DOCSIS_DIR  = $(BUILD_DIR)/../../../../Docsis
  STB_DOCSIS_STB_DIR  = $(BUILD_DIR)/../../../../Stb
  STB_DSG_COMMON  = $(STB_DOCSIS_STB_DIR)/Dsg
  STB_DSG_API = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/Dsg
  STB_DSG_OS = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/$(TARGETOS)
  STB_BSP_COMMON_SRC  = $(BUILD_DIR)/../../../../cm_bsp_v2/bsp_common/common_src
  STB_BSP_COMMON_INC  = $(BUILD_DIR)/../../../../cm_bsp_v2/bsp_common/common_inc
  STB_DOCSIS_API = $(STB_DOCSIS_STB_DIR)/StbDocsisApi

  ESTB_COMMON_ECM_INTERFACE = $(BUILD_DIR)/../../../Common/EcmInterface

  EXTRA_INCLUDE  +=  -I$(STB_DSG_DIR2) -I$(STB_RI_DIR) -I$(STB_CLIENT_DIR) \
						-I$(STB_DOCSIS_DIR) -I$(STB_DSG_COMMON) -I$(STB_DSG_API) \
						-I$(STB_DOCSIS_API_OS_DIR) -I$(STB_BSP_COMMON_INC) \
						-I$(STB_DOCSIS_STB_DIR) -I$(STB_DSG_OS) \
						-I$(ESTB_COMMON_ECM_INTERFACE) \
						-I$(STB_DOCSIS_API)

  VPATH          +=  $(STB_DSG_DIR2) $(STB_RI_DIR) $(STB_CLIENT_DIR) \
						$(STB_DOCSIS_DIR) $(STB_DSG_COMMON) $(STB_DSG_API) \
						$(STB_DOCSIS_API_OS_DIR) $(STB_DOCSIS_STB_DIR) \
						$(STB_DSG_OS) $(STB_BSP_COMMON_SRC) \
						$(ESTB_COMMON_ECM_INTERFACE)

ifeq ($(CLIENT_INTERFACE_SUPPORT), 1)
	EXTRA_INCLUDE += -I$(STB_CLIENT_DIR)/SampleApp/$(LC_TARGETOS) \
					 -I$(STB_CLIENT_DIR)/$(LC_TARGETOS)
    VPATH         += $(STB_CLIENT_DIR)/SampleApp/$(LC_TARGETOS) \
					 $(STB_CLIENT_DIR)/$(LC_TARGETOS)
endif

ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC),1)
    DSGCC_SHM_DIR = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/SharedMemory
    EXTRA_INCLUDE += -I$(DSGCC_SHM_DIR)
    VPATH         += $(DSGCC_SHM_DIR)
else
    ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
        DSGCC_SHM_DIR = $(STB_DOCSIS_STB_DIR)/StbDocsisApi/SharedMemory
        EXTRA_INCLUDE += -I$(DSGCC_SHM_DIR)
        VPATH         += $(DSGCC_SHM_DIR)
    endif
endif

ifeq ($(DSG_RPC_INCLUDED),1)
	EXTRA_INCLUDE += -I$(STB_CLIENT_DIR)/RpcInterface \
			 -I$(STB_CLIENT_DIR)/RpcInterface/DsgCC \
			 -I$(STB_CLIENT_DIR)/RpcInterface/DsgClient \
			 -I$(STB_CLIENT_DIR)/RpcInterface/Common

	VPATH         += $(STB_CLIENT_DIR)/RpcInterface \
			 $(STB_CLIENT_DIR)/RpcInterface/DsgCC \
			 $(STB_CLIENT_DIR)/RpcInterface/DsgClient \
			 $(STB_CLIENT_DIR)/RpcInterface/Common
else
	VPATH         += $(STB_CLIENT_DIR)/FuncInterface
	EXTRA_INCLUDE += -I$(STB_CLIENT_DIR)/FuncInterface
endif

############################################################################
##                 OBJECT FILES
############################################################################

DSGCC_OBJS +=	$(TARGETDIR)/CmApplication.o \
		$(TARGETDIR)/CmAppMain.o  \
		$(TARGETDIR)/BcmBasicString.o \
		$(TARGETDIR)/$(OS_PREFIX)Interface.o \
		$(TARGETDIR)/DsgClientCtlInit.o


DSGCC_OBJS +=	$(TARGETDIR)/DsgClientCtlThread.o \
		$(TARGETDIR)/DsgTunnelSettings.o \
		$(TARGETDIR)/DsgClientId.o \
		$(TARGETDIR)/DsgClassifierSettings.o \
		$(TARGETDIR)/IpClassifierSettings.o \
		$(TARGETDIR)/TLV.o \
		$(TARGETDIR)/DcdMsg.o \
		$(TARGETDIR)/DsgConfigSettings.o \
		$(TARGETDIR)/DsgClient.o \
		$(TARGETDIR)/DsgClientRegistrar.o \
		$(TARGETDIR)/DsgClientManager.o \
		$(TARGETDIR)/DsgEstbTunnelSettingsList.o \
		$(TARGETDIR)/DsgEstbTunnelSettings.o \
		$(TARGETDIR)/Type43VendorSettings.o \
		$(TARGETDIR)/ErrorSet.o \
		$(TARGETDIR)/DsgBroadcastTunnelHeader.o \
		$(TARGETDIR)/DsgBroadcastTunnelPktReassembler.o

ifeq ($(APPNAME_SUPPORT), 1)
	DSGCC_OBJS += 	$(TARGETDIR)/DsgAppNameTable.o $(TARGETDIR)/DsgAppName.o
endif

#ifeq ($(ECM_STUB_API), 0)
#	DSGCC_OBJS += 	$(TARGETDIR)/EstbRemoteIfHelperThread.o \
#			$(TARGETDIR)/EstbRemoteInterfaceStub.o
#endif

ifeq ($(DSGCC_BUILT_AS_LIB_FOR_ESTBOPENCABLESYSTEM), 0)
	DSGCC_OBJS += 	$(TARGETDIR)/EstbRemoteIfHelperThread.o \
			$(TARGETDIR)/EstbRemoteInterfaceStub.o \
			$(TARGETDIR)/StubInterface.o
endif

ifeq "$(TARGETOS)" "vxWorks"
	ifeq ($(CLIENT_INTERFACE_SUPPORT), 1)
		DSGCC_OBJS +=   $(TARGETDIR)/dsgcc_clnt_if.o \
				$(TARGETDIR)/SampleClient.o
	endif
endif

ifeq ($(DSG_RPC_INCLUDED),1)
	DSGCC_OBJS	+=	$(TARGETDIR)/dsgccClientRegistration_rpcServer_Stub.o \
				$(TARGETDIR)/dsgccClientRegistration_rpc_xdr.o \
				$(TARGETDIR)/dsgccClientCallback_rpcClient.o \
				$(TARGETDIR)/dsgccClientCallback_rpc_xdr.o \
				$(TARGETDIR)/dsgccClientRegistration_rpcServer.o
else
		DSGCC_OBJS +=   $(TARGETDIR)/dsgcc_clnt_if.o
endif

ifeq ($(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC),1)
	DSGCC_OBJS +=	$(TARGETDIR)/EstbRemoteIfDataHandlerThread.o \
			$(TARGETDIR)/EstbRemoteInterfaceDataStub.o \
			$(TARGETDIR)/BcmSharedMemory.o
else
    ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
	DSGCC_OBJS += $(TARGETDIR)/BcmSharedMemory.o
endif
endif

ifeq ($(BCM_DUAL_DOWNLOAD_SUPPORT),1)
	DSGCC_OBJS	+=	$(TARGETDIR)/DsgDownloadCtlThread.o \
				$(TARGETDIR)/DsgTftpServer.o
endif

ifeq ($(BCM_CONSOLE_SUPPORT),1)
	DSGCC_OBJS +=	$(TARGETDIR)/DsgCommandTable.o
endif

ifeq ($(BCM_NONVOL_SUPPORT),1)
	DSGCC_OBJS     +=  $(TARGETDIR)/DiskFileNonVolDevice.o $(TARGETDIR)/DsgccNonVolSettings.o \
				$(TARGETDIR)/EstbTestNonVolSettings.o

        ifeq ($(BCM_CONSOLE_SUPPORT),1)
		DSGCC_OBJS     +=  $(TARGETDIR)/DsgccNonVolSettingsCommandTable.o \
					$(TARGETDIR)/EstbTestNonVolSettingsCommandTable.o
        endif
endif

ifeq ($(CABLECARD_SUPPORT),1)
        DSGCC_OBJS     += $(TARGETDIR)/DsgccCableCardIf.o $(TARGETDIR)/DsgCableCardClient.o
        EXTRA_INCLUDE  +=  -I$(STB_CLIENT_DIR)/CableCardInterface
        VPATH         += $(STB_CLIENT_DIR)/CableCardInterface
endif
############################################################################
##                 CFLAGS
############################################################################


############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################


############################################################################
##                 VPATH - source path
############################################################################


############################################################################
##                 DSGCC_OBJS - object files
############################################################################


############################################################################
##                 LIB_EXTRA - libraries
############################################################################

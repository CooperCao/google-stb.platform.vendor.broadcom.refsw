########################################################################
#
# Makefile - 
#
# modification history
# --------------------
# 7/1/2000 - JRM - original
# 5/9/2002 - Mike Sieweke - modified for Linux
#
# DESCRIPTION
# This file contains rules for building Linux
#
#########################################################################

# TGT_DIR=$(WIND_BASE)/target
# include $(TGT_DIR)/h/make/defs.bsp
# include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
# include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

# All of the above make files are condensed into defs.bsp.
#include ../../../CmApp_Docsis1.0/linux/makeinc/defs.bsp
include ../../../../StbApp_DsgCC/linux/makeinc/defs.bsp

# These definitions are used by the generic make include files to define
# the names of directories and files.  It would be better if all directories
# and file names were consistent.
TARGETOS     = Linux
LC_TARGETOS  = linux
OS_PREFIX    = Lnx
LC_OS_PREFIX = lnx

# Set this to 0 if you want to use the socket interface
BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC  = 1

###########################################################################
##  dsg - this includes the dsg components
############################################################################
#DSG_SRC = ../../../StbDocsisApi/DsgMa

EXTRA_INCLUDE += -I.. -I../.. -I../../../../Stb/StbDocsisApi/SharedMemory
ADDED_CFLAGS   += -DBCM_DSG_API_TEST_DUAL_PROCESS -DPROCESSOR_TYPE=BCMMIPSEL -DDSG_RPC_INCLUDED=1
ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
    VPATH += ../../../../Stb/StbDocsisApi/SharedMemory
    ADDED_CFLAGS += -DBCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC=1 
endif

EXTRA_INCLUDE += -I../../../../DsgCC/ClientInterface/RpcInterface/DsgClient \
		 -I../../../../DsgCC/ClientInterface/RpcInterface/Common
		 
VPATH         += ../../../../DsgCC/ClientInterface/RpcInterface/DsgClient \
		 ../../../../DsgCC/ClientInterface/RpcInterface/Common

#MACH_EXTRA     += HelloWorld.o
MACH_EXTRA     += dsgClient.o \
				  dsgccClientRegistration_rpc_xdr.o \
				  dsgccClientRegistration_rpcClient.o \
				  dsgccClientRegistration_rpcClient_Stub.o \
				  dsgccClientCallback_rpcServer.o \
				  dsgccClientCallback_rpcServer_Stub.o \
				  dsgccClientCallback_rpc_xdr.o

ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
    MACH_EXTRA   += BcmSharedMemory.o
endif

## klmod added:
ADDED_CFLAGS += -DTARGETOS_Linux -Wno-deprecated
ifeq ($(GDB_DEBUGGING), 0)
  ## Omit all symbol info from the output file. Add map file
  LDFLAGS      += -s
else
  LDFLAGS      += -Wl,-Map -Wl,lxoutput.map
  CC_OPTIM_NORMAL += -g -ggdb
  CC_OPTIM_TARGET += -g -ggdb
endif

############################################################################
ROM_LDFLAGS       = -Map lxoutput.map
BOOT_EXTRA        = itab.o
ADDED_C++FLAGS   += $(ADDED_CFLAGS)

#########################################################################
## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.
#########################################################################

#include ../../../CmApp_Docsis1.0/linux/makeinc/rules.bsp
include ../../../../StbApp_DsgCC/linux/makeinc/rules.bsp


############################################################
#  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#
############################################################

#
# Conditional options for this makefile
#
# None at this time
#

#
# Identify this library
#
B_LIBS += B_CDL_LIB

#
# Define source files in this library
#
B_CDL_LIB_SOURCES += \
                    $(B_CABLE_TOP)/lib/cdl/src/b_cdl_lib.c \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_storage_flash.c \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_storage_file.c \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_storage_dir.c \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_bootinfo_nvram.c \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_image_brcm.c \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_msg.c \
                    $(B_CABLE_TOP)/lib/cdl/src/lib_queue.c 

ifeq ($(DSG_SUPPORT),y)
	BSEAV ?= $(shell cd ../../../BSEAV; pwd)

	ifeq ($(findstring $(NEXUS_PLATFORM), 97420 97422 97459), $(NEXUS_PLATFORM))
	DOCSIS_CHIP = 3255
	else ifeq ($(findstring $(NEXUS_PLATFORM), 97125), $(NEXUS_PLATFORM))
	DOCSIS_CHIP = 7125
	else ifeq ($(findstring $(NEXUS_PLATFORM), 97429 97425 97435), $(NEXUS_PLATFORM))
	ifneq ($(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT), y)
	DOCSIS_CHIP = 3383
	else
	DOCSIS_CHIP = 3384
	endif
	else 
	DOCSIS_CHIP = 3384
	endif
	DSG_DIR = $(BSEAV)/cable/docsis/$(DOCSIS_CHIP)	

	B_CDL_LIB_DEFINES += DSG_SUPPORT BCM_DSG_DUAL_PROCESSOR_INTERFACE

	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/cm_bsp_v2/bsp_common/common_inc/
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/cm_bsp_v2/bsp_common/BoardHal/
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/DsgCC/
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/RswDload
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/Stb/StbDocsisApi/Dsg
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/StbEcmNvolEditor/Stb/StbDocsisApi/Linux
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/Stb/StbDocsisApi/Linux
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/DsgCC/ClientInterface
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/DsgCC/ClientInterface/RpcInterface/DsgClient
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/DsgCC/ClientInterface/RpcInterface/Common
	B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/StbHostApps/DsgCC/ClientInterface/SampleApp/Linux

	ifeq ($(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC),1)
		B_CDL_LIB_PUBLIC_INCLUDES += $(DSG_DIR)/Stb/StbDocsisApi/SharedMemory
	endif

	B_CDL_LIB_SOURCES += \
        	 	$(B_CABLE_TOP)/lib/cdl/src/cdl_estb.c \
                	$(B_CABLE_TOP)/lib/cdl/src/cdl_common.c \
                    	$(B_CABLE_TOP)/lib/cdl/src/cdl_ctl.c 

	B_CDL_LIB_DEFINES += COMMON_DOWNLOAD_SUPPORT
endif

##ifeq ($(CABLECARD_SUPPORT),MPOD)
ifeq ($(CABLECARD_SUPPORT),y)
	B_CDL_LIB_SOURCES += \
        		$(B_CABLE_TOP)/lib/cdl/src/cdl_dsg.c \
                    	$(B_CABLE_TOP)/lib/cdl/src/cdl_adpu.c
	B_CDL_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/mpod/include

	ifeq ($(API_SUPPORT),NEXUS)
		B_CDL_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dsmcc/include
		B_CDL_LIB_DEFINES +=  NEXUS_SUPPORT
	endif
	ifeq ($(API_SUPPORT),SETTOPAPI)
		B_CDL_LIB_PUBLIC_INCLUDES += $(BSEAV)/api/include \
					$(BSEAV)/lib/bfile \
					$(BSEAV)/lib/mpeg2_ts_parse
		B_CDL_LIB_SOURCES += \
                	$(B_CABLE_TOP)/lib/cdl/src/cdl_dsmcc.c
		B_CDL_LIB_DEFINES +=  SETTOPAPI_SUPPORT
	endif
	B_CDL_LIB_DEFINES +=  MPOD_SUPPORT
	
	ifeq ($(ESTB_CFG_SUPPORT),y)
		B_CDL_LIB_DEFINES += ESTB_CFG_SUPPORT
		B_CDL_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/estb_cfg/include
	endif
endif

ifeq ($(TEST_INBAND_DOWNLOAD),y)
     B_CDL_LIB_DEFINES +=  TEST_INBAND_DOWNLOAD
     B_CDL_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dsmcc/include
     B_CDL_LIB_DEFINES +=  NEXUS_SUPPORT
     B_CDL_LIB_DEPENDENCIES += B_DSMCC_LIB
endif

ifeq ($(TEST_ON_THE_FLY_CDL),y)
     B_CDL_LIB_DEFINES +=  TEST_ON_THE_FLY_CDL
     B_CDL_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/dsmcc/include
     B_CDL_LIB_DEFINES +=  NEXUS_SUPPORT
     B_CDL_LIB_DEPENDENCIES += B_DSMCC_LIB
     B_CDL_LIB_SOURCES += \
                    $(B_CABLE_TOP)/lib/cdl/src/cdl_storage_fifo.c 
endif
ifeq ($(CDL_SUPPORT_ENABLE_SIGNED_IMAGE),y)
     B_CDL_LIB_DEFINES +=  CDL_SUPPORT_ENABLE_SIGNED_IMAGE
endif

#
# Define public include path(s) for this library
#
B_CDL_LIB_PUBLIC_INCLUDES += $(B_LIB_TOP)/os/include $(B_LIB_TOP)/os/include/linuxuser
B_CDL_LIB_PUBLIC_INCLUDES += $(B_CABLE_TOP)/lib/cdl/include

#
# Define private include path(s) for this library
#
B_CDL_LIB_PRIVATE_INCLUDES += $(B_CABLE_TOP)/lib/cdl/src \
                              $(B_CABLE_TOP)/lib/cdl/src/$(SAMPLE)

#
# Define other libs that this library is dependent upon.  
# This will check for cyclic dependencies and setup include paths
#
#B_CDL_LIB_DEPENDENCIES := ## N/A for this library
B_CDL_LIB_DEPENDENCIES := B_ESTB_CFG_LIB
##ifeq ($(CABLECARD_SUPPORT),MPOD)
ifeq ($(CABLECARD_SUPPORT),y)
	ifeq ($(API_SUPPORT),NEXUS)
		B_CDL_LIB_DEPENDENCIES += B_DSMCC_LIB
	endif
endif
#
# Define extra C definitions
#
#B_CDL_LIB_DEFINES += ## N/A for this library

#
# Define possible options - this is only for debug, it does _not_ actually define them.
#
#B_DSMCC_CDL_OPTIONS += ## N/A for this library


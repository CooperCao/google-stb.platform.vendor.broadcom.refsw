##############################################################################
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
##############################################################################

############################################################################
##                 FLAGS
############################################################################
##############################################################################
#
# Compile time definitions - these are generally passed to the compiler with
#                            the -D flag.  Note that you do not specify the
#                            -D here; this is done for you by the top level
#                            make processing.
  
BCM_RSWDLOAD_DEFINES += ESTB_RSWDLOAD_SUPPORT=1
	
############################################################################
##                 V2 COMPONENT INCLUDES
############################################################################


############################################################################
##                 PATHS
############################################################################

  STB_DSG_RPS  = StbHostApps/RswDload
  STB_DSG_API = Stb/StbDocsisApi/Dsg
  
  STB_BSP_COMMON_SRC  = cm_bsp_v2/bsp_common/common_src
  STB_BSP_COMMON_INC  = cm_bsp_v2/bsp_common/common_inc

############################################################################
##                 OBJECT FILES
############################################################################

BCM_RSWDLOAD_OBJECTS +=	rProgramStoreCustomerApi.o rProgramStoreHost.o crc.o

#Include DsgDownloadCtlThread in the build since this is where rPS is executed from.
ifeq ($(BCM_DUAL_DOWNLOAD_SUPPORT),0)
	BCM_RSWDLOAD_OBJECTS	+=	DsgDownloadCtlThread.o
endif

############################################################################
##                 EXTRA_INCLUDE - include path
############################################################################

BCM_RSWDLOAD_INCLUDE_DIRS += -I$(STB_DSG_RPS)

############################################################################
##                 VPATH - source path
############################################################################

BCM_RSWDLOAD_SOURCE_DIRS         += $(STB_DSG_RPS)

############################################################################
##                 LIB_EXTRA - libraries
############################################################################

##############################################################################
#
# Conditional processing - this is where you should add the locally defined
# 			   variables (object files, directories, etc) to the
#			   global/external variables that are used by the top
#			   level makefile.  You will also have conditional
#			   processing here (e.g. if console support enabled,
#			   add local console files and defines).

# Add RSWDLOAD components, if included by BfcSystemConfig.mak.
ifeq ($(ESTB_RSWDLOAD_SUPPORT), 1)

	BFC_OBJECTS += $(BCM_RSWDLOAD_OBJECTS) 

	BFC_COMPILER_DEFINES += $(BCM_RSWDLOAD_DEFINES)

	BFC_INCLUDE_DIRS += $(BCM_RSWDLOAD_INCLUDE_DIRS) 

	BFC_SOURCE_DIRS += $(BCM_RSWDLOAD_SOURCE_DIRS) 

	BFC_LIBRARIES += 

	BFC_LIBRARIES_ABS_PATH += 
endif



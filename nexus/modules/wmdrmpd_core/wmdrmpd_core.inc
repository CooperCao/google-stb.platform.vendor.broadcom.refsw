############################################################
#     (c)2007-2013 Broadcom Corporation
#  
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# $brcm_Log: $
#
############################################################

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += WMDRMPD_CORE

# Only support Nexus WmDrm Interface with Playready 2.0
# Playready 2.0 is only supported with robustness enabled.
NEXUS_PLAYREADY_SDK_VERSION:=2.0

#endif

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_WMDRMPD_CORE_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/wmdrmpd_core/include

##################
# Module Sources #
##################
NEXUS_WMDRMPD_CORE_SOURCES := \
	$(NEXUS_TOP)/modules/wmdrmpd_core/src/nexus_wmdrmpd_core.c \
	$(NEXUS_TOP)/modules/wmdrmpd_core/src/nexus_wmdrmpd_core_fileio.c \
	$(NEXUS_TOP)/modules/wmdrmpd_core/src/nexus_wmdrmpd_core_module.c

# thunk layer
NEXUS_WMDRMPD_CORE_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/wmdrmpd_core/src $(NEXUS_SYNCTHUNK_DIR)
NEXUS_WMDRMPD_CORE_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_wmdrmpd_core_thunks.c
NEXUS_WMDRMPD_CORE_PRIVATE_INCLUDES += $(BSEAV)/lib/drmrootfs

#
# This pulls in the bdrm sources for nexus kernel mode build
#
ifeq ($(B_REFSW_OS), linuxkernel)
include $(BSEAV)/lib/drmrootfs/drmrootfs_libdir.inc
NEXUS_EXTRALIBS += $(BSEAV)/lib/drmrootfs/$(DRMROOTFS_LIBDIR)/libdrmrootfs.a

include $(BSEAV)/lib/playready/$(NEXUS_PLAYREADY_SDK_VERSION)/Makefile.wmdrmpd
NEXUS_WMDRMPD_CORE_DEFINES += $(WMDRMPD_CFLAGS)
NEXUS_WMDRMPD_CORE_DEFINES += B_HAS_MSDRM_PRDY=1 B_HAS_MSDRM_PD=1 
NEXUS_WMDRMPD_CORE_DEFINES += NEXUS_WMDRMPD_FILE_HANDLING
NEXUS_WMDRMPD_CORE_DEFINES += PRDY_ROBUSTNESS
NEXUS_WMDRMPD_CORE_DEFINES += PRDY_SDK_2_0

NEXUS_WMDRMPD_CORE_PRIVATE_INCLUDES += $(WMDRMPD_INCLUDES)
NEXUS_WMDRMPD_CORE_SOURCES += $(WMDRMPD_SRCS)

# Guard against double inclusion of precompiled SDK libs
ifeq (,$(findstring oem_ansi,$(NEXUS_EXTRALIBS)))
NEXUS_EXTRALIBS += $(sort $(WMDRMPD_PRECOMPILED_LIBS))
endif
endif

#
# This pulls in the bdrm sources for nexus user mode build
#
ifneq ($(NEXUS_MODE),proxy)
ifeq ($(B_REFSW_OS), linuxuser)

include $(BSEAV)/lib/drmrootfs/drmrootfs_libdir.inc
NEXUS_EXTRALIBS += $(BSEAV)/lib/drmrootfs/$(DRMROOTFS_LIBDIR)/libdrmrootfs.a
include $(BSEAV)/lib/playready/$(NEXUS_PLAYREADY_SDK_VERSION)/Makefile.wmdrmpd

NEXUS_WMDRMPD_CORE_DEFINES += $(WMDRMPD_CFLAGS)
NEXUS_WMDRMPD_CORE_DEFINES += B_HAS_MSDRM_PRDY=1 B_HAS_MSDRM_PD=1
NEXUS_WMDRMPD_CORE_DEFINES += PRDY_ROBUSTNESS
NEXUS_WMDRMPD_CORE_DEFINES += PRDY_SDK_2_0

NEXUS_WMDRMPD_CORE_PRIVATE_INCLUDES += $(WMDRMPD_INCLUDES)

ifneq ($(NEXUS_MODE),client)
NEXUS_WMDRMPD_CORE_SOURCES += $(WMDRMPD_SRCS)

# Guard against double inclusion of precompiled SDK libs
ifeq (,$(findstring oem_ansi,$(NEXUS_EXTRALIBS)))
NEXUS_EXTRALIBS += $(sort $(WMDRMPD_PRECOMPILED_LIBS))
endif
endif
endif

endif

#######################
# Module Dependencies #
#######################
NEXUS_WMDRMPD_CORE_DEPENDENCIES := BASE CORE DMA SECURITY TRANSPORT WMDRMPD_IO 

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds



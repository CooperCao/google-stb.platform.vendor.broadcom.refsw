############################################################
#     (c)2003-2011 Broadcom Corporation
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
#   This file defines the OS-specific flags for building in ucos.
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

#ifeq ($(ALREADY_RAN),true)
#$(error This file is being included twice)
#endif
#ALREADY_RAN = true

#STD_INC  ?= $(B_REFSW_TOOLCHAIN_DIR)/sde/include

#ifndef UCOS_INCLUDES
#$(error UCOS_INCLUDES has not been defined.  Stop.)
#endif

#ifndef HDI4_INCLUDES
#$(error HDI4_INCLUDES has not been defined.  Stop.)
#endif

#ifeq ($(NEXUS_ENDIAN), BSTD_ENDIAN_BIG)
#ENDIAN_FLAG     = -EB
#else
#ENDIAN_FLAG     = -EL
#endif

# Required by archMips.h
#NEXUS_CFLAGS += -DCPU=MIPS32
# Required by HDI4 header files
#NEXUS_CFLAGS += -D_CC3

#NEXUS_CFLAGS += -DMIPS_SDE5
#NEXUS_CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}
#NEXUS_CFLAGS += $(ENDIAN_FLAG)
#NEXUS_CFLAGS += -DLANGUAGE_C

# SDE 6.06
#NEXUS_CFLAGS += -march=mips32 -mips32
# SDE 5.03
#NEXUS_CFLAGS += -mcpu=4kc -mips32

#NEXUS_CFLAGS += -msoft-float -Wimplicit -G 0
#NEXUS_CFLAGS += -nostdinc -nostdlib
#NEXUS_CFLAGS += -Wall

##NEXUS_CFLAGS += -ffreestanding -finline-limit=100000  -mabi=32
##NEXUS_CFLAGS += -fomit-frame-pointer -fno-strict-aliasing  -mno-abicalls -fno-pic -Wa,--trap -mlong-calls
##NEXUS_CFLAGS += -pipe

#
# Optimization flag is set in os_flags_nexus.inc
#

# Default includes
#NEXUS_CFLAGS += -I$(STD_INC)
#NEXUS_CFLAGS += -I$(UCOS_INCLUDES)/inc
#NEXUS_CFLAGS += -I$(UCOS_INCLUDES)/Code
#NEXUS_CFLAGS += -I$(UCOS_INCLUDES)/CPU
#NEXUS_CFLAGS += -I$(HDI4_INCLUDES)
#NEXUS_CFLAGS += -I$(NEXUS_TOP)/platforms/$(NEXUS_PLATFORM)/src/$(B_REFSW_OS)/bsp
#NEXUS_CFLAGS += -I$(MAGNUM)/basemodules/std/config

# Flag to generate dependencies
#CDEP_FLAG = -MD



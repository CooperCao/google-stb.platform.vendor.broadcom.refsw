############################################################
#     (c)2003-2008 Broadcom Corporation
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
#   This file defines the OS-specific flags for building in vxworks.
#
# Revision History:
#
# $brcm_Log: $
# 
############################################################

ifeq ($(WIND_BASE),)
#$(error WIND_BASE is required. It is normally defined by torVars.bat or wrenv.exe)
$(error WIND_BASE is required. It is normally defined by wrenv.exe)
endif

# We only support big endian build -- make sure they didn't try LE.
ifneq ($(NEXUS_ENDIAN),BSTD_ENDIAN_BIG)
$(error Only Big Endian builds are supported.)
endif

ifeq ($(ALREADY_RAN),true)
$(error This file is being included twice)
endif
#ALREADY_RAN = true

STD_INC ?= $(WIND_BASE)/target/h
STD_CORE_INC ?= $(WIND_BASE)/target/h/wrn/coreip

# This only supports 32-bit products so far
NEXUS_CFLAGS += -DVXWORKS
NEXUS_CFLAGS += -DTOOL_FAMILY=gnu -DCPU=MIPS32 -DTOOL=sfgnu -mips2 -msoft-float -DSOFT_FLOAT
NEXUS_CFLAGS += -DVXWORKS6

NEXUS_CFLAGS += -G 0 -W -Wall -mno-branch-likely -fno-builtin -fdollars-in-identifiers
NEXUS_CFLAGS += -I${STD_CORE_INC} -I${STD_INC} 

NEXUS_CFLAGS += -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_BIG -EB -DMIPSEB

NEXUS_LDFLAGS += -nostdlib -r -Wl,-EB -Wl,-X

# Flag to generate dependencies
CDEP_FLAG = -MD



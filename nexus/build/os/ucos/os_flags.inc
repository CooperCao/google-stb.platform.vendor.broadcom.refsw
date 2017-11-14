#############################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
############################################################

############################################################
# Overview:
#
# This file should only be included by nexus_defs.inc
#
############################################################

NEXUS_CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}
NEXUS_CFLAGS += -nostdlib -ffreestanding -W -Wstrict-prototypes -Wall

#ifeq ($(B_REFSW_ARCH), arm-linux)
ifeq ($(filter $(B_REFSW_ARCH),arm-linux aarch64-linux), $(B_REFSW_ARCH))
    ifeq ($(filter $(B_REFSW_ARCH),arm-linux), $(B_REFSW_ARCH))
        NEXUS_CFLAGS += -marm
    endif
    ifeq ($(ARM_ARCH), V8)
        NEXUS_CFLAGS += -march=armv8-a
    else
        ifeq ($(ARM_ARCH), V53)
            NEXUS_CFLAGS += -mcpu=cortex-a53
        else
            NEXUS_CFLAGS += -march=armv7-a
        endif
    endif
    NEXUS_CFLAGS += -Wno-unused-but-set-variable
    ifeq ($(NEXUS_ENDIAN), BSTD_ENDIAN_BIG)
        NEXUS_CFLAGS += -mbig-endian
    endif
else
    NEXUS_CFLAGS += -mcpu=4kc -mips32 -G 0 -c
    ifeq ($(NEXUS_ENDIAN), BSTD_ENDIAN_BIG)
        NEXUS_CFLAGS += -EB
    else
        NEXUS_CFLAGS += -EL
    endif
    NEXUS_CFLAGS += -DMIPS_SDE
endif

# Debug
# NEXUS_CFLAGS += -g -O0

# NEXUS_EXTRA_CFLAGS can be defined by the user to bring in extra CFLAGS
NEXUS_CFLAGS += ${NEXUS_EXTRA_CFLAGS}

#
# Optimization flag is set in os_flags_nexus.inc
#

# Flag to generate dependencies
CDEP_FLAG = -MMD

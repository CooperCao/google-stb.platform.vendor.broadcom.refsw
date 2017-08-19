#############################################################
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
############################################################

############################################################
# Overview:
#
# This file should only be included by nexus_defs.inc
#
############################################################

NEXUS_CFLAGS += -D_GNU_SOURCE=1 -DLINUX -pipe
NEXUS_CFLAGS += -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
NEXUS_CFLAGS += -DBSTD_CPU_ENDIAN=${NEXUS_ENDIAN}

ifeq ($(findstring mips, ${B_REFSW_ARCH}),mips)
#
# Default to mips32 in linuxuser
#
NEXUS_CFLAGS += -mips32
endif

ifeq ($(findstring arm, ${B_REFSW_ARCH}),arm)
#
# Default to armv7-a in linuxuser
#
NEXUS_CFLAGS +=-marm -march=armv7-a
endif

ifeq ($(B_REFSW_ANDROID),y)
IS_TOOLCHAIN_VALID := $(shell test -e $(B_REFSW_CROSS_COMPILE)gcc && echo y)
else
IS_TOOLCHAIN_VALID := y
endif
ifeq ($(IS_TOOLCHAIN_VALID),y)
B_REFSW_TOOLCHAIN_VER := $(shell $(B_REFSW_CROSS_COMPILE)gcc -dumpversion)
# TODO: parse string for true "greater than or equal" test
ifneq ($(findstring 4.8.,$(B_REFSW_TOOLCHAIN_VER)),)
B_REFSW_TOOLCHAIN_VER_GE_4_8 := y
endif
endif

# We use c89 for maximum compatibility across OS's
# But, we allow overrides for integrating other code
ifeq ($(NEXUS_C_STD),)
NEXUS_C_STD=c89
endif
NEXUS_INTERNAL_CFLAGS += -std=$(NEXUS_C_STD)

ifeq ($(NEXUS_C_WARNINGS),)
NEXUS_C_WARNINGS += -pedantic
ifeq ($(B_REFSW_DEBUG),y)
# -W is deprecated and is equivalent to -Wextra. We don't use it in release builds.
NEXUS_C_WARNINGS += -W -Wstrict-prototypes
# Release builds generate unused values because BDBG_MSG and WRN are compiled out. -Wno-unused-value does not suppress, so we must remove -Wall.
NEXUS_C_WARNINGS += -Wall
ifeq ($(B_REFSW_TOOLCHAIN_VER_GE_4_8),y)
NEXUS_C_WARNINGS += -Wno-unused-but-set-variable
endif
endif
endif

ifneq ($(filter $(B_REFSW_DEBUG_LEVEL),none wrn err),)
# Release builds generate unused values. We suppress this warning.
NEXUS_C_WARNINGS += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function
endif

NEXUS_INTERNAL_CFLAGS += $(NEXUS_C_WARNINGS)
NEXUS_CFLAGS += $(NEXUS_INTERNAL_CFLAGS)

# NEXUS_EXTRA_CFLAGS  and/or NEXUS_EXTRA_LDFLAGS can be defined by the user to bring in compiler and/or linker flags
NEXUS_CFLAGS += ${NEXUS_EXTRA_CFLAGS}
NEXUS_LDFLAGS += ${NEXUS_EXTRA_LDFLAGS}

ifeq ($(B_REFSW_ANDROID),y)
include $(NEXUS_TOP)/build/os/linuxuser/os_flags_android.inc
else
# only use these when running cpp on the NEXUS_HEADERS generated nexus.cpp.h
NEXUS_API_CPP_CFLAGS := -Wundef -Werror
endif

# Flag to generate dependencies
CDEP_FLAG = -MMD

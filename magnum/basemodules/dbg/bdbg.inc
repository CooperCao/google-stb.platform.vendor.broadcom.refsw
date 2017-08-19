############################################################
#  Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

# Define this module
MAGNUM_MODULES += BDBG

# Backward-compatibility
B_REFSW_OS ?= $(OS)

# Module Includes
BDBG_INCLUDES := $(MAGNUM)/basemodules/dbg

# Sanity Check OS provided
BDBG_SUPPORTED_OS := linuxuser linuxkernel linuxclient linuxemu linuxpal vxworks ucos nucleus win32 bkernel ucos_ii ucos_iii bare no_os
ifeq ($(findstring $(B_REFSW_OS), $(BDBG_SUPPORTED_OS)),)
$(error [dbg] OS must be one of the following: $(BDBG_SUPPORTED_OS))
endif

BDBG_SOURCES := $(MAGNUM)/basemodules/dbg/bdbg.c \
                $(MAGNUM)/basemodules/dbg/bdbg_fifo.c 

ifeq ($(findstring $(B_REFSW_OS), linuxclient linuxemu linuxpal),${B_REFSW_OS})
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/linuxuser/bdbg_os_priv.c
else
ifeq ($(B_REFSW_OS),ucos)
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/$(B_REFSW_UCOS_SUBDIR_NAME)/bdbg_os_priv.c
else
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/$(B_REFSW_OS)/bdbg_os_priv.c
endif
endif

ifeq ($(B_REFSW_OS),vxworks)
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/$(B_REFSW_OS)/bdbg_output.c
else
BDBG_SOURCES += $(MAGNUM)/basemodules/dbg/bdbg_output.c
endif

#
# Compile time options:
# B_REFSW_DEBUG is used by Nexus (see nexus/build/nexus_defs.inc) and other SW above Magnum.
# There is no Magnum build variable to enable DBG; instead, build with the macro -DBDBG_DEBUG_BUILD.
# In addition to that macro, you can also build with the following build variables which
# give intermediate levels of debug functionality:
#
# B_REFSW_DEBUG_LEVEL - compile out some but not all levels
# B_REFSW_DEBUG_COMPACT_ERR - reduce size by using stock messages for errors
# B_REFSW_DEBUG_ASSERT_FAIL - don't call BKNI_Fail if BDBG_ASSERT fails
#

ifeq ($(B_REFSW_DEBUG_LEVEL),none)
BDBG_DEFINES += BDBG_NO_ERR=1 BDBG_NO_LOG=1 BDBG_NO_WRN=1 BDBG_NO_MSG=1
endif
ifeq ($(B_REFSW_DEBUG_LEVEL),err)
BDBG_DEFINES += BDBG_NO_LOG=1 BDBG_NO_WRN=1 BDBG_NO_MSG=1
endif
ifeq ($(B_REFSW_DEBUG_LEVEL),log)
BDBG_DEFINES += BDBG_NO_WRN=1 BDBG_NO_MSG=1
endif
ifeq ($(B_REFSW_DEBUG_LEVEL),wrn)
BDBG_DEFINES += BDBG_NO_MSG=1
endif

# reduce size by reusing a single format string for all errors
ifeq ($(B_REFSW_DEBUG_COMPACT_ERR),y)
BDBG_DEFINES += B_REFSW_DEBUG_COMPACT_ERR=1
endif

ifeq ($(B_REFSW_DEBUG_ASSERT_FAIL),n)
# BDBG_ASSERT will print ERR, but not fail
# the env variable is positive logic, but the macro is negative logic to not introduce a new default
BDBG_DEFINES += B_REFSW_DEBUG_ASSERT_NO_FAIL=1
endif

ifeq ($(B_REFSW_DEBUG_VERBOSE_ERR),y)
# BDBG_ERR will also display file and line number
BDBG_DEFINES += B_REFSW_DEBUG_VERBOSE_ERR=1
endif

ifeq ($(B_REFSW_STRICT_PRINTF_FORMAT),y)
BDBG_DEFINES += B_REFSW_STRICT_PRINTF_FORMAT=1
endif

ifeq ($(B_REFSW_BUILD_FOR_STATIC_ANALYSIS),y)
BDBG_DEFINES += B_REFSW_BUILD_FOR_STATIC_ANALYSIS=1
endif

ifeq ($(B_REFSW_ANDROID), y)
ifneq ($(B_REFSW_OS), linuxkernel)
BDBG_DEFINES += BDBG_ANDROID_LOG=1
else
BDBG_DEFINES += BDBG_ASSERT_TO_KNI=1
endif
endif

# used for proc, otherwise save code size
ifeq ($(B_REFSW_OS), linuxkernel)
BDBG_DEFINES += BDBG_USE_VA_ARGS
endif

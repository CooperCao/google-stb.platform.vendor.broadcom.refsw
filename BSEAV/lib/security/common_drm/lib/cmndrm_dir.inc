#############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#############################################################################

# Determine core size and ZEUS version
include ${NEXUS_TOP}/modules/security/security_defs.inc
#$(info NEXUS_SECURITY_ZEUS_VERSION_MAJOR=$(NEXUS_SECURITY_ZEUS_VERSION_MAJOR) NEXUS_SECURITY_ZEUS_VERSION_MINOR=$(NEXUS_SECURITY_ZEUS_VERSION_MINOR))

ifeq ($(B_REFSW_DEBUG), n)
BUILD_MODE = retail
else
ifeq ($(B_REFSW_DEBUG), minimal)
BUILD_MODE = minimal
else
ifeq ($(B_REFSW_DEBUG), no_error_messages)
BUILD_MODE = noerrors
else
BUILD_MODE = debug
endif
endif
endif

ifeq ($(findstring mips,$(B_REFSW_ARCH)),mips)

GCC_GTEQ_485 := $(shell expr `$(CC) -dumpversion | sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/'` \>= 40805)

endif

ifeq ($(findstring $(NEXUS_SECURITY_ZEUS_VERSION_MAJOR), 1 2), $(NEXUS_SECURITY_ZEUS_VERSION_MAJOR))
ifeq ($(ANDROID_BUILD),y)
CMNDRM_DIR := Zeus20/android/${BUILD_MODE}
else
ifeq "$(GCC_GTEQ_485)" "1"
CMNDRM_DIR := Zeus20/${BUILD_MODE}/eglibc
else
CMNDRM_DIR := Zeus20/${BUILD_MODE}
endif
endif
endif

ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),3)
ifeq ($(ANDROID_BUILD),y)
CMNDRM_DIR := Zeus30/android/${BUILD_MODE}
else
ifeq "$(GCC_GTEQ_485)" "1"
CMNDRM_DIR := Zeus30/${BUILD_MODE}/eglibc
else
CMNDRM_DIR := Zeus30/${BUILD_MODE}
endif
endif
endif


ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),4)
ifeq ($(ANDROID_BUILD),y)
CMNDRM_DIR := Zeus4x/android/${BUILD_MODE}
else
CMNDRM_DIR := Zeus4x/${BUILD_MODE}
endif
endif

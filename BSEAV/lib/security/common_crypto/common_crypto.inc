############################################################
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
############################################################

# COMMON CRYPTO module

# Do not extend COMMON_CRYPTO_INCLUDES with manual paths into Nexus.
# Instead, whoever compiles common_crypto must already have the required Nexus module paths set.
# This allows Nexus to prevent circular calling dependencies.
COMMON_CRYPTO_INCLUDES := ${BSEAV}/lib/security/common_crypto/include

########################################
# Determine core size and ZEUS version
########################################
include ${NEXUS_TOP}/modules/security/security_defs.inc
#$(info NEXUS_SECURITY_ZEUS_VERSION_MAJOR=$(NEXUS_SECURITY_ZEUS_VERSION_MAJOR) NEXUS_SECURITY_ZEUS_VERSION_MINOR=$(NEXUS_SECURITY_ZEUS_VERSION_MINOR))

COMMON_CRYPTO_INCLUDES += ${NEXUS_TOP}/extensions/security/keyladder/include/40nm

########################################
# Core 40nm
########################################
ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),1)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_40NM_CHIP=1
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=10
endif

ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),2)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_40NM_CHIP=1

    ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),0)
        NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=20
    else
        ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),2)
            NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=22
        endif
    endif
endif

ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),3)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_40NM_CHIP=1
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=30
endif

########################################
# Core 28nm
########################################
ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),4)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_28NM_CHIP=1

    ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),1)
        NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=41
    else
        ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MINOR),2)
            NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=42
        endif
    endif
endif
ifeq ($(NEXUS_SECURITY_ZEUS_VERSION_MAJOR),5)
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_28NM_CHIP=1
    NEXUS_CFLAGS += -DCOMMON_CRYPTO_ZEUS_VERSION=50
endif

COMMON_CRYPTO_INCLUDES += ${NEXUS_TOP}/extensions/security/securersa/include
COMMON_CRYPTO_SOURCES :=  ${BSEAV}/lib/security/common_crypto/src/common_crypto.c

ifeq ($(SECURERSA_SUPPORT), y)
    COMMON_CRYPTO_SECURERSA := $(shell test -e ${BSEAV}/lib/security/common_crypto/src/common_crypto_securersa.c && echo y)
    ifeq ($(COMMON_CRYPTO_SECURERSA),y)
        $(warning Compiling Common Crypto Secure RSA module into Nexus)
        COMMON_CRYPTO_SOURCES +=  ${BSEAV}/lib/security/common_crypto/src/common_crypto_securersa.c
    endif
endif

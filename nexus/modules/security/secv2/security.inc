#############################################################
#*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#*
#*  This program is the proprietary software of Broadcom and/or its licensors,
#*  and may only be used, duplicated, modified or distributed pursuant to the terms and
#*  conditions of a separate, written license agreement executed between you and Broadcom
#*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#*  no license (express or implied), right to use, or waiver of any kind with respect to the
#*  Software, and Broadcom expressly reserves all rights in and to the Software and all
#*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#*
#*  Except as expressly set forth in the Authorized License,
#*
#*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#*  and to use this information only in connection with your use of Broadcom integrated circuit products.
#*
#*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#*  USE OR PERFORMANCE OF THE SOFTWARE.
#*
#*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#*  ANY LIMITED REMEDY.
############################################################

#
# Conditional Options for this Makefile
#

##################
# Module Options #
##################

ifeq ($(NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE),y)
    NEXUS_SECURITY_DEFINES += NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE=1
    _NEXUS_SECURITY_DUMP_FIRMWARE=y
endif

ifeq ($(NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW),y)
    NEXUS_SECURITY_DEFINES += NEXUS_REGION_VERIFICATION_DUMP_FIRMWARE_RAW=1
    _NEXUS_SECURITY_DUMP_FIRMWARE=y
endif

ifeq ($(NEXUS_ENFORCE_REGION_VERIFICATION),y)
    NEXUS_SECURITY_DEFINES += NEXUS_ENFORCE_REGION_VERIFICATION=1
endif

###################
# Module Features #
###################


###################
# Module Includes #
###################

NEXUS_SECURITY_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/security/secv2/include

##################
# Module Sources #
##################
NEXUS_SECURITY_SOURCES := $(NEXUS_TOP)/modules/security/secv2/src/nexus_security_init.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_security.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_security_priv.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_keyslot.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_keyladder.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_otp_msp.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_otp_datasection.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_otp_key.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_hash_priv.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_hash.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_hmac.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_rsa.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_random_number.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_regver.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_regver_rsa.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_regver_priv.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_signed_command.c
NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/nexus_pcie_window.c

ifeq ($(NEXUS_DUPLE_KEYLADDER_SUPPORT),)
        NEXUS_DUPLE_KEYLADDER_SUPPORT := $(shell test -e $(NEXUS_TOP)/extensions/security/secv2/duple_keyladder/src/nexus_duple_keyladder.c && echo y)
endif
ifeq ($(NEXUS_DUPLE_KEYLADDER_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/secv2/duple_keyladder/duple_keyladder.inc
endif

ifeq ($(NEXUS_POWAY_SUPPORT),)
        NEXUS_POWAY_SUPPORT := $(shell test -e $(NEXUS_TOP)/extensions/security/secv2/poway/src/nexus_poway.c && echo y)
endif
ifeq ($(NEXUS_POWAY_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/secv2/poway/poway.inc
endif

ifeq ($(NEXUS_ENCINITAS_SUPPORT),)
        NEXUS_ENCINITAS_SUPPORT := $(shell test -e $(NEXUS_TOP)/extensions/security/secv2/encinitas/src/nexus_encinitas.c && echo y)
endif
ifeq ($(NEXUS_ENCINITAS_SUPPORT),y)
    include ${NEXUS_TOP}/extensions/security/secv2/encinitas/encinitas.inc
endif

ifeq ($(_NEXUS_SECURITY_DUMP_FIRMWARE),y)
	ifeq ($(findstring $(B_REFSW_OS), linuxkernel),${B_REFSW_OS})
        NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/linuxkernel/nexus_security_regver_dump.c
    else
        NEXUS_SECURITY_SOURCES += $(NEXUS_TOP)/modules/security/secv2/src/linuxuser/nexus_security_regver_dump.c
    endif
endif

ifeq ($(NEXUS_COMMON_CRYPTO_SUPPORT), y)
    include $(BSEAV)/lib/security/common_crypto/common_crypto.inc
    NEXUS_SECURITY_PRIVATE_INCLUDES += $(COMMON_CRYPTO_INCLUDES)
    NEXUS_SECURITY_SOURCES += $(COMMON_CRYPTO_SOURCES)
endif

# thunk layer
NEXUS_SECURITY_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/security/secv2/src  $(NEXUS_SYNCTHUNK_DIR)
NEXUS_SECURITY_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_security_thunks.c


#######################
# Module Dependencies #
#######################
NEXUS_SECURITY_DEPENDENCIES := BASE CORE TRANSPORT

##################
# Magnum Modules #
##################

NEXUS_SECURITY_MAGNUM_MODULES = $(MAGNUM)/portinginterface/hsm/bhsm.inc

NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_ZEUS_VERSION_MAJOR=$(NEXUS_SECURITY_ZEUS_VERSION_MAJOR)
NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_ZEUS_VERSION_MINOR=$(NEXUS_SECURITY_ZEUS_VERSION_MINOR)
NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_ZEUS_VERSION_SUBMINOR=$(NEXUS_SECURITY_ZEUS_VERSION_SUBMINOR)
NEXUS_SECURITY_DEFINES += NEXUS_SECURITY_API_VERSION=2

ifneq ($(filter $(NEXUS_MODE),proxy driver),)
    SEC_BUILD_MODE := linuxkernel
    SEC_OBJ_EXT := o
else
    SEC_BUILD_MODE := linuxuser
    SEC_OBJ_EXT := lo
endif


ifeq ($(HSM_SOURCE_AVAILABLE),n)
    NEXUS_SECURITY_SOURCES := $(filter %_thunks.c, $(NEXUS_SECURITY_SOURCES))
endif

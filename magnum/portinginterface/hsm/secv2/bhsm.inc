#/******************************************************************************
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
#******************************************************************************/

BHSM_BSP_INTERRUPT_SUPPORT ?= y
BHSM_ZEUS5_BFW110 ?= y

include $(MAGNUM)/portinginterface/hsm/bhsm_defs.inc

BHSM_DEFINES += BHSM_ZEUS_VER_MAJOR=$(BHSM_ZEUS_VER_MAJOR)
BHSM_DEFINES += BHSM_ZEUS_VER_MINOR=$(BHSM_ZEUS_VER_MINOR)
BHSM_DEFINES += BHSM_ZEUS_VER_SUBMINOR=$(BHSM_ZEUS_VER_SUBMINOR)
BHSM_DEFINES += BHSM_VERBOSE_BSP_TRACE
BHSM_DEFINES += BHSM_API_VERSION=2

ifeq ($(BHSM_BUILD_HSM_FOR_SAGE),y)
    BHSM_DEFINES += BHSM_BUILD_HSM_FOR_SAGE=1
    _BHSM_BUILD_HSM_FOR_SAGE=y  #not for public use.
endif

ifeq ($(BHSM_BSP_INTERRUPT_SUPPORT),y)
    BHSM_DEFINES += BHSM_BSP_INTERRUPT_SUPPORT=1
endif

ifeq ($(BHSM_ZEUS5_BFW110),y)
    BHSM_DEFINES += BHSM_ZEUS5_BFW110=1
endif

# Define this module
MAGNUM_MODULES += BHSM

# Includes
BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/secv2/include

ifeq ($(BHSM_ZEUS_VER_MAJOR),5)
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/secv2/src/zeus5
else
    BHSM_INCLUDES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/zeus42
endif

# Public Module Sources
ifneq ($(HSM_SOURCE_AVAILABLE),n)

    ifeq ($(BHSM_ZEUS_VER_MAJOR),5)
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/zeus5/bhsm_p_keyslot.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_keyladder.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_otpdatasection.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_otpmisc.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_otpmode0.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_otpmsp.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_hdcp1x.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_crypto.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_memcarch.c
        ifeq ($(BHSM_ZEUS5_BFW110),y)
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_rv_BFW110.c
        else
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_rv.c
        endif
        ifeq ($(_BHSM_BUILD_HSM_FOR_SAGE),y)
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_crypto_sage_restricted.c
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_hdcp22.c
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_hdcp22_sage_restricted.c
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_otpsoa_restricted.c
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/zeus5/bhsm_p_rv_sage_restricted.c
            ifeq ($(BHSM_ZEUS5_BFW110),y)
            else
            endif
        endif

        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_bsp_msg.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_keyslot.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_keyladder.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_otp_datasection.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_otp_msp.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_otp_key.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_hash.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_random_number.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_rsa.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_checker_biu.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_hdcp1x.c
        BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/bhsm_memarch.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_rv_rsa.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_rv_region.c
        ifeq ($(_BHSM_BUILD_HSM_FOR_SAGE),y)
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_hdcp22_restricted.c
            BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src/bhsm_rv_region_sage_restricted.c
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/bhsm_restricted_anti_rollback_sage.c
            BHSM_SOURCES += ${MAGNUM}/portinginterface/hsm/secv2/src/bhsm_memarch_restricted.c
        endif

    else
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_bsp_msg.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_keyslot.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_keyladder.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_otp_datasection.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_otp_msp.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_otp_key.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_hash.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_random_number.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_rsa.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_checker_biu.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_hdcp1x.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_rv_rsa.c
        BHSM_SOURCES += $(MAGNUM)/portinginterface/hsm/secv2/src_zeus432/bhsm_rv_region.c
    endif
endif

#############################################################################
#    (c)2013-2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
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
#############################################################################
#
# this file is included by nexus/platform/common/build/platform_modules.inc
# you can define your platform's options here

NEXUS_RFM_SUPPORT := n

ifneq ($(NEXUS_PLATFORM),973465)
ifeq ($(BCHP_VER),A0)
NEXUS_PICTURE_DECODER_SUPPORT := n
endif
endif

ifeq ($(SFF_V10),y)
NEXUS_PLATFORM_DEFINES += SFF_V10
endif

#
# 7346 Platforms
#
#
ifeq ($(NEXUS_PLATFORM),$(filter $(NEXUS_PLATFORM), 97346 973465))
ifeq ($(NEXUS_PLATFORM_VER),HMC)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_HMC
endif
ifeq ($(NEXUS_PLATFORM_VER),SFF)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_SFF
endif
ifeq ($(NEXUS_PLATFORM_VER),I2SFF)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_I2SFF
NEXUS_FRONTEND_3472 := y
endif
ifeq ($(NEXUS_PLATFORM_VER),HR44)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_HR44
NEXUS_FRONTEND_4506 := y
NEXUS_FRONTEND_4506_VER := B0
NEXUS_FRONTEND_3440 := y
endif
ifeq ($(NEXUS_PLATFORM_VER),H43)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_H43
# H43 can have different frontends.  Default to 4548 if another, supported frontend is not configured
ifneq ($(NEXUS_FRONTEND_4538),y)
ifneq ($(NEXUS_FRONTEND_4548),y)
NEXUS_FRONTEND_45216 := y
endif
endif
endif
ifeq ($(NEXUS_PLATFORM_VER),SHR44)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_SHR44
NEXUS_FRONTEND_4538 := y
NEXUS_FRONTEND_3472 := y
ifeq ($(USE_SPI_FRONTEND),y)
    USE_SPI_FRONTEND := y
    NEXUS_FRONTEND_DEFINES+=USE_SPI_FRONTEND
endif
endif
ifeq ($(NEXUS_PLATFORM_VER),SV)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97346_SV
NEXUS_FRONTEND_4506 := y
NEXUS_FRONTEND_4506_VER := B0
NEXUS_FRONTEND_3440 := y
endif
endif

#
# 7344/7418 Platforms
#
#
ifeq ($(NEXUS_PLATFORM_7344SV),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_97344SV
NEXUS_FRONTEND_4506 := y
NEXUS_FRONTEND_4506_VER := B0
NEXUS_FRONTEND_3440 := y
endif

ifeq ($(NEXUS_PLATFORM_7418SFF_H),y)
    NEXUS_FRONTEND_3128 := y
    ifeq ($(NEXUS_FRONTEND_312x_VER),)
        NEXUS_FRONTEND_312x_VER := C0
    endif
endif

ifeq ($(NEXUS_PLATFORM_7344SV),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7344SV=1
endif
ifeq ($(NEXUS_PLATFORM_7418SFF_H),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7418SFF_H=1
endif
ifeq ($(NEXUS_PLATFORM_7418SFF_RVU),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7418SFF_RVU=1
endif

#
# by default, we include the following
#
include $(NEXUS_TOP)/platforms/common/build/platform_generic_options.inc

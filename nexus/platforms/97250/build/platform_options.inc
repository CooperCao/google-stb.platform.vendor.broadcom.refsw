#############################################################################
# Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
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
# Module Description:
#
#############################################################################
#
# this file is included by nexus/platform/common/build/platform_modules.inc
# you can define your platform's options here

#Todo remove when HSM checked in.
# NEXUS_SECURITY_SUPPORT := n

# Raaga video encoder support only, no VICE
BDSP_ENCODER_SUPPORT ?= y
BDSP_ENCODER_ACCELERATOR_SUPPORT ?= y

ifeq ($(NEXUS_USE_7250_SV),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7250_SV=1
endif
ifeq ($(NEXUS_USE_7250_USFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7250_USFF=1
endif
ifeq ($(NEXUS_USE_7250_DGL),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7250_DGL=1
endif
ifeq ($(NEXUS_USE_7250_CWF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7250_CWF=1
endif
ifeq ($(NEXUS_USE_7250_ACX16),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7250_ACX16=1
endif
ifeq ($(NEXUS_USE_7250_CD2),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7250_CD2=1
endif
ifeq ($(NEXUS_USE_72501_SFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_72501_SFF=1
endif
ifeq ($(NEXUS_USE_72501_SAT),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_72501_SAT=1
endif


#
# by default, we include the following
#
include $(NEXUS_TOP)/platforms/common/build/platform_generic_options.inc

#############################################################################
# Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#############################################################################
#
# this file is included by nexus/platform/common/build/platform_modules.inc
# you can define your platform's options here

VIDEO_ENCODER_SUPPORT ?= y

ifeq (${BCHP_VER}, B0)
NEXUS_RFM_SUPPORT ?= n
endif

ifeq ($(NEXUS_HDMI_INPUT_SUPPORT),n)
NEXUS_HDMI_INPUT_SUPPORT := n
else
NEXUS_HDMI_INPUT_SUPPORT ?=y
endif

ifeq ($(NEXUS_USE_7439_SFF),y)
    ifeq ($(NEXUS_USE_3461_FRONTEND_DAUGHTER_CARD),y)
        NEXUS_FRONTEND_3461 := y
        NEXUS_PLATFORM_DEFINES += NEXUS_USE_3461_FRONTEND_DAUGHTER_CARD=1
        ifeq ($(NEXUS_FRONTEND_3461_VER),)
            NEXUS_FRONTEND_3461_VER := B0
        endif
        ifeq ($(USE_SPI_FRONTEND),y)
            USE_SPI_FRONTEND := y
            NEXUS_FRONTEND_DEFINES+=USE_SPI_FRONTEND
        endif
        NEXUS_PLATFORM_DEFINES += NEXUS_USE_7439_SFF=1
    endif
else ifeq ($(NEXUS_USE_7252S_VMS_SFF),y)
#add 7252S VMS_SFF support
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7252S_VMS_SFF=1
else ifeq ($(NEXUS_USE_7252S_SAT),y)
#add 7252S SAT support
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7252S_SAT=1
NEXUS_PLATFORM_DEFINES += NEXUS_FRONTEND_HAS_A8299_DISEQC
else ifeq ($(NEXUS_USE_7252S_GW),y)
        NEXUS_PLATFORM_DEFINES += NEXUS_USE_7252S_GW=1
	ifeq ($(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT),y)
		NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT=1
	endif
	ifeq ($(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT),y)
		NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT=1
	endif
	ifeq ($(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT),y)
		NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT=1
	endif
	ifeq ($(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT),y)
		NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_DOCSIS_IB_SUPPORT=1
	endif
else ifeq ($(NEXUS_USE_3390_VMS),y)
#add 3390 VMS support
NEXUS_PLATFORM_DEFINES += NEXUS_USE_3390_VMS=1
else ifeq ($(NEXUS_USE_7449S_CWM),y)
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7449S_CWM=1
NEXUS_FRONTEND_3158 := y
NEXUS_FRONTEND_315x_OOB := y
else
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7439_SV=1
NEXUS_PLATFORM_DEFINES += NEXUS_USE_7449_SV=1
    ifeq ($(NEXUS_USE_7439_DR3),y)
#for testing ONLY 7251S Chips on Socket boards 7449SSV_DR3 set this option
        NEXUS_PLATFORM_DEFINES += NEXUS_USE_7439_DR3=1
    endif
endif

#
# by default, we include the following
#
include $(NEXUS_TOP)/platforms/common/build/platform_generic_options.inc

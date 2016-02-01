#############################################################################
#    (c)2013 Broadcom Corporation
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

ifeq ($(NEXUS_PLATFORM_7231_FBTSFF),y)
    NEXUS_FRONTEND_3128 := y
    ifeq ($(NEXUS_FRONTEND_312x_VER),)
        NEXUS_FRONTEND_312x_VER := C0
    else
        ifneq ($(NEXUS_FRONTEND_312x_VER), C0)
            $(error Only 312x version C0 is supported for now.)
        endif
    endif
else
    ifeq ($(NEXUS_PLATFORM_7231_CSFF),y)
        NEXUS_FRONTEND_3109 := y
        NEXUS_FRONTEND_31xx_VER := B0
    else
    ifeq ($(NEXUS_PLATFORM_7231_DCSFBTSFF),y)
        NEXUS_FRONTEND_3128 := y
        ifeq ($(NEXUS_FRONTEND_312x_VER),)
            NEXUS_FRONTEND_312x_VER := C0
        else
           ifneq ($(NEXUS_FRONTEND_312x_VER), C0)
               $(error Only 312x version C0 is supported for now.)
           endif
        endif
    else
        ifeq ($(NEXUS_PLATFORM_7231_3520),y)
            NEXUS_FRONTEND_3520 := y
        else
            #for NEXUS_PLATFORM_7231_EUSFF,  NEXUS_PLATFORM_7231_EUSFF_V20 and CVBS BOARDS
            NEXUS_FRONTEND_3461 := y
            ifeq ($(NEXUS_FRONTEND_3461_VER),)
            NEXUS_FRONTEND_3461_VER := B0
            endif
        endif
    endif
    endif
endif

OTFPVR_SUPPORT := y
NEXUS_UHF_INPUT_SUPPORT := n
BDSP_ENCODER_SUPPORT ?= y

ifeq ($(NEXUS_PLATFORM_7231_FBTSFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7231_FBTSFF=1
endif
ifeq ($(NEXUS_PLATFORM_7231_CSFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7231_CSFF=1
endif
ifeq ($(NEXUS_PLATFORM_7231_EUSFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7231_EUSFF=1
endif
ifeq ($(NEXUS_PLATFORM_7231_EUSFF_V20),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7231_EUSFF_V20=1
endif
ifeq ($(NEXUS_PLATFORM_7231_3520),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7231_3520=1
endif
ifeq ($(NEXUS_PLATFORM_7231_DCSFBTSFF),y)
NEXUS_PLATFORM_DEFINES += NEXUS_PLATFORM_7231_DCSFBTSFF=1
endif

#
# by default, we include the following
#
include $(NEXUS_TOP)/platforms/common/build/platform_generic_options.inc

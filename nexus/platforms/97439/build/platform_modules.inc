#############################################################################
#    Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#############################################################################
ifeq ($(NEXUS_PLATFORM_7439_VMS),y)
    NEXUS_PLATFORM_7445_CABLE := y
    NEXUS_FRONTEND_3128 := y
    NEXUS_FRONTEND_DOCSIS := y
endif

ifeq ($(NEXUS_USE_7252S_SAT),y)
    NEXUS_FRONTEND_45216 := y
    NEXUS_FRONTEND_45308 := y
endif

ifeq ($(NEXUS_USE_7252S_VMS_SFF),y)
    NEXUS_SYSTEM_CABLE := y
    NEXUS_FRONTEND_3128 := y
#    NEXUS_FRONTEND_DOCSIS := y
endif

ifeq ($(NEXUS_USE_7252S_GW),y)
    NEXUS_SYSTEM_CABLE := y
    NEXUS_PLATFORM_DOCSIS_IB_SUPPORT :=y
    NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT :=n
    NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT :=n
	ifeq ($(NEXUS_PLATFORM_DOCSIS_PLUS_BCM3128_IB_SUPPORT),y)
		ifeq ($(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT),y)
		    NEXUS_FRONTEND_DOCSIS := y
		else
		    NEXUS_FRONTEND_3383 := y
		endif
		NEXUS_FRONTEND_3128 := y
		NEXUS_FRONTEND_312x := NEXUS_FRONTEND_3128
	endif
	ifeq ($(NEXUS_PLATFORM_DOCSIS_IB_SUPPORT),y)
		ifeq ($(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT),y)
		    NEXUS_FRONTEND_DOCSIS := y
		else
		    NEXUS_FRONTEND_3383 := y
		endif
	else
		NEXUS_FRONTEND_3128 := y
	endif
	ifeq ($(NEXUS_PLATFORM_DOCSIS_OOB_SUPPORT),y)
		ifeq ($(NEXUS_PLATFORM_DOCSIS_BCM33843_SUPPORT),y)
		    NEXUS_FRONTEND_DOCSIS := y
		else
		    NEXUS_FRONTEND_3383 := y
		endif
	else
		NEXUS_FRONTEND_312x_OOB := y
	endif
endif

ifeq ($(NEXUS_USE_3390_VMS),y)
    NEXUS_FRONTEND_3128 := n
    NEXUS_FRONTEND_DOCSIS := y
    ifeq ($(NEXUS_FRONTEND_312x_VER),)
        NEXUS_FRONTEND_312x_VER := C0
    else
        NEXUS_FRONTEND_312x_VER := $(NEXUS_FRONTEND_312x_VER)
    endif
endif

# 3128 flag covers a variety of tuners from 312x to 314x
ifeq ($(NEXUS_FRONTEND_3128),y)
    ifeq ($(NEXUS_FRONTEND_312x_VER),)
        NEXUS_FRONTEND_312x_VER := C0
    else
        NEXUS_FRONTEND_312x_VER := $(NEXUS_FRONTEND_312x_VER)
    endif
endif


include $(NEXUS_TOP)/platforms/common/build/platform_modules.inc

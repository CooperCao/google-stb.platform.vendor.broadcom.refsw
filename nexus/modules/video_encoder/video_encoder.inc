##############################################################################
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
##############################################################################

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += VIDEO_ENCODER

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_VIDEO_ENCODER_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/video_encoder/include

##########################
# Common Module Settings #
##########################
NEXUS_VIDEO_ENCODER_DEPENDENCIES += BASE CORE SECURITY DISPLAY SURFACE TRANSPORT

# thunk layer
NEXUS_VIDEO_ENCODER_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_video_encoder_thunks.c
NEXUS_VIDEO_ENCODER_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)

ifeq ($(BDSP_ENCODER_SUPPORT),y)

#######################
# DSP video encoder   #
#######################
NEXUS_VIDEO_ENCODER_DEPENDENCIES += AUDIO
NEXUS_VIDEO_ENCODER_SOURCES += $(NEXUS_TOP)/modules/video_encoder/src/dsp/nexus_video_encoder.c 
NEXUS_VIDEO_ENCODER_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/video_encoder/src/dsp

else

#######################
# VCE video encoder   #
#######################
NEXUS_VIDEO_ENCODER_SOURCES += $(NEXUS_TOP)/modules/video_encoder/src/nexus_video_encoder.c 
NEXUS_VIDEO_ENCODER_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/video_encoder/src 

ifeq ($(NEXUS_CONFIG_IMAGE),y)
ifeq ($(NEXUS_MODE),proxy)
BVCE_IMG_ONLY := y
else
BVCE_IMG_EXTERNAL := y
endif
endif

# Allow NEXUS_POWER_MANAGEMENT to be a master switch
ifeq ($(NEXUS_POWER_MANAGEMENT),n)
BVCE_POWER_MANAGEMENT := n
else
# VCE defaults off for settops, so force it on
BVCE_POWER_MANAGEMENT := y
endif

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds
# Note that we have to include vbilib and vbi just to get dccparse.
NEXUS_VIDEO_ENCODER_MAGNUM_MODULES :=  \
    $(MAGNUM)/portinginterface/vce/bvce.inc \
    $(MAGNUM)/portinginterface/vce/bvce_auth.inc \
    $(MAGNUM)/portinginterface/vce/bvce_debug.inc \
    $(MAGNUM)/commonutils/img/bimg.inc 

NEXUS_FW_SIGN_MODULES += BVCE

# Used to load the new ARC600 based encoder
ifeq ($(filter $(BCHP_CHIP), 7422 7425 7445 7145 7366 7439 74371), $(BCHP_CHIP))
NEXUS_VIDEO_ENCODER_MAGNUM_MODULES +=  \
    $(MAGNUM)/commonutils/afl/bafl.inc
endif

include ${NEXUS_TOP}/extensions/video_encoder/security/secure_ext.inc

endif

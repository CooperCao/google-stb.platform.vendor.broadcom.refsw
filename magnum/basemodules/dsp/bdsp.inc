#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
# Conditional Options for this Makefile
#
# BDSP_AUDIO_SUPPORT (default=y) - Include audio support
# BDSP_VIDEO_SUPPORT (default=n) - Include video support
# BDSP_IMG_ONLY (default=n) - Include firmware only
# BDSP_IMG_EXTERNAL (default=n) - Include non-firmware files only
#

# Only build for chips with DSP support
ifneq ($(BCHP_CHIP),7543)

# Define this module
MAGNUM_MODULES += BDSP

# Set Default conditional options
BDSP_AUDIO_SUPPORT ?= y
BDSP_VIDEO_SUPPORT ?=y
BDSP_VIDEO_ENCODE_SUPPORT ?= y
BDSP_SCM_SUPPORT ?=y

# Include standard files
BDSP_INCLUDES := $(MAGNUM)/basemodules/dsp/include

ifneq ($(BDSP_IMG_ONLY),y)
BDSP_SOURCES := \
        $(MAGNUM)/basemodules/dsp/src/bdsp.c \
        $(MAGNUM)/basemodules/dsp/src/bdsp_context.c \
        $(MAGNUM)/basemodules/dsp/src/bdsp_task.c \
        $(MAGNUM)/basemodules/dsp/src/bdsp_stage.c \
        $(MAGNUM)/basemodules/dsp/src/bdsp_priv.c

# Include conditional files
ifeq ($(BDSP_AUDIO_SUPPORT),y)
BDSP_SOURCES += $(MAGNUM)/basemodules/dsp/src/bdsp_audio_task.c
BDSP_DEFINES += BDSP_AUDIO_SUPPORT=1
endif
ifeq ($(BDSP_VIDEO_SUPPORT),y)
BDSP_SOURCES += $(MAGNUM)/basemodules/dsp/src/bdsp_video_task.c
BDSP_DEFINES += BDSP_VIDEO_SUPPORT=1
endif

ifeq ($(BDSP_VIDEO_ENCODE_SUPPORT),y)
BDSP_SOURCES += $(MAGNUM)/basemodules/dsp/src/bdsp_video_encode_task.c
BDSP_DEFINES += BDSP_VIDEO_ENCODE_SUPPORT=1
endif
ifeq ($(BDSP_SCM_SUPPORT),y)
BDSP_SOURCES += $(MAGNUM)/basemodules/dsp/src/bdsp_scm_task.c
BDSP_DEFINES += BDSP_SCM_SUPPORT=1
endif

endif #BDSP_IMG_ONLY != y

# DSP specifics
# Define the DSP type(s) used on this chipset
BDSP_RAAGA_SUPPORT := y

# Define Fp version for all supported chips
ifeq ($(BCHP_CHIP),7358)
BDSP_RAAGA_FP_VER := fp2008
endif

ifeq ($(BCHP_CHIP),7552)
ifeq ($(BCHP_VER_LOWER),a0)
BDSP_RAAGA_FP_VER := fp2008
else
BDSP_RAAGA_FP_VER := fp2011
endif
endif

### A0 build is not supported for the below chips ###
### A0 versions for the below chips are fp2008 ver ##
ifeq ($(BCHP_CHIP),7231)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7425)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7344)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7346)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),73465)
BDSP_RAAGA_FP_VER := fp2011
endif

######################################################

ifeq ($(BCHP_CHIP),7360)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7362)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),73625)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(findstring $(BCHP_CHIP), 7429 74295), $(BCHP_CHIP))
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7584)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),75845)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7435)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7439)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7445)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(findstring $(BCHP_CHIP), 7563 75635 75525), $(BCHP_CHIP))
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7145)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7439)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7366)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7364)
BDSP_RAAGA_FP_VER := fp2012
endif

ifeq ($(BCHP_CHIP),7268)
BDSP_RAAGA_FP_VER := fp2012_imx
BDSP_DEFINES += BDSP_INTR_MODE_AX_VIDEO_ENCODE
endif

ifeq ($(BCHP_CHIP),7271)
BDSP_RAAGA_FP_VER := fp2012_imx
BDSP_DEFINES += BDSP_INTR_MODE_AX_VIDEO_ENCODE
endif

ifeq ($(BCHP_CHIP),7250)
BDSP_RAAGA_FP_VER := fp2012
endif

ifeq ($(BCHP_CHIP),7228)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),74371)
BDSP_RAAGA_FP_VER := fp2011
endif

ifeq ($(BCHP_CHIP),7260)
BDSP_RAAGA_FP_VER := fp2012_imx
BDSP_DEFINES += BDSP_INTR_MODE_AX_VIDEO_ENCODE
endif

ifeq ($(BCHP_CHIP),7278)
BDSP_RAAGA_FP_VER := fp4015
endif

ifeq ($(BDSP_ARM_AUDIO_SUPPORT),y)
ifeq ($(BDSP_RAAGA_AUDIO_SUPPORT),n)
include $(MAGNUM)/basemodules/dsp/build/bdsp_arm_v2.inc
include $(MAGNUM)/basemodules/dsp/build/bdsp_common_v2.inc
else
include $(MAGNUM)/basemodules/dsp/build/bdsp_arm.inc
endif
endif

ifneq ($(BDSP_RAAGA_AUDIO_SUPPORT),n)
ifneq ($(BDSP_RAAGA_FP_VER),fp4015)
include $(MAGNUM)/basemodules/dsp/build/bdsp_raaga.inc
include $(MAGNUM)/basemodules/dsp/build/bdsp_common.inc
else
include $(MAGNUM)/basemodules/dsp/build/bdsp_octave.inc
include $(MAGNUM)/basemodules/dsp/build/bdsp_common_v2.inc
endif
endif
endif #ifneq ($(BCHP_CHIP),7543)

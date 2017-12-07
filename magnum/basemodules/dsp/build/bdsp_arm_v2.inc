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

#
# Derive the architecture of ARM compilation to determine the firmware version
#
ifeq ($(B_REFSW_ARCH),arm-linux)
AONA_VER = v7
else ifeq ($(B_REFSW_ARCH),aarch64-linux)
AONA_VER = v8
endif

# Include standard files
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/include/arm_v2
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/include/common_v2

# Include standard files
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)

# Include standard files
ifneq ($(BDSP_IMG_ONLY),y)
BDSP_SOURCES += \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_fwdownload_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_fwinterface_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_mm_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_command_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_fw_algo.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_int_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_io_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_cit_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_cit_log.c \
        $(MAGNUM)/basemodules/dsp/src/arm_v2/bdsp_arm_cit_log_ext.c
endif

# Set CFLAG for ARM Support
BDSP_DEFINES += BDSP_ARM_AUDIO_SUPPORT=1

# Include firmware files
include $(MAGNUM)/basemodules/dsp/build/bdsp_arm_v2_img.inc

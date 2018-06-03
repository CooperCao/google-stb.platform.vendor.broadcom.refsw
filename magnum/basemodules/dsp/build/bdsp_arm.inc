#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################


#
# Conditional Options for this Makefile
#
#

ifeq ($(B_REFSW_ARCH),arm-linux)
AONA_VER = v7
else ifeq ($(B_REFSW_ARCH),aarch64-linux)
AONA_VER = v8
endif

# Set CFLAG for ARM Support
BDSP_DEFINES += BDSP_ARM_AUDIO_SUPPORT=1

# Include standard files
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/include/arm
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/include/common
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/common
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/arm/firmware/$(AONA_VER)
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src

BDSP_ARM_DDPENC_SUPPORT ?= y

# Include standard files
ifneq ($(BDSP_IMG_ONLY),y)
BDSP_SOURCES += \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_dsp_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_mm_priv.c \
		$(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_priv.c \
		$(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_cit_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_cit_log.c \
		$(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_cit_log_ext.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_fwdownload_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_fw_algo.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_int_priv.c \
        $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_fwinterface_priv.c \
	    $(MAGNUM)/basemodules/dsp/src/arm/bdsp_arm_fw_settings.c
endif

########################################################################
# Added lines for tzioc integration
########################################################################

#BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/arm/tzioc/
#BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/arm/libtzioc/

#ifneq ($(BDSP_IMG_ONLY),y)
#BDSP_SOURCES += \
#        $(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_peer.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_msg.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_mem.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_ioctl.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_drv.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_client.c \
#		$(MAGNUM)/basemodules/dsp/src/arm/tzioc/tzioc_api.c

#BDSP_SOURCES += \
#        $(MAGNUM)/basemodules/dsp/src/arm/libtzioc/libtzioc.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/libtzioc/libtzioc_api.c \
#        $(MAGNUM)/basemodules/dsp/src/arm/libtzioc/libtzioc_ioctl.c
#endif

########################################################################
# Added lines for tzioc integration
########################################################################

# Include firmware files
include $(MAGNUM)/basemodules/dsp/build/bdsp_arm_img.inc

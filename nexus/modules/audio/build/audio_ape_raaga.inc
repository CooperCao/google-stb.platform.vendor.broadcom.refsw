##############################################################################
#  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

######################
# Module Definitions #
######################
ifeq ($(NEXUS_CONFIG_IMAGE),y)
ifeq ($(NEXUS_MODE),proxy)
BDSP_IMG_ONLY := y
BAPE_IMG_ONLY := y
else
BDSP_IMG_EXTERNAL := y
BAPE_IMG_EXTERNAL := y
endif
endif

##################
# Magnum Modules #
##################
ifneq ($(BAPE_DSP_SUPPORT),n)
NEXUS_AUDIO_MAGNUM_MODULES = \
	$(MAGNUM)/basemodules/dsp/bdsp.inc \
	$(MAGNUM)/basemodules/dsp/bdsp_auth.inc \
	$(MAGNUM)/portinginterface/ape/bape.inc
else
NEXUS_AUDIO_MAGNUM_MODULES = \
	$(MAGNUM)/portinginterface/ape/bape.inc
endif

NEXUS_FW_SIGN_MODULES += BDSP

ifneq ($(BAPE_DSP_SUPPORT),n)
#####################
# Module Extensions #
#####################
ifneq ($(NEXUS_AUDIO_EXTENSION_INC),)
include $(NEXUS_AUDIO_EXTENSION_INC)
endif
ifeq ($(NEXUS_AUDIO_BUFFER_CAPTURE),y)
include $(NEXUS_TOP)/../rockford/unittests/nexus/audio/buffer_capture/buffer_capture.inc
endif
ifeq ($(NEXUS_AUDIO_RAVE_MONITOR),y)
include $(NEXUS_TOP)/../rockford/unittests/nexus/audio/xpt_monitor/rave_monitor.inc
endif
ifeq ($(BDSP_VP6_SUPPORT),y)
include $(NEXUS_TOP)/extensions/audio/dsp_video_decoder/dsp_video_decoder.inc
endif
ifeq ($(BDSP_ENCODER_SUPPORT),y)
include $(NEXUS_TOP)/extensions/audio/dsp_video_encoder/dsp_video_encoder.inc
endif
endif

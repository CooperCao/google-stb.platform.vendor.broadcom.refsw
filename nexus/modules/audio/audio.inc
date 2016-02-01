############################################################
#     (c)2010-2013 Broadcom Corporation
#  
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
############################################################

# Determine audio family
ifeq ($(BCHP_CHIP),7400)
NEXUS_AUDIO_FAMILY := rap_v1
else
ifeq ($(BCHP_CHIP),7408)
NEXUS_AUDIO_FAMILY := ape_mips
else
ifeq ($(BCHP_CHIP),$(findstring $(BCHP_CHIP), 7125 7325 7335 7336 7340 7342 7420 7468 7550 7405))
NEXUS_AUDIO_FAMILY := rap_v3
else
NEXUS_AUDIO_FAMILY := ape_raaga
endif
endif
endif
# determine whether primer is supported
ifeq ($(NEXUS_AUDIO_FAMILY),$(findstring $(NEXUS_AUDIO_FAMILY), rap_v1 rap_v3 ape_mips))
NEXUS_AUDIO_PRIMER := nexus_audio_decoder_primer_stubs.c
else
NEXUS_AUDIO_PRIMER := nexus_audio_decoder_primer.c
endif

######################
# Define this module #
######################
NEXUS_MODULES += AUDIO

###################
# Module Includes #
###################
NEXUS_AUDIO_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/audio/include/$(NEXUS_AUDIO_FAMILY) $(NEXUS_TOP)/modules/audio/include 
NEXUS_AUDIO_PRIVATE_INCLUDES := $(NEXUS_TOP)/modules/audio/src/$(NEXUS_AUDIO_FAMILY) $(NEXUS_TOP)/modules/audio/src/common

NEXUS_AUDIO_PRECOMPILED_HEADER = $(NEXUS_TOP)/modules/audio/src/$(NEXUS_AUDIO_FAMILY)/nexus_audio_module.h                                    

##################
# Module Sources #
##################
NEXUS_AUDIO_MODULE_PATH := $(NEXUS_TOP)/modules/audio/src/$(NEXUS_AUDIO_FAMILY)
NEXUS_AUDIO_MODULE_COMMON := $(NEXUS_TOP)/modules/audio/src/common

##################
# Module Sources #
##################
NEXUS_AUDIO_SOURCES := \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_3d_surround.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_ac3_encode.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_capture.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_crc.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_dac.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_decoder.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_decode_to_memory.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_decoder_trick.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_debug.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_dsp.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_dummy_output.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_encoder.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_equalizer.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_input.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_input_capture.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_mixer.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_module.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_mux_output.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_output.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_output_group.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_playback.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_processor.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_audio_return_channel.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_auto_volume_level.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_dolby_volume.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_dolby_digital_reencode.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_dts_encode.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_echo_canceller.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_i2s_input.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_i2s_output.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_rf_audio_encoder.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_spdif_input.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_spdif_output.c \
    ${NEXUS_AUDIO_MODULE_PATH}/nexus_tru_volume.c

NEXUS_AUDIO_SOURCES += \
    ${NEXUS_AUDIO_MODULE_COMMON}/${NEXUS_AUDIO_PRIMER}

#######################
# Module Dependencies #
#######################
NEXUS_AUDIO_DEPENDENCIES := BASE CORE TRANSPORT HDMI_OUTPUT RFM SECURITY HDMI_INPUT

###############
# Thunk Layer #
###############
NEXUS_AUDIO_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
NEXUS_AUDIO_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_audio_thunks.c

#####################
# Version Specifics #
#####################
-include $(NEXUS_TOP)/modules/audio/build/audio_$(NEXUS_AUDIO_FAMILY).inc


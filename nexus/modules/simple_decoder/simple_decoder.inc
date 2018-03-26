############################################################
#  Copyright (C) 2007-2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
############################################################

#
# Conditional Options for this Makefile
#

######################
# Define this module #
######################
NEXUS_MODULES += SIMPLE_DECODER

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_SIMPLE_DECODER_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/simple_decoder/include
NEXUS_SIMPLE_DECODER_PRECOMPILED_HEADER := $(NEXUS_TOP)/modules/simple_decoder/src/nexus_simple_decoder_module.h

##################
# Module Sources #
##################
NEXUS_SIMPLE_DECODER_SOURCES := \
    $(NEXUS_TOP)/modules/simple_decoder/src/nexus_simple_video_decoder.c \
    $(NEXUS_TOP)/modules/simple_decoder/src/nexus_simple_audio_decoder.c \
    $(NEXUS_TOP)/modules/simple_decoder/src/nexus_simple_stc_channel.c \
    $(NEXUS_TOP)/modules/simple_decoder/src/nexus_simple_decoder_module.c \
    $(NEXUS_TOP)/modules/simple_decoder/src/nexus_simple_encoder.c

# thunk layer
NEXUS_SIMPLE_DECODER_PRIVATE_INCLUDES += \
    $(NEXUS_TOP)/modules/simple_decoder/src $(NEXUS_SYNCTHUNK_DIR)

NEXUS_SIMPLE_DECODER_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_simple_decoder_thunks.c

#######################
# Module Dependencies #
#######################
NEXUS_SIMPLE_DECODER_DEPENDENCIES += VIDEO_DECODER DISPLAY AUDIO GRAPHICS2D ASTM SYNC_CHANNEL VIDEO_ENCODER STREAM_MUX SIMPLE_AUDIO_PLAYBACK

NEXUS_SIMPLE_DECODER_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/tshdrbuilder/tshdrbuilder.inc
    
NEXUS_EXTRA_MODULES += TSHDRBUILDER

######################
# Module Definitions #
######################

#####################
# Module Extensions #
#####################

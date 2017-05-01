############################################################
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
#
############################################################

NEXUS_TOP := $(shell cd ../../../../nexus && pwd)

APPS = \
    audio_arc_input \
    audio_arc_output \
    audio_decode_test \
    audio_drip_decode \
    audio_hdmi_input_edid_ram \
    audio_img_load \
    audio_mixer \
    audio_mixer_input_stress \
    audio_playback \
    audio_test_auto \
    codec_test \
    i2s_input \
    i2s_input_decoder \
    audio_wma_tsm  \
    echo_canceller \
    playback_pip \
    audio_hdmi_input_encode \
    app_audio

APPS_DEP = $(APPS:%=%.d)

# audio_test requires user input

# For app_audio to play asf/avi files
BSEAV = $(NEXUS_TOP)/../BSEAV
include $(NEXUS_TOP)/build/nexus_defs.inc
include $(BSEAV)/lib/bfile/bfile.inc
ifeq ($(MEDIA_AVI_SUPPORT),y)
include $(BSEAV)/lib/avi/bavi.inc
endif
ifeq ($(MEDIA_ASF_SUPPORT),y)
include $(BSEAV)/lib/asf/basf.inc
endif

include $(BSEAV)/lib/utils/batom.inc
include $(BSEAV)/lib/media/bmedia.inc
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES) $(BFILE_MEDIA_INCLUDES)) -I$(BSEAV)/lib/mpeg2_ts_parse
CFLAGS += $(addprefix -D,$(BMEDIA_PROBE_DEFINES))

# For Enabling ring buffer capture # 
MAGNUM = $(NEXUS_TOP)/../magnum
CFLAGS += -I$(MAGNUM)/basemodules/chp/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER) -I$(MAGNUM)/basemodules/mem
ifeq ($(RAP_RBUF_CAP_SUPPORT),y)
CFLAGS += -DBDSP_FW_RBUF_CAPTURE
endif


# Adding support for building app_audio for AVL,3d,DV258 audio PPs under single export. Refer SW7425-5635
ifeq ($(RAP_IQA_TEST_SUPPORT),y)
#support for AVL
RAP_AVL_SUPPORT = y
#support for 3D Surround
RAP_BRCM_3DSURROUND_SUPPORT = y
#support for Srs tru volume
RAP_SRSTRUVOL_SUPPORT = y
#support for DV258
RAP_DV258_SUPPORT = y
endif


# For Enabling Auto Vol Leveling (BRCM AVL) # 
ifeq ($(RAP_AVL_SUPPORT),y)
CFLAGS += -DNEXUS_AVL_ENABLE
endif

# For Enabling Auto Vol Leveling (Phillips AVL) # 
ifeq ($(RAP_AVLP_SUPPORT),y)
CFLAGS += -DNEXUS_AVLP_ENABLE
endif

# For Enabling 3D Surround (BRCM 3D Surround) # 
ifeq ($(RAP_BRCM_3DSURROUND_SUPPORT),y)
CFLAGS += -DNEXUS_3DSURROUND_ENABLE
endif

# For Enabling Audyssey Volume # 
ifeq ($(RAP_ADV_SUPPORT),y)
CFLAGS += -DNEXUS_ADV_ENABLE
endif

# For Enabling Audyssey ABX # 
ifeq ($(RAP_ABX_SUPPORT),y)
CFLAGS += -DNEXUS_ABX_ENABLE
endif

# For SRS code studio # 
ifeq ($(RAP_SRSSTUDIOSOUND_SUPPORT),y)
CFLAGS += -DNEXUS_SRSSTUDIOSOUND_ENABLE
endif
# For SRS TVOL # 
ifeq ($(RAP_SRSTRUVOL_SUPPORT),y)
CFLAGS += -DNEXUS_SRSTRUVOL_ENABLE
endif
# For SRS TSHD # 
ifeq ($(RAP_SRSHD_SUPPORT),y)
CFLAGS += -DNEXUS_SRSHD_ENABLE
endif

# For DV258 support
ifeq ($(RAP_DV258_SUPPORT),y)
CFLAGS += -DNEXUS_DV258_ENABLE
CFLAGS += -DRAP_DV258_SUPPORT
CFLAGS += -DUSE_DV258
endif

# For MS11 support # 
ifeq ($(RAP_MS11_SUPPORT),y)
CFLAGS += -DNEXUS_MS11_SUPPORT
endif

ifneq ($(NEXUS_AUDIO_EXTENSION_INC),)
CFLAGS += -DCUSTOM_VOICE_ENABLE
endif

include ../common/Makefile.inc

#-include $(APPS_DEP)
$(BINDIR)/app_audio : rbuf_capture.h

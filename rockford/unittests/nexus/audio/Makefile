############################################################
#     Copyright (c) 2003-2013, Broadcom Corporation
#     All Rights Reserved
#     Confidential Property of Broadcom Corporation
#
#  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
#  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
#  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
#
# $brcm_Workfile: $
# $brcm_Revision: $
# $brcm_Date: $
#
# Module Description:
#
# Revision History:
#
# Created: 08/28/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

ifdef B_REFSW_REAL_NEXUS_TOP
NEXUS_TOP ?= ${B_REFSW_REAL_NEXUS_TOP}
endif
ifdef COMSPEC
# Any DOS environment
NEXUS_TOP ?= $(shell cd ../../../../nexus && cd)
else
NEXUS_TOP ?= $(shell cd ../../../../nexus; pwd)
endif

APPS = \
    audio_arc_input \
    audio_arc_output \
    audio_decode_test \
    audio_drip_decode \
    audio_hdmi_input_edid_ram \
    audio_img_load \
    audio_mixer \
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
CFLAGS += $(addprefix -I,$(BMEDIA_INCLUDES) $(BFILE_MEDIA_INCLUDES)) -I$(BSEAV)/lib/mpeg2_ts_parse -I$(BSEAV)/lib/tspsi
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

include ${NEXUS_TOP}/../rockford/unittests/nexus/common/Makefile.inc

#-include $(APPS_DEP)
app_audio : $(NEXUS_TOP)/../rockford/unittests/nexus/audio/rbuf_capture.h

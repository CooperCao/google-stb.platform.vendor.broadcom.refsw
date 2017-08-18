 ###############################################################################
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
 ##############################################################################

#
# Conditional Options for this Makefile
#
# BDSP_AUDIO_SUPPORT (default=y) - Include audio support
# BDSP_VIDEO_SUPPORT (default=n) - Include video support
# BDSP_IMG_ONLY (default=n) - Include firmware only
# BDSP_IMG_EXTERNAL (default=n) - Include non-firmware files only
#

# Option to disable all/3rd-party algo support
BDSP_DEFAULT_ALGO_SUPPORT ?= y
BDSP_3RDPARTY_ALGO_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)

# Include standard files
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)

#
# All Firmware sources must be in BDSP_IMG_SOURCES, not BDSP_SOURCES.  Based on BDSP_IMG_EXTERNAL, choose to include the firmware files or not at the end.
#
BDSP_IMG_SOURCES += $(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/bdsp_raaga_img.c

ifeq ($(BCHP_CHIP), 7278)
BDSP_IMG_SOURCES += \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_rdbvars_${BCHP_CHIP}.c \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_fpos_kernel.c \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_init_romfs.c\
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_system_code.c\
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_system_lib.c\
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_version.c
endif

#################################################################
#		ADD AUDIO ALGORITHM SUPPORT			#
#################################################################

##########################################
#		DECODER			 #
##########################################

#
#MPEG DECODE
#
ifneq ($(BDSP_MPEG_SUPPORT),n)
BDSP_MPEG_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_mpeg1.c
BDSP_MPEG_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_mpeg1.c

ifneq ($(wildcard $(BDSP_MPEG_SOURCES)),)
ifneq ($(wildcard $(BDSP_MPEG_IDS_SOURCES)),)
BDSP_MPEG_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_MPEG_SUPPORT),y)
BDSP_DEFINES += BDSP_MPEG_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MPEG_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_MPEG_IDS_SOURCES)
endif

#
#PCMWAV DECODE
#
ifneq ($(BDSP_PCMWAV_SUPPORT),n)
BDSP_PCMWAV_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_pcmwav.c
BDSP_PCMWAV_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_wavformatex.c

ifneq ($(wildcard $(BDSP_PCMWAV_SOURCES)),)
ifneq ($(wildcard $(BDSP_PCMWAV_IDS_SOURCES)),)
BDSP_PCMWAV_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_PCMWAV_SUPPORT),y)
BDSP_DEFINES += BDSP_PCMWAV_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_PCMWAV_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_PCMWAV_IDS_SOURCES)
endif

#
#UDC DECODE
#
BDSP_UDC_SUPPORT = n
ifneq ($(BDSP_UDC_SUPPORT),n)
BDSP_UDC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_udc.c
BDSP_UDC_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ddp.c

ifneq ($(wildcard $(BDSP_UDC_SOURCES)),)
ifneq ($(wildcard $(BDSP_UDC_IDS_SOURCES)),)
BDSP_UDC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_UDC_SUPPORT),y)
BDSP_DEFINES += BDSP_UDC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_UDC_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_UDC_IDS_SOURCES)
endif

#
#DDP DECODE
#
ifneq ($(BDSP_DDP_SUPPORT),n)
BDSP_DDP_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ddp.c
BDSP_DDP_IDS_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ddp.c

ifneq ($(wildcard $(BDSP_DDP_SOURCES)),)
ifneq ($(wildcard $(BDSP_DDP_IDS_SOURCES)),)
BDSP_DDP_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DDP_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user
BDSP_DDP_PASSTHRU_SUPPORT ?= y
BDSP_DEFINES += BDSP_DDP_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDP_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_DDP_IDS_SOURCES)
endif

#
#DDP Passthrough
#
ifneq ($(BDSP_DDP_PASSTHRU_SUPPORT),n)
BDSP_DDP_PASSTHRU_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_passthru.c
BDSP_DDP_PASSTHRU_IDS_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ddp.c

ifneq ($(wildcard $(BDSP_DDP_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_DDP_PASSTHRU_IDS_SOURCES)),)
BDSP_DDP_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DDP_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_DDP_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDP_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_DDP_PASSTHRU_IDS_SOURCES)
endif

#
#AC3 DECODE
#
ifneq ($(BDSP_DDP_SUPPORT),y)
ifneq ($(BDSP_AC3_SUPPORT),n)
BDSP_AC3_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ac3.c
BDSP_AC3_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ddp.c

ifneq ($(wildcard $(BDSP_AC3_SOURCES)),)
ifneq ($(wildcard $(BDSP_AC3_IDS_SOURCES)),)
BDSP_AC3_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif
endif

ifeq ($(BDSP_AC3_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user
BDSP_AC3_PASSTHRU_SUPPORT ?= y
BDSP_DEFINES += BDSP_AC3_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AC3_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_AC3_IDS_SOURCES)
endif

#
#AC3 Passthrough
#
ifneq ($(BDSP_AC3_PASSTHRU_SUPPORT),n)
BDSP_AC3_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_passthru.c
BDSP_AC3_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ddp.c

ifneq ($(wildcard $(BDSP_AC3_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_AC3_PASSTHRU_IDS_SOURCES)),)
BDSP_AC3_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_AC3_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_AC3_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AC3_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_AC3_PASSTHRU_IDS_SOURCES)
endif

#
#AAC DECODE
#
ifneq ($(BDSP_AACSBR_SUPPORT),n)
BDSP_AACSBR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_aache.c
BDSP_AACSBR_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_adts.c \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_loas.c \

ifneq ($(wildcard $(BDSP_AACSBR_SOURCES)),)
ifneq ($(wildcard $(BDSP_AACSBR_IDS_SOURCES)),)
BDSP_AACSBR_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_AACSBR_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user
BDSP_AACSBR_PASSTHRU_SUPPORT ?= y
BDSP_DEFINES += BDSP_AACSBR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AACSBR_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_AACSBR_IDS_SOURCES)
endif

#
#AAC Passthrough
#
ifneq ($(BDSP_AACSBR_PASSTHRU_SUPPORT),n)
BDSP_AACSBR_PASSTHRU_SOURCES :=	\
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_passthru.c
BDSP_AACSBR_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_adts.c \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_loas.c \

ifneq ($(wildcard $(BDSP_AACSBR_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_AACSBR_PASSTHRU_IDS_SOURCES)),)
BDSP_AACSBR_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_AACSBR_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_AACSBR_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AACSBR_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_AACSBR_PASSTHRU_IDS_SOURCES)
endif

##########################################
#		POST PROCESS             #
##########################################

#
#SRC PP
#
ifneq ($(BDSP_SRC_SUPPORT),n)
BDSP_SRC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/octave/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_src.c

ifneq ($(wildcard $(BDSP_SRC_SOURCES)),)
BDSP_SRC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SRC_SUPPORT),y)
BDSP_DEFINES += BDSP_SRC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SRC_SOURCES)
endif

ifneq ($(BDSP_IMG_EXTERNAL),y)
BDSP_DEFINES += BDSP_IMG_SUPPORT=1
BDSP_SOURCES += $(BDSP_IMG_SOURCES)
endif

BDSP_SOURCES := $(shell echo $(BDSP_SOURCES) | tr ' ' '\n' | awk '!a[$$0]++')

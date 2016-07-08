 ###############################################################################
 # Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 #
 # This program is the proprietary software of Broadcom and/or its
 # licensors, and may only be used, duplicated, modified or distributed pursuant
 # to the terms and conditions of a separate, written license agreement executed
 # between you and Broadcom (an "Authorized License").  Except as set forth in
 # an Authorized License, Broadcom grants no license (express or implied), right
 # to use, or waiver of any kind with respect to the Software, and Broadcom
 # expressly reserves all rights in and to the Software and all intellectual
 # property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 # HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 # NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 #
 # Except as expressly set forth in the Authorized License,
 #
 # 1. This program, including its structure, sequence and organization,
 #    constitutes the valuable trade secrets of Broadcom, and you shall use all
 #    reasonable efforts to protect the confidentiality thereof, and to use
 #    this information only in connection with your use of Broadcom integrated
 #    circuit products.
 #
 # 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 #    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 #    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 #    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 #    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 #    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 #    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 #    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 #
 # 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 #    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 #    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 #    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 #    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 #    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 #    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 #    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)

#
# All Firmware sources must be in BDSP_IMG_SOURCES, not BDSP_SOURCES.  Based on BDSP_IMG_EXTERNAL, choose to include the firmware files or not at the end.
#
BDSP_IMG_SOURCES += $(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/bdsp_raaga_img.c

# Vom table files should not be part of BDSP_IMG_SOURCES
ifneq ($(BDSP_IMG_ONLY),y)
BDSP_SOURCES += \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_vom_algo_start_addr.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_vom_table.c
endif

# Determine defines based on presence of files.  E.g. BDSP_AC3_SUPPORT, BDSP_DDP_SUPPORT, BDSP_MS11_SUPPORT, etc.

# Based on algo defines, include proper firmware files.
# Add firmware files here
BDSP_IMG_SOURCES += \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_rdbvars_${BCHP_CHIP}.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_sys_code.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_lib_sys.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_lib_algo.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_common.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_vids_common.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_vids_common_inter_frame.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_stask_scm.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_stask_video_decode.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_stask_video_encode.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_apt_generic.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_version.c

#################################################################
#		ADD AUDIO ALGORITHM SUPPORT			#
#################################################################
#

# Enabling the passthru if decode is enabled
ifeq ($(BDSP_MPEG_SUPPORT),y)
BDSP_MPEG_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_DDP_SUPPORT),y)
BDSP_DDP_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_AC3_SUPPORT),y)
BDSP_AC3_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_UDC_SUPPORT),y)
BDSP_UDC_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_AACSBR_SUPPORT),y)
BDSP_AACSBR_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_DTSHD_SUPPORT),y)
BDSP_DTSHD_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_DTSLBR_SUPPORT),y)
BDSP_DTSLBR_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_DTSBROADCAST_SUPPORT),y)
BDSP_DTSBROADCAST_PASSTHRU_SUPPORT = y
endif
ifeq ($(BDSP_DRA_SUPPORT),y)
BDSP_DRA_PASSTHRU_SUPPORT = y
endif


#MS12 not Support for Raaga Lite Chips
ifeq ($(BDSP_RAAGA_FP_VER),fp2008)
BDSP_MS12_SUPPORT = n
BDSP_DOLBY_DCV_SUPPORT = y
endif

ifneq ($(filter $(BDSP_MS12_SUPPORT),A B C D Y a b c d y),)
BDSP_UDC_SUPPORT = y
BDSP_AC4_SUPPORT = y
BDSP_DPCMR_SUPPORT = y
BDSP_DDPENC_SUPPORT = y
BDSP_MIXERDAPV2_SUPPORT = y
BDSP_DOLBY_AACHE_SUPPORT = y
BDSP_MS10_SUPPORT = n
BDSP_MS11_SUPPORT = n
BDSP_AACSBR_SUPPORT = n
BDSP_DDP_SUPPORT = n
BDSP_AC3_SUPPORT = n
BDSP_DDRE_SUPPORT = n
BDSP_AC3ENC_SUPPORT = n
BDSP_DDP_PASSTHRU_SUPPORT = n
BDSP_AC3_PASSTHRU_SUPPORT = n
BDSP_AACSBR_PASSTHRU_SUPPORT ?= y
BDSP_UDC_PASSTHRU_SUPPORT = y

ifeq ($(BDSP_MS12_SUPPORT),y)
BDSP_MS12_SUPPORT = B
endif

ifeq ($(BDSP_MS12_SUPPORT),A)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=65
else ifeq ($(BDSP_MS12_SUPPORT),a)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=65
else ifeq ($(BDSP_MS12_SUPPORT),B)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=66
else ifeq ($(BDSP_MS12_SUPPORT),b)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=66
else ifeq ($(BDSP_MS12_SUPPORT),C)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=67
else ifeq ($(BDSP_MS12_SUPPORT),c)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=67
else ifeq ($(BDSP_MS12_SUPPORT),D)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=68
else ifeq ($(BDSP_MS12_SUPPORT),d)
	BDSP_DEFINES += BDSP_MS12_SUPPORT=68
else
	BDSP_DEFINES += BDSP_MS12_SUPPORT=67
endif
endif

#
#DDP Passthrough
#
ifneq ($(BDSP_DDP_PASSTHRU_SUPPORT),n)
BDSP_DDP_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ddp.c

ifneq ($(wildcard $(BDSP_DDP_PASSTHRU_SOURCES)),)
BDSP_DDP_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DDP_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_DDP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDP_PASSTHRU_SOURCES)
endif

#
#AC3 Passthrough
#
ifneq ($(BDSP_AC3_PASSTHRU_SUPPORT),n)
BDSP_AC3_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ac3.c

ifneq ($(wildcard $(BDSP_AC3_PASSTHRU_SOURCES)),)
BDSP_AC3_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AC3_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_AC3_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AC3_PASSTHRU_SOURCES)
endif


#
#AACSBR Passthrough
#
ifneq ($(BDSP_AACSBR_PASSTHRU_SUPPORT),n)
BDSP_AACSBR_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_aache.c

ifneq ($(wildcard $(BDSP_AACSBR_PASSTHRU_SOURCES)),)
BDSP_AACSBR_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AACSBR_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_AACSBR_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AACSBR_PASSTHRU_SOURCES)
endif


ifeq ($(BDSP_MS12_SUPPORT),n)
BDSP_UDC_SUPPORT = n
BDSP_AC4_SUPPORT = n
BDSP_DPCMR_SUPPORT = n
BDSP_DDPENC_SUPPORT = n
BDSP_DOLBY_AACHE_SUPPORT = n
BDSP_MIXERDAPV2_SUPPORT = n
BDSP_UDC_PASSTHRU_SUPPORT = n
endif

#
#MS11 - Support
#
ifeq ($(BDSP_MS11_SUPPORT),y)
BDSP_MS10_SUPPORT = y
BDSP_DDRE_SUPPORT = y
BDSP_DV258_SUPPORT ?= y
BDSP_DDP_PASSTHRU_SUPPORT = y
BDSP_AACSBR_PASSTHRU_SUPPORT = y
endif

#
#MS10 - Support
#
ifneq ($(BDSP_MS10_SUPPORT),n)
BDSP_MS10_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dolby_ms_ddp.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dolby_pulse.c

ifneq ($(wildcard $(BDSP_MS10_SOURCES)),)
ifneq ($(wildcard $(BDSP_DDP_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_AC3_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_AACSBR_PASSTHRU_SOURCES)),)
BDSP_MS10_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif
endif
endif

ifeq ($(BDSP_MS10_SUPPORT),y)
BDSP_DEFINES += BDSP_MS10_SUPPORT
BDSP_DEFINES += BDSP_AACSBR_SUPPORT
BDSP_DEFINES += BDSP_DDP_SUPPORT
BDSP_DEFINES += BDSP_AC3_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MS10_SOURCES)
endif


#
#DDP (non-MS10)
#
ifneq ($(BDSP_MS10_SUPPORT),y)
ifneq ($(BDSP_DDP_SUPPORT),n)
BDSP_DDP_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ddp.c

ifneq ($(wildcard $(BDSP_DDP_SOURCES)),)
ifneq ($(wildcard $(BDSP_DDP_PASSTHRU_SOURCES)),)
BDSP_DDP_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
BDSP_AC3_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif
endif

ifeq ($(BDSP_DDP_SUPPORT),y)
BDSP_DEFINES += BDSP_DDP_SUPPORT
BDSP_DEFINES += BDSP_AC3_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDP_SOURCES)
endif

#
#AC3 (non-MS10)
#
ifneq ($(BDSP_MS10_SUPPORT),y)
ifneq ($(BDSP_DDP_SUPPORT),y)
ifneq ($(BDSP_AC3_SUPPORT),n)
BDSP_AC3_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ac3.c

ifneq ($(wildcard $(BDSP_AC3_SOURCES)),)
ifneq ($(wildcard $(BDSP_AC3_PASSTHRU_SOURCES)),)
BDSP_AC3_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif
endif

ifeq ($(BDSP_AC3_SUPPORT),y)
BDSP_DEFINES += BDSP_AC3_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AC3_SOURCES)
endif
endif

#
#AACSBR (non-MS10)
#
ifneq ($(BDSP_MS10_SUPPORT),y)
ifneq ($(BDSP_AACSBR_SUPPORT),n)
BDSP_AACSBR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_aache.c

ifneq ($(wildcard $(BDSP_AACSBR_SOURCES)),)
ifneq ($(wildcard $(BDSP_AACSBR_PASSTHRU_SOURCES)),)
BDSP_AACSBR_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif
endif

ifeq ($(BDSP_AACSBR_SUPPORT),y)
BDSP_DEFINES += BDSP_AACSBR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AACSBR_SOURCES)
endif


#
#MPEG Passthrough
#
ifneq ($(BDSP_MPEG_PASSTHRU_SUPPORT),n)
BDSP_MPEG_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_mpeg1.c

ifneq ($(wildcard $(BDSP_MPEG_PASSTHRU_SOURCES)),)
BDSP_MPEG_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_MPEG_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_MPEG_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MPEG_PASSTHRU_SOURCES)
endif



#
#MPEG DECODE
#
ifneq ($(BDSP_MPEG_SUPPORT),n)
BDSP_MPEG_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_mpeg1.c

ifneq ($(wildcard $(BDSP_MPEG_SOURCES)),)
ifneq ($(wildcard $(BDSP_MPEG_PASSTHRU_SOURCES)),)
BDSP_MPEG_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_MPEG_SUPPORT),y)
BDSP_DEFINES += BDSP_MPEG_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MPEG_SOURCES)
endif


#
#WMA DECODE
#
ifneq ($(BDSP_WMA_SUPPORT),n)
BDSP_WMA_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_wma.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_wma.c

ifneq ($(wildcard $(BDSP_WMA_SOURCES)),)
BDSP_WMA_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_WMA_SUPPORT),y)
BDSP_DEFINES += BDSP_WMA_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_WMA_SOURCES)
endif

#
#WMAPRO DECODE
#
ifneq ($(BDSP_WMAPRO_SUPPORT),n)
BDSP_WMAPRO_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_wma_pro.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_wma_pro.c

ifneq ($(wildcard $(BDSP_WMAPRO_SOURCES)),)
BDSP_WMAPRO_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_WMAPRO_SUPPORT),y)
BDSP_DEFINES += BDSP_WMAPRO_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_WMAPRO_SOURCES)
endif

#
#LPCMDVD DECODE
#
ifneq ($(BDSP_LPCMDVD_SUPPORT),n)
BDSP_LPCMDVD_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_lpcm.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_lpcm.c


ifneq ($(wildcard $(BDSP_LPCMDVD_SOURCES)),)
BDSP_LPCMDVD_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_LPCMDVD_SUPPORT),y)
BDSP_DEFINES += BDSP_LPCMDVD_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_LPCMDVD_SOURCES)
endif


#
#DTSHD Passthrough
#
ifneq ($(BDSP_DTSHD_PASSTHRU_SUPPORT),n)
BDSP_DTSHD_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_dtshd.c

ifneq ($(wildcard $(BDSP_DTSHD_PASSTHRU_SOURCES)),)
BDSP_DTSHD_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DTSHD_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSHD_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSHD_PASSTHRU_SOURCES)
endif

#
#DTSHD DECODE
#
ifneq ($(BDSP_DTSHD_SUPPORT),n)
BDSP_DTSHD_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dtshd.c

ifneq ($(wildcard $(BDSP_DTSHD_SOURCES)),)
ifneq ($(wildcard $(BDSP_DTSHD_PASSTHRU_SOURCES)),)
BDSP_DTSHD_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DTSHD_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSHD_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSHD_SOURCES)
endif


#
#DTSLBR Passthrough
#
ifneq ($(BDSP_DTSLBR_PASSTHRU_SUPPORT),n)
BDSP_DTSLBR_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_dts_express.c

ifneq ($(wildcard $(BDSP_DTSLBR_PASSTHRU_SOURCES)),)
BDSP_DTSLBR_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DTSLBR_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSLBR_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSLBR_PASSTHRU_SOURCES)
endif



#
#DTSLBR DECODE  (Change it to DTS MX latter even macro to BDSP_DTSMX_SUPPORT )
#
ifneq ($(BDSP_DTSLBR_SUPPORT),n)
BDSP_DTSLBR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dts_express.c \

ifneq ($(wildcard $(BDSP_DTSLBR_SOURCES)),)
ifneq ($(wildcard $(BDSP_DTSLBR_PASSTHRU_SOURCES)),)
BDSP_DTSLBR_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DTSLBR_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSLBR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSLBR_SOURCES)
endif

#
#DTSBROADCAST Passthrough
#
ifneq ($(BDSP_DTSBROADCAST_PASSTHRU_SUPPORT),n)
BDSP_DTSBROADCAST_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_dts.c

ifneq ($(wildcard $(BDSP_DTSBROADCAST_PASSTHRU_SOURCES)),)
BDSP_DTSBROADCAST_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DTSBROADCAST_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSBROADCAST_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSBROADCAST_PASSTHRU_SOURCES)
endif

#
#DTSBROADCAST DECODE
#
ifneq ($(BDSP_DTSBROADCAST_SUPPORT),n)
BDSP_DTSBROADCAST_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dtshd.c

ifneq ($(wildcard $(BDSP_DTSBROADCAST_SOURCES)),)
ifneq ($(wildcard $(BDSP_DTSBROADCAST_PASSTHRU_SOURCES)),)
BDSP_DTSBROADCAST_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DTSBROADCAST_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSBROADCAST_SUPPORT
ifneq ($(BDSP_DTSHD_SUPPORT),y)
BDSP_IMG_SOURCES += $(BDSP_DTSBROADCAST_SOURCES)
endif
endif
#
#PCMWAV DECODE
#
ifneq ($(BDSP_PCMWAV_SUPPORT),n)
BDSP_PCMWAV_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_pcmwav.c

ifneq ($(wildcard $(BDSP_PCMWAV_SOURCES)),)
BDSP_PCMWAV_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_PCMWAV_SUPPORT),y)
BDSP_DEFINES += BDSP_PCMWAV_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_PCMWAV_SOURCES)
endif

#
#DRA Passthrough
#
ifneq ($(BDSP_DRA_PASSTHRU_SUPPORT),n)
BDSP_DRA_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_dra.c

ifneq ($(wildcard $(BDSP_DRA_PASSTHRU_SOURCES)),)
BDSP_DRA_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DRA_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_DRA_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DRA_PASSTHRU_SOURCES)
endif



#
#DRA DECODE
#
ifneq ($(BDSP_DRA_SUPPORT),n)
BDSP_DRA_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dra.c

ifneq ($(wildcard $(BDSP_DRA_SOURCES)),)
ifneq ($(wildcard $(BDSP_DRA_PASSTHRU_SOURCES)),)
BDSP_DRA_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DRA_SUPPORT),y)
BDSP_DEFINES += BDSP_DRA_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DRA_SOURCES)
endif

#
#PCM PASSTHRU DECODER
#
ifneq ($(BDSP_PCM_SUPPORT),n)
BDSP_PCM_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_pcm.c
ifneq ($(wildcard $(BDSP_PCM_SOURCES)),)
BDSP_PCM_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_PCM_SUPPORT),y)
BDSP_DEFINES += BDSP_PCM_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_PCM_SOURCES)
endif

#
#REALAUDIOLBR DECODE
#
ifneq ($(BDSP_REALAUDIOLBR_SUPPORT),n)
BDSP_REALAUDIOLBR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ralbr.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ralbr.c

ifneq ($(wildcard $(BDSP_REALAUDIOLBR_SOURCES)),)
BDSP_REALAUDIOLBR_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_REALAUDIOLBR_SUPPORT),y)
BDSP_DEFINES += BDSP_REALAUDIOLBR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_REALAUDIOLBR_SOURCES)
endif

#
#ADPCM DECODE
#
ifneq ($(BDSP_ADPCM_SUPPORT),n)
BDSP_ADPCM_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_adpcm.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_adpcm.c

ifneq ($(wildcard $(BDSP_ADPCM_SOURCES)),)
BDSP_ADPCM_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_ADPCM_SUPPORT),y)
BDSP_DEFINES += BDSP_ADPCM_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_ADPCM_SOURCES)
endif


#
#G711G726 DECODE
#
ifneq ($(BDSP_G711G726_SUPPORT),n)
BDSP_G711G726_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_g711_g726.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_g711_g726.c

ifneq ($(wildcard $(BDSP_G711G726_SOURCES)),)
BDSP_G711G726_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G711G726_SUPPORT),y)
BDSP_DEFINES += BDSP_G711G726_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G711G726_SOURCES)
endif


#
#G729 DECODE
#
ifneq ($(BDSP_G729_SUPPORT),n)
BDSP_G729_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_g729.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_g729.c

ifneq ($(wildcard $(BDSP_G729_SOURCES)),)
BDSP_G729_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G729_SUPPORT),y)
BDSP_DEFINES += BDSP_G729_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G729_SOURCES)
endif

#
#G723.1 DECODE
#
ifneq ($(BDSP_G723_1_SUPPORT),n)
BDSP_G723_1_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_g723_1.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_g723_1.c

ifneq ($(wildcard $(BDSP_G723_1_SOURCES)),)
BDSP_G723_1_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G723_1_SUPPORT),y)
BDSP_DEFINES += BDSP_G723_1_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G723_1_SOURCES)
endif

#
#SPEEX AEC
#
ifneq ($(BDSP_SPEEXAEC_SUPPORT),n)
BDSP_SPEEXAEC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_speexaec.c

ifneq ($(wildcard $(BDSP_SPEEXAEC_SOURCES)),)
BDSP_SPEEXAEC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SPEEXAEC_SUPPORT),y)
BDSP_DEFINES += BDSP_SPEEXAEC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SPEEXAEC_SOURCES)
endif

#
#KARAOKE PP
#
ifneq ($(BDSP_KARAOKE_SUPPORT),n)
BDSP_KARAOKE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_karaoke.c

ifneq ($(wildcard $(BDSP_KARAOKE_SOURCES)),)
BDSP_KARAOKE_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_KARAOKE_SUPPORT),y)
BDSP_DEFINES += BDSP_KARAOKE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_KARAOKE_SOURCES)
endif

#
#OUTPUTFORMATTER PP
#
ifneq ($(BDSP_OUTPUTFORMATTER_SUPPORT),n)
BDSP_OUTPUTFORMATTER_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_outputformatter.c

ifneq ($(wildcard $(BDSP_OUTPUTFORMATTER_SOURCES)),)
BDSP_OUTPUTFORMATTER_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_OUTPUTFORMATTER_SUPPORT),y)
BDSP_DEFINES += BDSP_OUTPUTFORMATTER_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_OUTPUTFORMATTER_SOURCES)
endif

#
#VOCALS PP
#

ifneq ($(BDSP_VOCALPP_SUPPORT),n)
BDSP_VOCALPP_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_vocals.c

ifneq ($(wildcard $(BDSP_VOCALPP_SOURCES)),)
BDSP_VOCALPP_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_VOCALPP_SUPPORT),y)
BDSP_DEFINES += BDSP_VOCALPP_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_VOCALPP_SOURCES)
endif

#
#VORBIS DECODE
#
ifneq ($(BDSP_VORBIS_SUPPORT),n)
BDSP_VORBIS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_vorbis.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_vorbis.c

ifneq ($(wildcard $(BDSP_VORBIS_SOURCES)),)
BDSP_VORBIS_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_VORBIS_SUPPORT),y)
BDSP_DEFINES += BDSP_VORBIS_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_VORBIS_SOURCES)
endif

#
#OPUS DECODE
#
ifneq ($(BDSP_OPUSDEC_SUPPORT),n)
BDSP_OPUSDEC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_opus.c

ifneq ($(wildcard $(BDSP_OPUSDEC_SOURCES)),)
BDSP_OPUSDEC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_OPUSDEC_SUPPORT),y)
BDSP_DEFINES += BDSP_OPUSDEC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_OPUSDEC_SOURCES)
endif
#
#ALS DECODE
#
ifneq ($(BDSP_ALS_SUPPORT),n)
BDSP_ALS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_als.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_als.c

ifneq ($(wildcard $(BDSP_ALS_SOURCES)),)
BDSP_ALS_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_ALS_SUPPORT),y)
BDSP_DEFINES += BDSP_ALS_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_ALS_SOURCES)
endif

#
#MLP DECODE AND PASSTHROUGH
#
ifneq ($(BDSP_MLP_SUPPORT),n)
BDSP_MLP_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_truehd.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_apt_mlp.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_truehd.c

ifneq ($(wildcard $(BDSP_MLP_SOURCES)),)
BDSP_MLP_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_MLP_SUPPORT),y)
BDSP_DEFINES += BDSP_MLP_SUPPORT
BDSP_DEFINES += BDSP_MLP_PASSTHROUGH_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MLP_SOURCES)
endif

#
#FLAC DECODE
#
ifneq ($(BDSP_FLAC_SUPPORT),n)
BDSP_FLAC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_flac.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_flac.c

ifneq ($(wildcard $(BDSP_FLAC_SOURCES)),)
BDSP_FLAC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_FLAC_SUPPORT),y)
BDSP_DEFINES += BDSP_FLAC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_FLAC_SOURCES)
endif

#
#MAC DECODE
#
ifneq ($(BDSP_MAC_SUPPORT),n)
BDSP_MAC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_mac.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_mac.c

ifneq ($(wildcard $(BDSP_MAC_SOURCES)),)
BDSP_MAC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_MAC_SUPPORT),y)
BDSP_DEFINES += BDSP_MAC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MAC_SOURCES)
endif


#
#AMR DECODE / Passthrough
#
ifneq ($(BDSP_AMR_SUPPORT),n)
BDSP_AMR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_amr.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_amr.c

ifneq ($(wildcard $(BDSP_AMR_SOURCES)),)
BDSP_AMR_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AMR_SUPPORT),y)
BDSP_DEFINES += BDSP_AMR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AMR_SOURCES)
endif

#
#AMRWB DECODE / Passthrough
#
ifneq ($(BDSP_AMRWB_SUPPORT),n)
BDSP_AMRWB_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_amrwb.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_amrwb.c

ifneq ($(wildcard $(BDSP_AMRWB_SOURCES)),)
BDSP_AMRWB_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AMRWB_SUPPORT),y)
BDSP_DEFINES += BDSP_AMRWB_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AMRWB_SOURCES)
endif

#
#ILBC DECODE / Passthrough
#
ifneq ($(BDSP_ILBC_SUPPORT),n)
BDSP_ILBC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ilbc.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ilbc.c

ifneq ($(wildcard $(BDSP_ILBC_SOURCES)),)
BDSP_ILBC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_ILBC_SUPPORT),y)
BDSP_DEFINES += BDSP_ILBC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_ILBC_SOURCES)
endif

#
#ISAC DECODE
#
ifneq ($(BDSP_ISAC_SUPPORT),n)
BDSP_ISAC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_isac.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_isac.c

ifneq ($(wildcard $(BDSP_ISAC_SOURCES)),)
BDSP_ISAC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_ISAC_SUPPORT),y)
BDSP_DEFINES += BDSP_ISAC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_ISAC_SOURCES)
endif


#
#UDC Passthrough
#
ifneq ($(BDSP_UDC_PASSTHRU_SUPPORT),n)
BDSP_UDC_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_udc.c

ifneq ($(wildcard $(BDSP_UDC_PASSTHRU_SOURCES)),)
BDSP_UDC_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_UDC_PASSTHRU_SUPPORT),y)
BDSP_DEFINES += BDSP_UDC_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_UDC_PASSTHRU_SOURCES)
endif

#
#UDC DECODE
#
ifneq ($(BDSP_UDC_SUPPORT),n)
BDSP_UDC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_udc.c
endif
ifeq ($(BDSP_UDC_SUPPORT),y)
BDSP_DEFINES += BDSP_UDC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_UDC_SOURCES)
endif

#
#DOLBY AACHE DECODE
#
ifneq ($(BDSP_DOLBY_AACHE_SUPPORT),n)
BDSP_DOLBY_AACHE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_dolby_aache.c \

endif
ifeq ($(BDSP_DOLBY_AACHE_SUPPORT),y)
BDSP_DEFINES += BDSP_DOLBY_AACHE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DOLBY_AACHE_SOURCES)
endif

#
#AC4 DECODE
#
ifneq ($(BDSP_AC4_SUPPORT),n)
BDSP_AC4_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_adec_ac4.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_ac4.c
endif
ifeq ($(BDSP_AC4_SUPPORT),y)
BDSP_DEFINES += BDSP_AC4_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AC4_SOURCES)
endif
#################################################################
#		ADD VIDEO ALGORITHM SUPPORT			#
#################################################################

#
#VP6 DECODE
#
ifneq ($(BDSP_VP6_SUPPORT),n)
BDSP_VP6_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_vdec_vp6.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_vids_vp6.c

ifneq ($(wildcard $(BDSP_VP6_SOURCES)),)
BDSP_VP6_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_VP6_SUPPORT),y)
BDSP_DEFINES += BDSP_VP6_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_VP6_SOURCES)
endif

#################################################################
#		ADD VIDEO ENCODE ALGORITHM SUPPORT			#
#################################################################

#
#H264 ENCODE
#
ifneq ($(BDSP_H264_ENCODE_SUPPORT),n)
BDSP_H264_ENCODE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_venc_h264.c

ifneq ($(wildcard $(BDSP_H264_ENCODE_SOURCES)),)
BDSP_H264_ENCODE_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_H264_ENCODE_SUPPORT),y)
BDSP_DEFINES += BDSP_H264_ENCODE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_H264_ENCODE_SOURCES)
endif
#
#X264 ENCODE
#
ifneq ($(BDSP_X264_ENCODE_SUPPORT),n)
BDSP_X264_ENCODE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_venc_x264.c
ifneq ($(wildcard $(BDSP_X264_ENCODE_SOURCES)),)
BDSP_X264_ENCODE_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
ifeq ($(BDSP_X264_ENCODE_SUPPORT),y)
BDSP_DEFINES += BDSP_X264_ENCODE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_X264_ENCODE_SOURCES)
endif
#XVP8 ENCODE
#
ifneq ($(BDSP_XVP8_ENCODE_SUPPORT),n)
BDSP_XVP8_ENCODE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_venc_xvp8.c
ifneq ($(wildcard $(BDSP_XVP8_ENCODE_SOURCES)),)
BDSP_XVP8_ENCODE_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
ifeq ($(BDSP_XVP8_ENCODE_SUPPORT),y)
BDSP_DEFINES += BDSP_XVP8_ENCODE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_XVP8_ENCODE_SOURCES)
endif

#
#################################################################
#		ADD POST PROCESSING SUPPORT			#
#################################################################

#
#SRC Post Processing
#
ifneq ($(BDSP_SRC_SUPPORT),n)
BDSP_SRC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_src.c

ifneq ($(wildcard $(BDSP_SRC_SOURCES)),)
BDSP_SRC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SRC_SUPPORT),y)
BDSP_DEFINES += BDSP_SRC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SRC_SOURCES)
endif

#
#FWMIXER Post Processing
#
ifneq ($(BDSP_FWMIXER_SUPPORT),n)
BDSP_FWMIXER_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_fw_mixer.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_mixer.c

ifneq ($(wildcard $(BDSP_FWMIXER_SOURCES)),)
BDSP_FWMIXER_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_FWMIXER_SUPPORT),y)
BDSP_DEFINES += BDSP_FWMIXER_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_FWMIXER_SOURCES)
endif

#
#MIXER DAPV2 Post Processing
#
ifneq ($(BDSP_MIXERDAPV2_SUPPORT),n)
BDSP_MIXERDAPV2_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_mixer_dapv2.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aids_mixer_dapv2.c

endif

ifeq ($(BDSP_MIXERDAPV2_SUPPORT),y)
BDSP_DEFINES += BDSP_MIXERDAPV2_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MIXERDAPV2_SOURCES)
endif


#
#DSOLA Post Processing
#
ifneq ($(BDSP_DSOLA_SUPPORT),n)
BDSP_DSOLA_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_dsola.c

ifneq ($(wildcard $(BDSP_DSOLA_SOURCES)),)
BDSP_DSOLA_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DSOLA_SUPPORT),y)
BDSP_DEFINES += BDSP_DSOLA_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DSOLA_SOURCES)
endif

#
#FADECTRL Post Processing
#
ifneq ($(BDSP_FADECTRL_SUPPORT),n)
BDSP_FADECTRL_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_fadectrl.c

ifneq ($(wildcard $(BDSP_FADECTRL_SOURCES)),)
BDSP_FADECTRL_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_FADECTRL_SUPPORT),y)
BDSP_DEFINES += BDSP_FADECTRL_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_FADECTRL_SOURCES)
endif

#
#CUSTOMVOICE Post processing
#
ifneq ($(BDSP_CUSTOMVOICE_SUPPORT),n)
BDSP_CUSTOMVOICE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_customvoice.c

ifneq ($(wildcard $(BDSP_CUSTOMVOICE_SOURCES)),)
BDSP_CUSTOMVOICE_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_CUSTOMVOICE_SUPPORT),y)
BDSP_DEFINES += BDSP_CUSTOMVOICE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_CUSTOMVOICE_SOURCES)
endif


#
#SRSHD - SRS HD
#
ifneq ($(BDSP_SRSHD_SUPPORT),n)
BDSP_SRSHD_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_srs_trusurroundhd.c

ifneq ($(wildcard $(BDSP_SRSHD_SOURCES)),)
BDSP_SRSHD_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SRSHD_SUPPORT),y)
BDSP_DEFINES += BDSP_SRSHD_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SRSHD_SOURCES)
endif

#
#SRSTRUVOL - SRS Tru Volume
#
ifneq ($(BDSP_SRSTRUVOL_SUPPORT),n)
BDSP_SRSTRUVOL_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_srs_tvol.c

ifneq ($(wildcard $(BDSP_SRSTRUVOL_SOURCES)),)
BDSP_SRSTRUVOL_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SRSTRUVOL_SUPPORT),y)
BDSP_DEFINES += BDSP_SRSTRUVOL_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SRSTRUVOL_SOURCES)
endif


#
#AVL - Broadcom Auto Volume Leveller
#
ifneq ($(BDSP_AVL_SUPPORT),n)
BDSP_AVL_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_brcm_avl.c

ifneq ($(wildcard $(BDSP_AVL_SOURCES)),)
BDSP_AVL_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AVL_SUPPORT),y)
BDSP_DEFINES += BDSP_AVL_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AVL_SOURCES)
endif

#
#BRCM3DSURROUND - Broadcom 3D SURROUND
#
ifneq ($(BDSP_BRCM3DSURROUND_SUPPORT),n)
BDSP_BRCM3DSURROUND_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_brcm_3dsurround.c

ifneq ($(wildcard $(BDSP_BRCM3DSURROUND_SOURCES)),)
BDSP_BRCM3DSURROUND_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_BRCM3DSURROUND_SUPPORT),y)
BDSP_DEFINES += BDSP_BRCM3DSURROUND_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_BRCM3DSURROUND_SOURCES)
endif

#
#GENCDBITB - Generate CDB ITB
#
ifneq ($(BDSP_GENCDBITB_SUPPORT),n)
BDSP_GENCDBITB_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_gen_cdbitb.c

ifneq ($(wildcard $(BDSP_GENCDBITB_SOURCES)),)
BDSP_GENCDBITB_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_GENCDBITB_SUPPORT),y)
BDSP_DEFINES += BDSP_GENCDBITB_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_GENCDBITB_SOURCES)
endif

#
#BTSC ENCODER - BTSC Encoder
#
ifneq ($(BDSP_BTSCENC_SUPPORT),n)
BDSP_BTSCENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_btscenc.c

ifneq ($(wildcard $(BDSP_BTSCENC_SOURCES)),)
BDSP_BTSCENC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_BTSCENC_SUPPORT),y)
BDSP_DEFINES += BDSP_BTSCENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_BTSCENC_SOURCES)
endif


#################################################################
#		ADD ENCODE SUPPORT				#
#################################################################


#
#DTS Encoder
#
ifneq ($(BDSP_DTSENC_SUPPORT),n)
BDSP_DTSENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_dts.c

ifneq ($(wildcard $(BDSP_DTSENC_SOURCES)),)
BDSP_DTSENC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DTSENC_SUPPORT),y)
BDSP_DEFINES += BDSP_DTSENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSENC_SOURCES)
endif

#
#AC3 Encoder
#
ifeq ($(BDSP_MS10_SUPPORT),y)
BDSP_AC3ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_dd.c
endif

ifneq ($(wildcard $(BDSP_AC3ENC_SOURCES)),)
BDSP_AC3ENC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif

ifeq ($(BDSP_MS10_SUPPORT),y)
BDSP_DEFINES += BDSP_AC3ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AC3ENC_SOURCES)
endif

#
#AACHE Encoder
#
ifneq ($(BDSP_AACHEENC_SUPPORT),n)
BDSP_AACHEENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_aache.c

ifneq ($(wildcard $(BDSP_AACHEENC_SOURCES)),)
BDSP_AACHEENC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AACHEENC_SUPPORT),y)
BDSP_DEFINES += BDSP_AACHEENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AACHEENC_SOURCES)
endif

#
#MP3 Encoder
#
ifneq ($(BDSP_MP3ENC_SUPPORT),n)
BDSP_MP3ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_mp3.c

ifneq ($(wildcard $(BDSP_MP3ENC_SOURCES)),)
BDSP_MP3ENC_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_MP3ENC_SUPPORT),y)
BDSP_DEFINES += BDSP_MP3ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MP3ENC_SOURCES)
endif



#
#G711G726 Encoder
#
ifneq ($(BDSP_G711G726ENC_SUPPORT),n)
BDSP_G711G726ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_g711_g726.c

ifneq ($(wildcard $(BDSP_G711G726ENC_SOURCES)),)
BDSP_G711G726ENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G711G726ENC_SUPPORT),y)
BDSP_DEFINES += BDSP_G711G726ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G711G726ENC_SOURCES)
endif


#
#G729 Encoder
#
ifneq ($(BDSP_G729ENC_SUPPORT),n)
BDSP_G729ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_g729.c

ifneq ($(wildcard $(BDSP_G729ENC_SOURCES)),)
BDSP_G729ENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G729ENC_SUPPORT),y)
BDSP_DEFINES += BDSP_G729ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G729ENC_SOURCES)
endif

#
#G723.1 Encoder
#
ifneq ($(BDSP_G723_1ENC_SUPPORT),n)
BDSP_G723_1ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_g723_1.c

ifneq ($(wildcard $(BDSP_G723_1ENC_SOURCES)),)
BDSP_G723_1ENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G723_1ENC_SUPPORT),y)
BDSP_DEFINES += BDSP_G723_1ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G723_1ENC_SOURCES)
endif


#
#G722 Encoder
#
ifneq ($(BDSP_G722ENC_SUPPORT),n)
BDSP_G722ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_g722.c

ifneq ($(wildcard $(BDSP_G722ENC_SOURCES)),)
BDSP_G722ENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_G722ENC_SUPPORT),y)
BDSP_DEFINES += BDSP_G722ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_G722ENC_SOURCES)
endif

#
#AMR Encoder
#
ifneq ($(BDSP_AMRENC_SUPPORT),n)
BDSP_AMRENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_amr.c

ifneq ($(wildcard $(BDSP_AMRENC_SOURCES)),)
BDSP_AMRENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AMRENC_SUPPORT),y)
BDSP_DEFINES += BDSP_AMRENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AMRENC_SOURCES)
endif

#
#AMRWB Encoder
#
ifneq ($(BDSP_AMRWBENC_SUPPORT),n)
BDSP_AMRWBENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_amrwb.c

ifneq ($(wildcard $(BDSP_AMRWBENC_SOURCES)),)
BDSP_AMRWBENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_AMRWBENC_SUPPORT),y)
BDSP_DEFINES += BDSP_AMRWBENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_AMRWBENC_SOURCES)
endif

#
#ILBC Encoder
#
ifneq ($(BDSP_ILBCENC_SUPPORT),n)
BDSP_ILBCENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_ilbc.c

ifneq ($(wildcard $(BDSP_ILBCENC_SOURCES)),)
BDSP_ILBCENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_ILBCENC_SUPPORT),y)
BDSP_DEFINES += BDSP_ILBCENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_ILBCENC_SOURCES)
endif


#
#ISAC Encoder
#
ifneq ($(BDSP_ISACENC_SUPPORT),n)
BDSP_ISACENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_isac.c

ifneq ($(wildcard $(BDSP_ISACENC_SOURCES)),)
BDSP_ISACENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_ISACENC_SUPPORT),y)
BDSP_DEFINES += BDSP_ISACENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_ISACENC_SOURCES)
endif

#
#LPCM Encoder
#
ifneq ($(BDSP_LPCMENC_SUPPORT),n)
BDSP_LPCMENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_lpcm.c

ifneq ($(wildcard $(BDSP_LPCMENC_SOURCES)),)
BDSP_LPCMENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_LPCMENC_SUPPORT),y)
BDSP_DEFINES += BDSP_LPCMENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_LPCMENC_SOURCES)
endif

#
#OPUS Encoder
#
ifneq ($(BDSP_OPUS_ENC_SUPPORT),n)
BDSP_OPUS_ENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_opus.c
ifneq ($(wildcard $(BDSP_OPUS_ENC_SOURCES)),)
BDSP_OPUS_ENC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
ifeq ($(BDSP_OPUS_ENC_SUPPORT),y)
BDSP_DEFINES += BDSP_OPUS_ENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_OPUS_ENC_SOURCES)
endif
#DDP Encoder
#
ifneq ($(BDSP_DDPENC_SUPPORT),n)
BDSP_DDPENC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_aenc_ddp.c
endif
ifeq ($(BDSP_DDPENC_SUPPORT),y)
BDSP_DEFINES += BDSP_DDPENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDPENC_SOURCES)
endif
#
#DV258 - DOLBY VOLUME
#
ifneq ($(BDSP_DV258_SUPPORT),n)
BDSP_DV258_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_dv258.c

ifneq ($(wildcard $(BDSP_DV258_SOURCES)),)
BDSP_DV258_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DV258_SUPPORT),y)
BDSP_DEFINES += BDSP_DV258_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DV258_SOURCES)
endif
#
#Dpcmr - DOLBY PCM RENDERER
#
ifneq ($(BDSP_DPCMR_SUPPORT),n)
BDSP_DPCMR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_dpcmr.c
endif
ifeq ($(BDSP_DPCMR_SUPPORT),y)
BDSP_DEFINES += BDSP_DPCMR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DPCMR_SOURCES)
endif

#
#DDRE - DOLBY DIGITAL RE-ENCODE
#
ifneq ($(BDSP_DDRE_SUPPORT),n)
BDSP_DDRE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_app_ddre.c

ifneq ($(wildcard $(BDSP_DDRE_SOURCES)),)
BDSP_DDRE_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DDRE_SUPPORT),y)
BDSP_DEFINES += BDSP_DDRE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDRE_SOURCES)
endif

ifeq ($(RAP_RBUF_CAP_SUPPORT),y)
    BDSP_DEFINES += BDSP_FW_RBUF_CAPTURE
endif

#
# SCM 1 ALGORITHM
#
ifneq ($(BDSP_SCM1_SUPPORT),n)
BDSP_SCM1_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_scm1_digest.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_sec_scm1.c

ifneq ($(wildcard $(BDSP_SCM1_SOURCES)),)
BDSP_SCM1_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SCM1_SUPPORT),y)
BDSP_DEFINES += BDSP_SCM1_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SCM1_SOURCES)
endif

ifeq ($(RAP_RBUF_CAP_SUPPORT),y)
    BDSP_DEFINES += BDSP_FW_RBUF_CAPTURE
endif


#
# SCM 2 ALGORITHM
#
ifneq ($(BDSP_SCM2_SUPPORT),n)
BDSP_SCM2_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_scm2_digest.c \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_sec_scm2.c

ifneq ($(wildcard $(BDSP_SCM2_SOURCES)),)
BDSP_SCM2_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SCM2_SUPPORT),y)
BDSP_DEFINES += BDSP_SCM2_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SCM2_SOURCES)
endif

#
# SCM 3 ALGORITHM
#
ifneq ($(BDSP_SCM3_SUPPORT),n)
BDSP_SCM3_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/raaga/firmware/$(BDSP_RAAGA_FP_VER)/braaga_sec_scm3.c

ifneq ($(wildcard $(BDSP_SCM3_SOURCES)),)
BDSP_SCM3_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SCM3_SUPPORT),y)
BDSP_DEFINES += BDSP_SCM3_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SCM3_SOURCES)
endif

ifeq ($(HEAP_DBG),y)
    BDSP_DEFINES += HEAP_DBG
endif

ifeq ($(HEAP_DBG_NO_ASSERT),y)
    BDSP_DEFINES += HEAP_DBG_NO_ASSERT
endif

ifeq ($(RAP_RBUF_CAP_SUPPORT),y)
    BDSP_DEFINES += BDSP_FW_RBUF_CAPTURE
endif
# This must be last
#
ifneq ($(BDSP_IMG_EXTERNAL),y)
BDSP_DEFINES += BDSP_IMG_SUPPORT=1
BDSP_SOURCES += $(BDSP_IMG_SOURCES)
endif

ifeq ($(BDSP_DOLBY_DCV_SUPPORT),y)
	BDSP_DEFINES += BDSP_DOLBY_DCV_SUPPORT
endif

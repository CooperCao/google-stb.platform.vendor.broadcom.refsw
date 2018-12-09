 ###############################################################################
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
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)
BDSP_INCLUDES += $(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)
#
# All Firmware sources must be in BDSP_IMG_SOURCES, not BDSP_SOURCES.  Based on BDSP_IMG_EXTERNAL, choose to include the firmware files or not at the end.
#

BDSP_IMG_SOURCES += \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/bdsp_arm_img.c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/barm_rdbvars_$(BCHP_CHIP).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_init_process_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_idle_process_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_algo_process_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_messaging_process_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_routing_process_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_scheduling_process_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_common_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_common_ids_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_kernel_ops_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_mm_client_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_stage_io_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_debug_control_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_mm_server_code$(BDSP_AONA_FILE_TYPE).c

#$(warning "From BDSP "$(BDSP_IMG_SOURCES))

#################################################################
#		Enable Modules For MS12/MS11PLUS Support        #
#################################################################
BDSP_MS12_SUPPORT ?= n

ifeq ($(BDSP_MS12_SUPPORT),n)
BDSP_MS11PLUS_SUPPORT ?= y
else
BDSP_MS11PLUS_SUPPORT = n
endif

ifeq ($(BDSP_MS11PLUS_SUPPORT),y)
BDSP_AC4_SUPPORT = n
BDSP_DEFINES += BDSP_MS11PLUS_SUPPORT
endif

ifeq ($(BDSP_MS12_SUPPORT),y)
BDSP_MS12_SUPPORT = B
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
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_mpeg1_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_mpeg1_data.c
BDSP_MPEG_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_mpeg1_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_MPEG_SOURCES)),)
ifneq ($(wildcard $(BDSP_MPEG_IDS_SOURCES)),)
BDSP_MPEG_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_MPEG_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user
BDSP_MPEG_PASSTHRU_SUPPORT ?= y
BDSP_DEFINES += BDSP_MPEG_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MPEG_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_MPEG_IDS_SOURCES)
endif


#
#MPEG Passthrough
#
ifneq ($(BDSP_MPEG_PASSTHRU_SUPPORT),n)
BDSP_MPEG_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_MPEG_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_mpeg1_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_MPEG_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_MPEG_PASSTHRU_IDS_SOURCES)),)
BDSP_MPEG_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_MPEG_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_MPEG_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MPEG_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_MPEG_PASSTHRU_IDS_SOURCES)
endif


#
#DTSHD Passthrough
#
ifneq ($(BDSP_DTSHD_PASSTHRU_SUPPORT),n)
BDSP_DTSHD_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_DTSHD_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_dtshd_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_DTSHD_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_DTSHD_PASSTHRU_IDS_SOURCES)),)
BDSP_DTSHD_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DTSHD_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_DTSHD_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTSHD_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_DTSHD_PASSTHRU_IDS_SOURCES)
endif

#
#DTS Passthrough
#
ifneq ($(BDSP_DTS_PASSTHRU_SUPPORT),n)
BDSP_DTS_PASSTHRU_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_DTS_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_dts_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_DTS_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_DTS_PASSTHRU_IDS_SOURCES)),)
BDSP_DTS_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DTS_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_DTS_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DTS_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_DTS_PASSTHRU_IDS_SOURCES)
endif


#
#PCMWAV DECODE
#
ifneq ($(BDSP_PCMWAV_SUPPORT),n)
BDSP_PCMWAV_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_pcmwav_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_pcmwav_data.c
BDSP_PCMWAV_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_wavformatex_code$(BDSP_AONA_FILE_TYPE).c

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
ifneq ($(BDSP_MS12_SUPPORT),n)
BDSP_UDC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_udc_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_udc_data.c
BDSP_UDC_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_UDC_SOURCES)),)
ifneq ($(wildcard $(BDSP_UDC_IDS_SOURCES)),)
BDSP_UDC_SUPPORT ?= y
endif
endif
endif

ifneq ($(BDSP_MS11PLUS_SUPPORT),n)
BDSP_UDC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_ms11plus_udc_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_ms11plus_udc_data.c
BDSP_UDC_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_UDC_SOURCES)),)
ifneq ($(wildcard $(BDSP_UDC_IDS_SOURCES)),)
BDSP_UDC_SUPPORT ?= y
endif
endif
endif

ifeq ($(BDSP_UDC_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user, disable legacy AC3/DDP
BDSP_UDC_PASSTHRU_SUPPORT ?= y
BDSP_DEFINES += BDSP_UDC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_UDC_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_UDC_IDS_SOURCES)
override BDSP_AC3_SUPPORT = n
override BDSP_AC3_PASSTHRU_SUPPORT = n
override BDSP_DDP_SUPPORT = n
override BDSP_DDP_PASSTHRU_SUPPORT = n
else
# Disable UDC passthru when UDC is not available, should use either AC3/DDP Passthru
override BDSP_UDC_PASSTHRU_SUPPORT = n
endif

#
#UDC Passthrough
#
ifneq ($(BDSP_UDC_PASSTHRU_SUPPORT),n)
BDSP_UDC_PASSTHRU_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_UDC_PASSTHRU_IDS_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_UDC_PASSTHRU_SOURCES)),)
ifneq ($(wildcard $(BDSP_UDC_PASSTHRU_IDS_SOURCES)),)
BDSP_UDC_PASSTHRU_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_UDC_PASSTHRU_SUPPORT),y)
# define internal to BDSP to enable passthru
BDSP_PASSTHRU_SUPPORT = y
BDSP_DEFINES += BDSP_UDC_PASSTHRU_SUPPORT
BDSP_DEFINES += BDSP_PASSTHRU_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_UDC_PASSTHRU_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_UDC_PASSTHRU_IDS_SOURCES)
endif

#
#DDP DECODE
#
ifneq ($(BDSP_DDP_SUPPORT),n)
BDSP_DDP_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_ddp_code$(BDSP_AONA_FILE_TYPE).c \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_ddp_data.c
BDSP_DDP_IDS_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_DDP_SOURCES)),)
ifneq ($(wildcard $(BDSP_DDP_IDS_SOURCES)),)
BDSP_DDP_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
endif
endif
endif

ifeq ($(BDSP_DDP_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user
BDSP_DDP_PASSTHRU_SUPPORT ?= y
# Disable AC3 support
override BDSP_AC3_SUPPORT = n
override BDSP_AC3_PASSTHRU_SUPPORT = n
BDSP_DEFINES += BDSP_DDP_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDP_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_DDP_IDS_SOURCES)
endif

#
#DDP Passthrough
#
ifneq ($(BDSP_DDP_PASSTHRU_SUPPORT),n)
BDSP_DDP_PASSTHRU_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_DDP_PASSTHRU_IDS_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

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
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_ac3_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_ac3_data.c
BDSP_AC3_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

ifneq ($(wildcard $(BDSP_AC3_SOURCES)),)
ifneq ($(wildcard $(BDSP_AC3_IDS_SOURCES)),)
BDSP_AC3_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
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
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_AC3_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_ddp_code$(BDSP_AONA_FILE_TYPE).c

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
#DOLBY AACHE DECODE
#
ifneq ($(BDSP_MS12_SUPPORT),n)
BDSP_DOLBY_AACHE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_dolby_aache_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_dolby_aache_data.c
BDSP_DOLBY_AACHE_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_adts_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_loas_code$(BDSP_AONA_FILE_TYPE).c \

ifneq ($(wildcard $(BDSP_DOLBY_AACHE_SOURCES)),)
ifneq ($(wildcard $(BDSP_DOLBY_AACHE_IDS_SOURCES)),)
BDSP_DOLBY_AACHE_SUPPORT ?= y
endif
endif
endif

ifneq ($(BDSP_MS11PLUS_SUPPORT),n)
BDSP_DOLBY_AACHE_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_ms11plus_dolby_aache_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_ms11plus_dolby_aache_data.c
BDSP_DOLBY_AACHE_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_adts_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_loas_code$(BDSP_AONA_FILE_TYPE).c \

ifneq ($(wildcard $(BDSP_DOLBY_AACHE_SOURCES)),)
ifneq ($(wildcard $(BDSP_DOLBY_AACHE_IDS_SOURCES)),)
BDSP_DOLBY_AACHE_SUPPORT ?= y
endif
endif
endif

ifeq ($(BDSP_DOLBY_AACHE_SUPPORT),y)
# Enable passthru also unless explicitly disabled by user
BDSP_AACSBR_PASSTHRU_SUPPORT ?= y
BDSP_DEFINES += BDSP_DOLBY_AACHE_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DOLBY_AACHE_SOURCES)
BDSP_IMG_SOURCES += $(BDSP_DOLBY_AACHE_IDS_SOURCES)
# Disable legacy AAC
override BDSP_AACSBR_SUPPORT = n
endif

#
#AAC DECODE
#
ifneq ($(BDSP_AACSBR_SUPPORT),n)
BDSP_AACSBR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_aache_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_aache_data.c
BDSP_AACSBR_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_adts_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_loas_code$(BDSP_AONA_FILE_TYPE).c \

ifneq ($(wildcard $(BDSP_AACSBR_SOURCES)),)
ifneq ($(wildcard $(BDSP_AACSBR_IDS_SOURCES)),)
BDSP_AACSBR_SUPPORT ?= $(BDSP_3RDPARTY_ALGO_SUPPORT)
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
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_passthru_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_passthru_data.c
BDSP_AACSBR_PASSTHRU_IDS_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_adts_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aids_loas_code$(BDSP_AONA_FILE_TYPE).c \

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

#
#OPUS DECODE
#
ifneq ($(BDSP_OPUSDEC_SUPPORT),n)
BDSP_OPUSDEC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_adec_opus_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_adec_opus_data.c

ifneq ($(wildcard $(BDSP_OPUSDEC_SOURCES)),)
BDSP_OPUSDEC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_OPUSDEC_SUPPORT),y)
BDSP_DEFINES += BDSP_OPUSDEC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_OPUSDEC_SOURCES)
endif

##########################################
#		POST PROCESS             #
##########################################

#
#SRC PP
#
ifneq ($(BDSP_SRC_SUPPORT),n)
BDSP_SRC_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_src_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_src_data.c

ifneq ($(wildcard $(BDSP_SRC_SOURCES)),)
BDSP_SRC_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_SRC_SUPPORT),y)
BDSP_DEFINES += BDSP_SRC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_SRC_SOURCES)
endif

#
#DPCMR PP
#
ifneq ($(BDSP_MS12_SUPPORT),n)
BDSP_DPCMR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_dpcmr_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_dpcmr_data.c

ifneq ($(wildcard $(BDSP_DPCMR_SOURCES)),)
BDSP_DPCMR_SUPPORT ?= y
endif
endif

ifneq ($(BDSP_MS11PLUS_SUPPORT),n)
BDSP_DPCMR_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_ms11plus_dpcmr_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_ms11plus_dpcmr_data.c

ifneq ($(wildcard $(BDSP_DPCMR_SOURCES)),)
BDSP_DPCMR_SUPPORT ?= y
endif
endif

ifeq ($(BDSP_DPCMR_SUPPORT),y)
BDSP_DEFINES += BDSP_DPCMR_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DPCMR_SOURCES)
endif

#
#DDPENC ENC
#
ifneq ($(BDSP_MS12_SUPPORT),n)
BDSP_DDPENC_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aenc_ddp_code$(BDSP_AONA_FILE_TYPE).c \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_aenc_ddp_data.c

ifneq ($(wildcard $(BDSP_DDPENC_SOURCES)),)
BDSP_DDPENC_SUPPORT ?= y
endif
endif

ifneq ($(BDSP_MS11PLUS_SUPPORT),n)
BDSP_DDPENC_SOURCES := \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_aenc_ms11plus_dd_code$(BDSP_AONA_FILE_TYPE).c \
		$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_aenc_ms11plus_dd_data.c

ifneq ($(wildcard $(BDSP_DDPENC_SOURCES)),)
BDSP_DDPENC_SUPPORT ?= y
endif
endif

ifeq ($(BDSP_DDPENC_SUPPORT),y)
BDSP_DEFINES += BDSP_DDPENC_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DDPENC_SOURCES)
endif

#
#MIXER_DAPV2 PP
#
ifneq ($(BDSP_MS12_SUPPORT),n)
BDSP_MIXER_DAPV2_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_mixer_dapv2_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_mixer_dapv2_data.c

ifneq ($(wildcard $(BDSP_MIXER_DAPV2_SOURCES)),)
BDSP_MIXERDAPV2_SUPPORT ?= y
endif
endif

ifneq ($(BDSP_MS11PLUS_SUPPORT),n)
BDSP_MIXER_DAPV2_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_ms11plus_mixer_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_ms11plus_mixer_data.c

ifneq ($(wildcard $(BDSP_MIXER_DAPV2_SOURCES)),)
BDSP_MIXERDAPV2_SUPPORT ?= y
endif
endif

ifeq ($(BDSP_MIXERDAPV2_SUPPORT),y)
BDSP_DEFINES += BDSP_MIXERDAPV2_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_MIXER_DAPV2_SOURCES)
endif

#
#FWMIXER PP
#
ifneq ($(BDSP_FWMIXER_SUPPORT),n)
BDSP_FWMIXER_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_fw_mixer_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_fw_mixer_data.c

ifneq ($(wildcard $(BDSP_FWMIXER_SOURCES)),)
BDSP_FWMIXER_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_FWMIXER_SUPPORT),y)
BDSP_DEFINES += BDSP_FWMIXER_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_FWMIXER_SOURCES)
endif

#
#DSOLA PP
#
ifneq ($(BDSP_DSOLA_SUPPORT),n)
BDSP_DSOLA_SOURCES := \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/$(BDSP_AONA_FOLDER_TYPE)/barm_app_dsola_code$(BDSP_AONA_FILE_TYPE).c \
	$(MAGNUM)/basemodules/dsp/src/arm_v2/firmware/$(AONA_VER)/data/barm_app_dsola_data.c

ifneq ($(wildcard $(BDSP_DSOLA_SOURCES)),)
BDSP_DSOLA_SUPPORT ?= $(BDSP_DEFAULT_ALGO_SUPPORT)
endif
endif

ifeq ($(BDSP_DSOLA_SUPPORT),y)
BDSP_DEFINES += BDSP_DSOLA_SUPPORT
BDSP_IMG_SOURCES += $(BDSP_DSOLA_SOURCES)
endif

#############################################################################
#Control to Disable MS12, if none of the MS12 required modules are available
#############################################################################
ifneq ($(BDSP_UDC_SUPPORT),y)
ifneq ($(BDSP_DOLBY_AACHE_SUPPORT),y)
ifneq ($(BDSP_MIXERDAPV2_SUPPORT),y)
ifneq ($(BDSP_DDPENC_SUPPORT),y)
ifneq ($(BDSP_DPCMR_SUPPORT),y)
ifneq ($(BDSP_AC4_SUPPORT),y)
override BDSP_MS12_SUPPORT=n
override BDSP_MS11PLUS_SUPPORT=n
endif
endif
endif
endif
endif
endif

ifneq ($(filter $(BDSP_MS12_SUPPORT),A B C D Y a b c d y),)
ifeq ($(BDSP_MS12_SUPPORT),A)
       BDSP_DEFINES += BDSP_MS12_SUPPORT=65
else ifeq ($(BDSP_MS12_SUPPORT),a)
       BDSP_DEFINES += BDSP_MS12_SUPPORT=65
else ifeq ($(BDSP_MS12_SUPPORT),B)
       BDSP_DEFINES += BDSP_MS12_SUPPORT=66
else ifeq ($(BDSP_MS12_SUPPORT),b)
       BDSP_DEFINES += BDSP_MS12_SUPPORT=66
else ifeq ($(BDSP_MS12_SUPPORT),Y)
       BDSP_DEFINES += BDSP_MS12_SUPPORT=66
else ifeq ($(BDSP_MS12_SUPPORT),y)
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

#This should be at the last
ifneq ($(BDSP_IMG_EXTERNAL),y)
BDSP_DEFINES += BDSP_IMG_SUPPORT=1
BDSP_SOURCES += $(BDSP_IMG_SOURCES)
endif

BDSP_SOURCES := $(shell echo $(BDSP_SOURCES) | tr ' ' '\n' | awk '!a[$$0]++')

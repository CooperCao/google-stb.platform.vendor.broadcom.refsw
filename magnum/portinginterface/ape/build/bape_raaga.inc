#############################################################################
# Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

# Define this module
MAGNUM_MODULES += BAPE

# Module Includes
BAPE_INCLUDES += $(MAGNUM)/portinginterface/ape/include \
                 $(MAGNUM)/portinginterface/ape/include/raaga \
                 $(MAGNUM)/portinginterface/ape/src/raaga

# Module Defines
BAPE_DEFINES += BAPE_RAAGA=1

# Determine if DSP support should be enabled or not
ifeq ($(BCHP_CHIP),7543)
BAPE_DSP_SUPPORT ?= n
else
ifndef BAPE_DSP_SUPPORT
BAPE_DSP_SUPPORT ?= y
endif
endif

ifndef BAPE_AIO_SUPPORT
BAPE_AIO_SUPPORT ?= y
endif

# Module Sources
ifneq ($(BAPE_IMG_ONLY),y)
BAPE_SOURCES := \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_3d_surround.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_arc.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_auto_volume_level.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_bf_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_buffer.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_crc.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_custom_processing.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dac.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_debug.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder_ancillary_data.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder_settings_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder_status_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dolby_digital_reencode.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dolby_volume.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dp_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dsp_utils_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dummy_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_echo_canceller.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_encoder.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_eq_coeff_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_equalizer.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_fmm_isr.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_fmt_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_fs_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_i2s_input.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_i2s_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_i2s_multi_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_input_capture.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_input_port_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_iop_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mai_input.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mai_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_bypass.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_dsp.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_input_capture.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_standard.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_ms_fw.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mux_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_nco.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_output_capture.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_path_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_playback.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_pll.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_processor.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_reg_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_resources.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_rf_encoder.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_rfmod.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_spdif_input.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_spdif_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_src_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_tru_volume.c

ifneq ($(wildcard ${BCHP_RDB_PATH}/bchp_ott_xpt_cdb_itb.h),)
BAPE_SOURCES += $(MAGNUM)/portinginterface/ape/src/raaga/bape_dma_output_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_buffer_filter_priv.c
endif

endif

ifneq ($(BAPE_DSP_SUPPORT),n)
BAPE_DEFINES += BAPE_DSP_SUPPORT=1
endif

ifneq ($(BAPE_AIO_SUPPORT),n)
BAPE_DEFINES += BAPE_AIO_SUPPORT=1
endif

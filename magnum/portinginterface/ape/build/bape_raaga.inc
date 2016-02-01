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
# Created: 08/27/2009 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

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
BAPE_DSP_SUPPORT ?= y
endif

# Module Sources
ifneq ($(BAPE_IMG_ONLY),y)
BAPE_FMM_SOURCES := \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_arc.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_bf_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_buffer.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_crc.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dac.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_debug.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dp_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dummy_output.c \
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
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_standard.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_ms_fw.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_nco.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_output_capture.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_path_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_playback.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_pll.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_reg_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_resources.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_rfmod.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_spdif_input.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_spdif_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_src_priv.c \

BAPE_DSP_SOURCES := \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_3d_surround.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_auto_volume_level.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_custom_processing.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder_ancillary_data.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder_settings_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_decoder_status_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dolby_digital_reencode.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dolby_volume.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_dsp_utils_priv.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_echo_canceller.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_processor.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_encoder.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_dsp.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mixer_input_capture.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_mux_output.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_rf_encoder.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_tru_surround.c \
                $(MAGNUM)/portinginterface/ape/src/raaga/bape_tru_volume.c

endif

ifeq ($(BAPE_DSP_SUPPORT),n)
BAPE_SOURCES := $(BAPE_FMM_SOURCES)
else
BAPE_SOURCES := $(BAPE_FMM_SOURCES) $(BAPE_DSP_SOURCES)
BAPE_DEFINES += BAPE_DSP_SUPPORT=1
endif


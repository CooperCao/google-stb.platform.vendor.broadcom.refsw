############################################################
#     Copyright (c) 2003-2012, Broadcom Corporation
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
# BAPE_EMULATION - If this is defined, messaging and ISR control will be left undefined for the application to implement in emulation
#

# Define this module
MAGNUM_MODULES += BAPE

# Module Includes
BAPE_INCLUDES += $(MAGNUM)/portinginterface/ape/include \
                 $(MAGNUM)/portinginterface/ape/include/mips \
                 $(MAGNUM)/portinginterface/ape/src/mips

# Determine presence of codecs.  If present, default them to enabled.
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_aac_1_decoder.c),)
BAPE_AAC_SUPPORT ?= y
endif
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_ac3_passthrough.c),)
BAPE_AC3_PASSTHROUGH_SUPPORT ?= y
endif
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_ac3_decoder.c),)
BAPE_AC3_SUPPORT ?= y
endif
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_ddp_2_converter.c),)
BAPE_DDP_SUPPORT ?= y
endif
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_mpeg_1_decoder.c),)
BAPE_MPEG_SUPPORT ?= y
endif
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_pcm_wav.c),)
BAPE_PCMWAV_SUPPORT ?= y
endif
ifneq ($(wildcard $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_wma_1_decoder.c),)
BAPE_WMA_SUPPORT ?= y
endif

ifneq ($(BAPE_IMG_ONLY),y)
# Module Sources
BAPE_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/bape.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_dac.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_mixer.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_playback.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_fmm_isr.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_pll.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_src.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_resources.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_ms_fw.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_spdif_output.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_mai_output.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_decoder.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_dsp_priv.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_dsp_debug.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_rfmod.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_output_capture.c \
                $(MAGNUM)/portinginterface/ape/src/mips/bape_fs_priv.c
endif

# Module definitions
BAPE_DEFINES += BAPE_MIPS=1

ifeq ($(BAPE_EMULATION),y)
BAPE_DEFINES += BAPE_EMULATION=1
endif
ifeq ($(BAPE_FILE_IMAGE),y)
BAPE_DEFINES += BAPE_FILE_IMAGE=1
endif

# Codec definitions
ifeq ($(BAPE_AAC_SUPPORT),y)
BAPE_DEFINES += BAPE_AAC_SUPPORT=1
endif
ifeq ($(BAPE_AC3_PASSTHROUGH_SUPPORT),y)
BAPE_DEFINES += BAPE_AC3_PASSTHROUGH_SUPPORT=1
endif
ifeq ($(BAPE_DDP_SUPPORT),y)
# DDP includes both AC3 and AC3+ support.
BAPE_DEFINES += BAPE_DDP_SUPPORT=1
else
# Only include AC3 if DDP is not defined
ifeq ($(BAPE_AC3_SUPPORT),y)
BAPE_DEFINES += BAPE_AC3_SUPPORT=1
endif
endif
ifeq ($(BAPE_MPEG_SUPPORT),y)
BAPE_DEFINES += BAPE_MPEG_SUPPORT=1
endif
ifeq ($(BAPE_PCMWAV_SUPPORT),y)
BAPE_DEFINES += BAPE_PCMWAV_SUPPORT=1
endif
ifeq ($(BAPE_WMA_SUPPORT),y)
BAPE_DEFINES += BAPE_WMA_SUPPORT=1
endif

# Firmware files
ifneq ($(BAPE_IMG_EXTERNAL),y)
include $(MAGNUM)/portinginterface/ape/build/bape_img_mips.inc
endif

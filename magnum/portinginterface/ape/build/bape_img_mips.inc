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
# $brcm_Log: $
# 
############################################################

MAGNUM_MODULES += BAPE_IMG
BAPE_IMG_INCLUDES := $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380

ifneq ($(BAPE_FILE_IMAGE),y)

BAPE_IMG_SOURCES := $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img.c \
                    $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_bkernel.c

ifeq ($(BAPE_AAC_SUPPORT),y)
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_aac_1_decoder.c
endif
ifeq ($(BAPE_AC3_PASSTHROUGH_SUPPORT),y)
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_ac3_passthrough.c
endif
ifeq ($(BAPE_DDP_SUPPORT),y)
# DDP includes both AC3 and AC3+ support.
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_ddp_2_converter.c
else
# Only include AC3 if DDP is not defined
ifeq ($(BAPE_AC3_SUPPORT),y)
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_ac3_decoder.c
endif
endif
ifeq ($(BAPE_MPEG_SUPPORT),y)
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_mpeg_1_decoder.c
endif
ifeq ($(BAPE_PCMWAV_SUPPORT),y)
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_pcm_wav.c
endif
ifeq ($(BAPE_WMA_SUPPORT),y)
BAPE_IMG_SOURCES += $(MAGNUM)/portinginterface/ape/src/mips/firmware/bmips4380/bape_img_wma_1_decoder.c
endif

BAPE_IMG_DEFINES += BAPE_IMG_COMPILED=1
else
BAPE_IMG_DEFINES += BAPE_FILE_IMAGE=1
endif

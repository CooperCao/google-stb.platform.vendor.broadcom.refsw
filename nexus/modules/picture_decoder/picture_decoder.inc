############################################################
#     (c)2003-2013 Broadcom Corporation
#  
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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

#
# Conditional Options for this Makefile
#
# None

######################
# Define this module #
######################
NEXUS_MODULES += PICTURE_DECODER

##################
# Module Options #
##################
NEXUS_PICTURE_DECODER_OPTIONS := 

###################
# Module Includes #
###################
NEXUS_PICTURE_DECODER_PUBLIC_INCLUDES := $(NEXUS_TOP)/modules/picture_decoder/include

##################
# Module Sources #
##################
NEXUS_PICTURE_DECODER_SOURCES := \
	$(NEXUS_TOP)/modules/picture_decoder/src/nexus_picture_decoder.c 

ifneq ($(findstring NEXUS_NUM_SID_VIDEO_DECODERS, $(NEXUS_PLATFORM_DEFINES)),)
include $(NEXUS_TOP)/extensions/picture_decoder/sid_video_decoder/sid_video_decoder.inc
endif


##################
# Module Defines #
##################

#######################
# Module Dependencies #
#######################
NEXUS_PICTURE_DECODER_DEPENDENCIES += BASE CORE SURFACE SECURITY

ifeq ($(NEXUS_CONFIG_IMAGE),y)
ifeq ($(NEXUS_MODE),proxy)
BSID_IMG_ONLY := y
else
BSID_IMG_EXTERNAL := y
endif
endif

##################
# Magnum Modules #
##################
NEXUS_PICTURE_DECODER_MAGNUM_MODULES += \
    $(MAGNUM)/portinginterface/sid/bsid.inc

###############
# Thunk Layer #
###############
NEXUS_PICTURE_DECODER_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/picture_decoder/src $(NEXUS_SYNCTHUNK_DIR)
NEXUS_PICTURE_DECODER_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_picture_decoder_thunks.c


############################################################
#     (c)2007-2012 Broadcom Corporation
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
# MEDIA_ASF_SUPPORT - If y, ASF  is enabled
# MEDIA_AVI_SUPPORT - If y, AVI/DIVX is enabled

######################
# Define this module #
######################
NEXUS_MODULES += PLAYBACK

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_PLAYBACK_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/playback/include
NEXUS_PLAYBACK_PRIVATE_INCLUDES = $(NEXUS_TOP)/modules/playback/src

##################
# Module Sources #
##################
NEXUS_PLAYBACK_SOURCES = \
    $(NEXUS_TOP)/modules/playback/src/nexus_playback.c \
    $(NEXUS_TOP)/modules/playback/src/nexus_playback_flow.c \
    $(NEXUS_TOP)/modules/playback/src/nexus_playback_media.c \
    $(NEXUS_TOP)/modules/playback/src/nexus_playback_module.c \
    $(NEXUS_TOP)/modules/playback/src/nexus_playback_trick.c

# thunk layer
NEXUS_PLAYBACK_PRIVATE_INCLUDES += $(NEXUS_SYNCTHUNK_DIR)
NEXUS_PLAYBACK_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_playback_thunks.c

##################
# Module Defines #
##################

#######################
# Module Dependencies #
#######################
NEXUS_PLAYBACK_DEPENDENCIES := FILE VIDEO_DECODER AUDIO TRANSPORT DMA SIMPLE_DECODER DIVXDRM

NEXUS_PLAYBACK_MAGNUM_MODULES =  \
    ${BSEAV}/lib/utils/batom.inc \
    ${BSEAV}/lib/media/bmedia.inc \
    ${BSEAV}/lib/bfile/bfile.inc

ifeq ($(MEDIA_ASF_SUPPORT), y)
NEXUS_PLAYBACK_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/asf/basf.inc
endif

ifeq ($(MEDIA_AVI_SUPPORT), y)
NEXUS_PLAYBACK_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/avi/bavi.inc
endif

ifeq ($(MEDIA_FLV_SUPPORT), y)
NEXUS_PLAYBACK_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/media/flv/bflv.inc
endif

ifeq ($(MEDIA_RMFF_SUPPORT), y)
NEXUS_PLAYBACK_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/media/rmff/brmff.inc
endif

NEXUS_PLAYBACK_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/bcmplayer/bcmplayer.inc 

# BSEAV makefiles don't include their module names into MAGNUM_MODULES

ifeq ($(NEXUS_COMMON_CRYPTO_SUPPORT), y)
include $(BSEAV)/lib/security/common_crypto/common_crypto.inc
NEXUS_PLAYBACK_PRIVATE_INCLUDES += $(COMMON_CRYPTO_INCLUDES)
ifneq ($(NEXUS_MODE),)
# if no proxy, common crypto built into wmdrmpd_core module
NEXUS_PLAYBACK_SOURCES += $(COMMON_CRYPTO_SOURCES)
endif
endif

# modules that are also used in the trasport/playpump
NEXUS_EXTRA_MODULES += BMEDIA BMEDIA_FILTER BATOM

# playback only modules
NEXUS_EXTRA_MODULES += BMEDIA_PLAYER BFILE BFILE_CACHE BFILE_MEDIA B_NAV_PLAYER B_MEDIA_NAV_PLAYER BAA_TREE 

# These modules are not needed by nexus itself, but are part of the media framework and are
# called directly by many apps.
NEXUS_EXTRA_MODULES += BMEDIA_PROBE BFILE_STDIO

ifeq ($(MSDRM_PRDY_SUPPORT),y)
ifneq ($(SAGE_SUPPORT),y)
include $(NEXUS_TOP)/extensions/playback/piff/piff_ext.inc
endif
endif

ifneq ($(NEXUS_PLAYBACK_EXTENSION_INC),)
include $(NEXUS_PLAYBACK_EXTENSION_INC)
endif

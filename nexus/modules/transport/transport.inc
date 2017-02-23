#############################################################################
# Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

######################
# Define this module #
######################
NEXUS_MODULES += TRANSPORT

##################
# Module Options #
##################

###################
# Module Includes #
###################
NEXUS_TRANSPORT_PUBLIC_INCLUDES = $(NEXUS_TOP)/modules/transport/include

ifeq ($(filter NEXUS_HAS_XPT_DMA, $(NEXUS_PLATFORM_DEFINES)), NEXUS_HAS_XPT_DMA)
NEXUS_TRANSPORT_PUBLIC_INCLUDES += $(NEXUS_TOP)/modules/dma/include
endif

NEXUS_TRANSPORT_PRECOMPILED_HEADER = $(NEXUS_TOP)/modules/transport/src/nexus_transport_module.h

##################
# Module Sources #
##################
NEXUS_TRANSPORT_SOURCES := \
    $(NEXUS_TOP)/modules/transport/src/nexus_demux.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_pid_channel_scrambling.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_parser_band.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_rave.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_timebase.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_stc_channel.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_playpump.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_playpump_crypto.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_playpump_media.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_playpump_priv.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_recpump.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_transport_capabilities.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_transport_module.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_tsmux.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_mpod.c   \
    $(NEXUS_TOP)/modules/transport/src/nexus_packetsub.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_remux.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_tsmf_stub.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_vcxo.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_parser_band_channelbonding.c \
    $(NEXUS_TOP)/modules/transport/src/nexus_gcb_sw.c

ifeq ($(filter NEXUS_HAS_XPT_DMA, $(NEXUS_PLATFORM_DEFINES)), NEXUS_HAS_XPT_DMA)
NEXUS_TRANSPORT_SOURCES += $(NEXUS_TOP)/modules/transport/src/nexus_xpt_dma.c
endif

ifneq ($(findstring $(BCHP_CHIP),7550 7408),)
# Software-based message filtering. Required for chips w/o HW filtering. Optional for other chips.
NEXUS_USE_SW_FILTER=y
endif

ifeq ($(NEXUS_USE_SW_FILTER),y)
NEXUS_TRANSPORT_SOURCES += $(NEXUS_TOP)/modules/transport/src/nexus_message_swfilter.c
NEXUS_TRANSPORT_SOURCES += $(NEXUS_TOP)/modules/transport/src/nexus_message_swfilter_parser.c
NEXUS_TRANSPORT_DEFINES += NEXUS_USE_SW_FILTER=1
else
NEXUS_TRANSPORT_SOURCES += $(NEXUS_TOP)/modules/transport/src/nexus_message_pid2buf.c
endif
NEXUS_TRANSPORT_SOURCES += $(NEXUS_TOP)/modules/transport/src/nexus_message_sam.c

ifeq ($(BXPT_VCT_SUPPORT),y)
NEXUS_TRANSPORT_DEFINES += BXPT_VCT_SUPPORT=1
endif

ifeq ($(NEXUS_INTEGRATED_M2M_SUPPORT),y)
NEXUS_TRANSPORT_DEFINES += NEXUS_INTEGRATED_M2M_SUPPORT
endif

# thunk layer
NEXUS_TRANSPORT_PRIVATE_INCLUDES += $(NEXUS_TOP)/modules/transport/src $(NEXUS_SYNCTHUNK_DIR)
NEXUS_TRANSPORT_SOURCES += $(NEXUS_SYNCTHUNK_DIR)/nexus_transport_thunks.c

# pvr utils
NEXUS_TRANSPORT_PRIVATE_INCLUDES += \
    $(BSEAV)/lib/utils
NEXUS_TRANSPORT_SOURCES += \
    $(BSEAV)/lib/utils/bsink_playback.c

#######################
# Module Dependencies #
#######################
ifeq ($(filter NEXUS_HAS_XPT_DMA, $(NEXUS_PLATFORM_DEFINES)), )
NEXUS_TRANSPORT_DEPENDENCIES := DMA
else
NEXUS_TRANSPORT_DEPENDENCIES := BASE CORE
endif

##################
# Magnum Modules #
##################
# These modules will be excluded from app-only builds
BPCRLIB_HAS_RAP := n
BPCRLIB_HAS_APE := n
BPCRLIB_HAS_XVD := n
BPCRLIB_HAS_MVD := n
NEXUS_TRANSPORT_MAGNUM_MODULES =  \
    $(MAGNUM)/portinginterface/xpt/bxpt.inc \
    $(MAGNUM)/portinginterface/xpt/bxpt_auth.inc \
    $(MAGNUM)/syslib/pvrlib/bpvrlib.inc \
    $(MAGNUM)/syslib/pcrlib/bpcrlib.inc

NEXUS_FW_SIGN_MODULES += BXPT

# change XPT PI default behavior so that app has option of starting msg filter with disabled pid channel
NEXUS_TRANSPORT_DEFINES += BXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL

ifeq ($(BXPT_FILTER_32),y)
NEXUS_TRANSPORT_DEFINES += BXPT_FILTER_32 NEXUS_MESSAGE_FILTER_SIZE=32
endif

# BSEAV makefiles don't include their module names into MAGNUM_MODULES
NEXUS_EXTRA_MODULES += BFILE BMEDIA BMEDIA_FILTER BATOM

ifeq ($(PVR_SUPPORT),n)
# we don't need the media framework, but still need some code from BSEAV
NEXUS_TRANSPORT_DEFINES += B_HAS_MEDIA=0
ifneq ($(B_REFSW_OS),ucos_ii)
NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/bfile/bfile.inc
endif

NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/utils/batom.inc

NEXUS_TRANSPORT_MAGNUM_MODULES += ${BSEAV}/lib/mpeg2_ts_parse/mpeg2_ts_parse.inc
NEXUS_EXTRA_MODULES += MPEG2_TS_PARSE
else
NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/utils/batom.inc \
    ${BSEAV}/lib/media/bmedia.inc

ifeq ($(MEDIA_ASF_SUPPORT), y)
NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/asf/basf.inc
endif

ifeq ($(MSDRM_PRDY_SUPPORT), y)
NEXUS_TRANSPORT_PRIVATE_INCLUDES += $(BSEAV)/lib/playready/$(NEXUS_PLAYREADY_SDK_VERSION) $(BSEAV)/lib/playready/$(NEXUS_PLAYREADY_SDK_VERSION)/inc $(BSEAV)/lib/utils

ifeq ($(MEDIA_ASF_SUPPORT), y)
NEXUS_TRANSPORT_PRIVATE_INCLUDES += $(BSEAV)/lib/asf
endif

endif

ifeq ($(MEDIA_AVI_SUPPORT), y)
NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/avi/bavi.inc
endif

endif

ifeq ($(MEDIA_FLV_SUPPORT), y)
NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/media/flv/bflv.inc
endif

ifeq ($(MEDIA_RMFF_SUPPORT), y)
NEXUS_TRANSPORT_MAGNUM_MODULES +=  \
    ${BSEAV}/lib/media/rmff/brmff.inc
endif

ifeq ($(B_REFSW_DSS_SUPPORT),)
B_REFSW_DSS_SUPPORT=y
endif
ifeq ($(B_REFSW_DSS_SUPPORT),y)
NEXUS_TRANSPORT_DEFINES += B_REFSW_DSS_SUPPORT=1
endif

ifeq ($(NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED),y)
NEXUS_TRANSPORT_DEFINES += NEXUS_RAVE_OUTPUT_CAPTURE_ENABLED=1
NEXUS_TRANSPORT_SOURCES += nexus_rave_capture.c
endif

ifeq ($(NEXUS_RAVE_INPUT_CAPTURE_ENABLED),y)
NEXUS_TRANSPORT_DEFINES += NEXUS_RAVE_INPUT_CAPTURE_ENABLED=1
NEXUS_TRANSPORT_CLIENT_CAPTURE_ENABLED:=y
endif

ifeq ($(NEXUS_TRANSPORT_CLIENT_CAPTURE_ENABLED),y)
NEXUS_TRANSPORT_DEFINES += NEXUS_TRANSPORT_CLIENT_CAPTURE_ENABLED=1
NEXUS_TRANSPORT_SOURCES += nexus_xc_capture.c
endif

#####################
# Module Extensions #
#####################
ifneq ($(NEXUS_TRANSPORT_EXTENSION_INC),)
include $(NEXUS_TRANSPORT_EXTENSION_INC)
endif

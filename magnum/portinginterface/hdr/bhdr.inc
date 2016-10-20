##############################################################################
#  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
#
#  This program is the proprietary software of Broadcom and/or its licensors,
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
##############################################################################

#
# Conditional Options for this Makefile
#
# N/A
#

# Define this module
MAGNUM_MODULES += BHDR

# Module Defines

# Module Includes
BHDR_INCLUDES = \
	$(MAGNUM)/portinginterface/hdr/include \
	$(MAGNUM)/portinginterface/hdr/src/common


# Module Common Sources
BHDR_SOURCES = \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_hdcp.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_hdcp_priv.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_priv.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_acp_isrcx.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_acr.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_audio.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_avi.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_drm.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_gamut.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_spd.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_vsi.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_phy_priv.c \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_debug.c

BHDR_EDID_RAM_SUPPORT ?= y
BHDR_HDCP_REPEATER_SUPPORT ?= y

BHDR_SOURCES += \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_pwr.c

ifeq ($(BHDR_EDID_RAM_SUPPORT),y)
BHDR_SOURCES += $(MAGNUM)/portinginterface/hdr/src/common/bhdr_edid_ram.c
BHDR_DEFINES += BHDR_CONFIG_EDID_RAM=1
endif


ifeq ($(BHDR_HDCP_REPEATER_SUPPORT),y)
BHDR_DEFINES += BHDR_CONFIG_HDCP_REPEATER=1
BHDR_SOURCES += \
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_hdcp_repeater.c
else
BHDR_DEFINES += BHDR_CONFIG_HDCP_REPEATER=0
endif


#################
#Chip specific Sources
#################

##################
### 40nm platforms ####
##################
ifeq ($(filter $(BCHP_CHIP), 7422 7425 7435 7429 74295), $(BCHP_CHIP))
	BHDR_INCLUDES += \
		$(MAGNUM)/portinginterface/hdr/src/core40nm

	BHDR_SOURCES += \
		$(MAGNUM)/portinginterface/hdr/src/core40nm/bhdr_fe.c \
		$(MAGNUM)/portinginterface/hdr/src/core40nm/bhdr_fe_priv.c

##################
##     New Chips          ##
##  28nm platforms      ##
##################
else
	BHDR_INCLUDES += \
		$(MAGNUM)/portinginterface/hdr/src/core28nm

	BHDR_SOURCES += \
		$(MAGNUM)/portinginterface/hdr/src/core28nm/bhdr_fe.c \
		$(MAGNUM)/portinginterface/hdr/src/core28nm/bhdr_fe_priv.c \

endif

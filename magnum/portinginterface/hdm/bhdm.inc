#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

# Define this module
MAGNUM_MODULES += BHDM

# Module Includes
BHDM_INCLUDES += \
	$(MAGNUM)/portinginterface/hdm/include

# Module Sources
BHDM_SOURCES += \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm.c

ifndef BHDM_FOR_BOOTUPDATER
BHDM_SOURCES += \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_edid.c \
	\
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_audio.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_avi.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_drm.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_spd.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_vsi.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_acr_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_acr_priv_isr.c \
	\
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_monitor.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_edid_debug.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_hdcp.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_pwr.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_tmds_priv.c
endif

#3D Support
ifndef (BHDM_3D_SUPPORT)
BHDM_3D_SUPPORT=y
endif

ifeq ($(BHDM_3D_SUPPORT), y)
	ifndef BHDM_FOR_BOOTUPDATER
	BHDM_SOURCES +=	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_edid_3d.c
	BHDM_DEFINES += BHDM_3D_SUPPORT
	endif
endif

ifdef BHDM_FOR_BOOTUPDATER
BHDM_DEFINES += BHDM_FOR_BOOTUPDATER
endif


#################
#Chip specific
#################

##################
### 40nm platforms ####
##################
ifeq ($(filter $(BCHP_CHIP), 7422 7425 7429 74295 7435 7231 7358 7344 7346 7360 7362 73625 7552 7640 7584 75845 7563 75635 7543 7228 75525 73465), $(BCHP_CHIP))
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core40nm/bhdm_priv.c


##################
##  28nm platforms      ##
##################
else
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core28nm/bhdm_priv.c
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core28nm/bhdm_save_restore.c

#########################################
## Majority of 28nm chips support HDMI 2.0
## BHDM_CONFIG adjusts macros for those 28nm that do not
#########################################

	BHDM_HDMI_20_SUPPORT=y
endif


##################
##   All chips except    ##
##  the following         ##
## support HDMI 1.3    ##
#################
ifneq ($(filter $(BCHP_CHIP), 7325 7335 7400 7405), $(BCHP_CHIP))
BHDM_DEFINES += BHDM_HAS_HDMI_1_3_FEATURES
endif

ifeq ($(BHDM_HDMI_20_SUPPORT), y)
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core28nm/bhdm_scdc.c
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core28nm/bhdm_auto_i2c.c
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core28nm/bhdm_auto_i2c_priv.c
endif


##
##
# For boot loder or mini-driver to keep gfx display on
#
ifeq ($(B_REFSW_GFX_PERSIST),y)
BHDM_DEFINES += BHDM_GFX_PERSIST
endif

BCHP_DBV_HW_DEF=$(MAGNUM)/basemodules/chp/include/$(BCHP_CHIP)/rdb/$(BCHP_VER_LOWER)/bchp_hdr_cmp_0_dbv.h
MAGNUM_HDMI_DBV_IMPL=$(MAGNUM)/portinginterface/hdm/src/common/bhdm_dbv_priv.c
ifneq ("$(wildcard $(BCHP_DBV_HW_DEF))","")
ifneq ("$(wildcard $(MAGNUM_HDMI_DBV_IMPL))","")
BHDM_DBV_SUPPORT?=y
endif
endif
ifeq ($(BHDM_DBV_SUPPORT),y)
BHDM_DEFINES += BHDM_DBV_SUPPORT=1
BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/commond/bhdm_dbv_priv.c
endif

# Module Dependencies
ifeq ($(BCHP_CHIP),7211)
include $(MAGNUM)/portinginterface/ott/bott.inc
endif
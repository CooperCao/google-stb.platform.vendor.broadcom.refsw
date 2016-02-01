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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
#
############################################################


# Define this module
MAGNUM_MODULES += BHDM

# Module Includes
BHDM_INCLUDES += \
	$(MAGNUM)/portinginterface/hdm/include \
	$(MAGNUM)/portinginterface/hdm/src/common

# Module Sources
BHDM_SOURCES += \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm.c

ifndef BHDM_FOR_BOOTUPDATER
BHDM_SOURCES += \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_edid.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_drm.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_acr_priv.c \
	$(MAGNUM)/portinginterface/hdm/src/common/bhdm_packet_acr_priv_isr.c \
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
### 65nm platforms ####
###   FALCON CORE ####
##################
ifeq ($(filter $(BCHP_CHIP), 7325 7335 7400 7405), $(BCHP_CHIP))

	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core65nm/bhdm_priv_falcon.c

	# DVO Support
	ifeq ($(BHDM_DVO_ENABLE_SUPPORT),y)
		BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core65nm/bhdm_dvo_falcon.c
		BHDM_DEFINES += BHDM_ENABLE_DVO
	endif

##################
### 65nm platforms ####
###   DVP HT CORE ####
##################
else ifeq ($(filter $(BCHP_CHIP), 7125 7340 7342 7408 7420 7468 7550), $(BCHP_CHIP))
	### 65nm platforms with HDMI DVP_HT core

	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core65nm/bhdm_priv_dvpht.c

	# DVO Support
	ifeq ($(BHDM_DVO_ENABLE_SUPPORT),y)
		BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core65nm/bhdm_dvo_dvpht.c
		BHDM_DEFINES += BHDM_ENABLE_DVO
	endif

##################
### 40nm platforms ####
##################
else ifeq ($(filter $(BCHP_CHIP), 7422 7425 7429 74295 7435 7231 7358 7344 7346 7360 7362 73625 7552 7640 7584 75845 7563 75635 7543 7228 75525 73465), $(BCHP_CHIP))
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core40nm/bhdm_priv.c


##################
##  28nm platforms      ##
##################
else
	BHDM_SOURCES += $(MAGNUM)/portinginterface/hdm/src/core28nm/bhdm_priv.c

## BHDM_CONFIG adjusts macros for non HDMI 2.0 SUPPORT
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
	BHDM_DEFINES += BHDM_HAS_HDMI_20_SUPPORT
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

ifeq ($(filter $(BCHP_CHIP), 7250 7268 7271 7364 7586), $(BCHP_CHIP))
include	$(MAGNUM)/portinginterface/hdm/bhdm_mhl.inc
endif

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
	$(MAGNUM)/portinginterface/hdr/src/common/bhdr_packet_priv.c \
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
		$(MAGNUM)/portinginterface/hdr/src/core40nm/bhdr_fe_priv.c \
		$(MAGNUM)/portinginterface/hdr/src/common/bhdr_hdcp_priv.c \
		$(MAGNUM)/portinginterface/hdr/src/common/bhdr_phy_priv.c


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
		$(MAGNUM)/portinginterface/hdr/src/common/bhdr_hdcp_priv.c \
		$(MAGNUM)/portinginterface/hdr/src/common/bhdr_phy_priv.c

endif

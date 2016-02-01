############################################################
#     Copyright (c) 2003-2014, Broadcom Corporation
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
MAGNUM_MODULES += BGRC

# Module Includes
BGRC_INCLUDES += $(MAGNUM)/portinginterface/grc/include

# Module Sources
ifeq ($(GRCPACKET_SUPPORT),y)
BGRC_SOURCES += $(MAGNUM)/portinginterface/grc/src/bgrc_packet.c
BGRC_SOURCES += $(MAGNUM)/portinginterface/grc/src/bgrc_packet_priv.c
BGRC_DEFINES += BGRC_PACKET_MODE
else
BGRC_SOURCES += $(MAGNUM)/portinginterface/grc/src/bgrc.c
BGRC_SOURCES += $(MAGNUM)/portinginterface/grc/src/bgrc_private.c
BGRC_SOURCES += $(MAGNUM)/portinginterface/grc/src/bgrc_coeff_private.c
BGRC_DEFINES += BGRC_NON_PACKET_MODE
endif

ifneq ($(BGRC_PACKET_VERIFY_SURFACE_RECTANGLE),n)
BGRC_DEFINES += BGRC_PACKET_P_VERIFY_SURFACE_RECTANGLE
endif

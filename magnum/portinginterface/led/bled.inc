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
# Created: 01/09/2007 by Jon Garrett
#
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#
# BLED_3250_SUPPORT - If defined, build for the 3250 led as opposed to the chip specified by BCHP_CHIP
#

# Define this module
MAGNUM_MODULES += BLED

# Module Includes
ifneq ($(BLED_3250_SUPPORT),)
	BLED_INCLUDES += $(MAGNUM)/portinginterface/led/3250
else
	BLED_INCLUDES += $(MAGNUM)/portinginterface/led/include
endif

# Module Sources
ifneq ($(BLED_3250_SUPPORT),)
	BLED_SOURCES += $(MAGNUM)/portinginterface/led/3250/bled.c
else
	BLED_SOURCES += $(MAGNUM)/portinginterface/led/src/bled.c
endif


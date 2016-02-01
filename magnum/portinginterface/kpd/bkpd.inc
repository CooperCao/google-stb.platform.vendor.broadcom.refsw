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
# BKPD_3250_SUPPORT - If defined, build for the 3250 keypad as opposed to the chip specified by BCHP_CHIP.
#

# Define this module
MAGNUM_MODULES += BKPD

# Module Includes
ifneq ($(BKPD_3250_SUPPORT),)
	BKPD_INCLUDES += $(MAGNUM)/portinginterface/kpd/3250
else
	BKPD_INCLUDES += $(MAGNUM)/portinginterface/kpd/include
endif

# Module Sources
ifneq ($(BKPD_3250_SUPPORT),)
	BKPD_SOURCES += $(MAGNUM)/portinginterface/kpd/3250/bkpd.c
else
	BKPD_SOURCES += $(MAGNUM)/portinginterface/kpd/src/bkpd.c
endif


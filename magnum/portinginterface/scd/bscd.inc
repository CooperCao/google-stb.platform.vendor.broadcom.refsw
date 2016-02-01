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
MAGNUM_MODULES += BSCD

# Module Includes
BSCD_INCLUDES += $(MAGNUM)/portinginterface/scd/include
BSCD_INCLUDES += $(MAGNUM)/portinginterface/scd/src/common

BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/common/bscd_emvpriv.c

# Module Sources
ifeq ($(findstring $(BCHP_CHIP), 7425 7422 7358 7552 7344 7640 7360 7435 7231 7346 7429 7584 7563 75635 7445 7145 7366 74371 7439 7362 73625 7364 7228 7250 7586 75845 74295 73465 75525 7271 7268), $(BCHP_CHIP))
	BSCD_INCLUDES += $(MAGNUM)/portinginterface/scd/src/core40nm

	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core40nm/bscd.c
	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core40nm/bscd_priv.c
else
	BSCD_INCLUDES += $(MAGNUM)/portinginterface/scd/src/core65nm

	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core65nm/bscd.c
	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core65nm/bscd_priv.c
endif


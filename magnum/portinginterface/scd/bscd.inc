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
ifeq ($(findstring $(BCHP_CHIP), 7125 7340 7420 7468 7405 7400 7325 7408 7550), $(BCHP_CHIP))
	BSCD_INCLUDES += $(MAGNUM)/portinginterface/scd/src/core65nm

	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core65nm/bscd.c
	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core65nm/bscd_priv.c
else
	BSCD_INCLUDES += $(MAGNUM)/portinginterface/scd/src/core40nm

	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core40nm/bscd.c
	BSCD_SOURCES += $(MAGNUM)/portinginterface/scd/src/core40nm/bscd_priv.c
endif

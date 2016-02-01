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
# N/A
#

# Define this module
MAGNUM_MODULES += BUHF

# Module Includes
BUHF_INCLUDES += $(MAGNUM)/portinginterface/uhf/include

# Module Sources
ifeq ($(filter $(BCHP_CHIP), 7325 7335 7336 7340 7342 7400 7401 7403 7405 7408 7413 7420), $(BCHP_CHIP))
	BUHF_SOURCES += $(MAGNUM)/portinginterface/uhf/src/core65nm/buhf.c
else
	BUHF_SOURCES += $(MAGNUM)/portinginterface/uhf/src/core40nm/buhf.c
endif


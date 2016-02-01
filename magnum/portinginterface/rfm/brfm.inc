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
MAGNUM_MODULES += BRFM

# Module Includes
BRFM_INCLUDES += $(MAGNUM)/portinginterface/rfm/include

# Module Sources
BRFM_SOURCES += $(MAGNUM)/portinginterface/rfm/src/brfm.c
BRFM_SOURCES += $(MAGNUM)/portinginterface/rfm/src/brfm_scripts.c

ifeq ($(filter $(BCHP_CHIP), 7400 7420), $(BCHP_CHIP))
BRFM_SOURCES += $(MAGNUM)/portinginterface/rfm/src/bbe/brfm_scripts_bbe.c
endif
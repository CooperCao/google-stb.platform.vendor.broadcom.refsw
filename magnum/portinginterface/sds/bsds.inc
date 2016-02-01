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
MAGNUM_MODULES += BSDS

# Module Includes
BSDS_INCLUDES += $(MAGNUM)/basemodules/chp/include/4500/common
BSDS_INCLUDES += $(MAGNUM)/portinginterface/sds/4500

# Module Sources
BSDS_SOURCES += \
	$(MAGNUM)/portinginterface/sds/4500/bsds.c	 \
	$(MAGNUM)/portinginterface/sds/4500/bsds_priv.c


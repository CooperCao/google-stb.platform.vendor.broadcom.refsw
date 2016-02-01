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
# $brcm_Log: $
# 
############################################################

#
# Conditional Options for this Makefile
#

# Define this module
MAGNUM_MODULES += BVDE

# Module Includes
BVDE_INCLUDES += $(MAGNUM)/portinginterface/vde/include \
                 $(MAGNUM)/portinginterface/vde/src

# Module Sources
BVDE_SOURCES += $(MAGNUM)/portinginterface/vde/src/bvde.c \
                $(MAGNUM)/portinginterface/vde/src/bvde_channel.c \
                $(MAGNUM)/portinginterface/vde/src/bvde_dsp_utils_priv.c \

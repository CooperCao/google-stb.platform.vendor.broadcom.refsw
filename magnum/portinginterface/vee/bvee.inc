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
MAGNUM_MODULES += BVEE

# Determine if Accelerator support should be enabled or not
ifeq ($(BDSP_ENCODER_ACCELERATOR_SUPPORT),y)
BVEE_DEFINES += BDSP_ENCODER_ACCELERATOR_SUPPORT=1
else
BVEE_DEFINES += BDSP_ENCODER_ACCELERATOR_SUPPORT=0
endif

# Module Includes
BVEE_INCLUDES += $(MAGNUM)/portinginterface/vee/include
BVEE_INCLUDES += $(MAGNUM)/portinginterface/vee/src
BVEE_INCLUDES += $(MAGNUM)/commonutils/sur/src

# Module Sources
BVEE_SOURCES += $(MAGNUM)/portinginterface/vee/src/bvee.c \
                $(MAGNUM)/portinginterface/vee/src/bvee_channel.c \
                $(MAGNUM)/portinginterface/vee/src/bvee_dsp_utils_priv.c \

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
MAGNUM_MODULES += BRDC

# Module Includes
BRDC_INCLUDES += $(MAGNUM)/commonutils/rdc/include

ifeq ($(BRDC_DISABLE_TRACK_EXECUTION),y)
BRDC_DEFINES += BRDC_DISABLE_TRACK_EXECUTION
endif

# Module Sources
BRDC_SOURCES += \
	$(MAGNUM)/commonutils/rdc/src/brdc.c \
	$(MAGNUM)/commonutils/rdc/src/brdc_dbg.c \
	$(MAGNUM)/commonutils/rdc/src/brdc_private.c \
	$(MAGNUM)/commonutils/rdc/src/brdc_blockout_priv.c

#
# End of file
#
